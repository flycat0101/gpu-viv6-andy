/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/
/* $XFree86: xc/lib/GL/dri/dri_glx.c,v 1.12 2003/02/06 12:42:10 alanh Exp $ */

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *   Brian Paul <brian@precisioninsight.com>
 *
 */


#include <unistd.h>
#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>

#ifdef X11_DRI3
#include <X11/Xlib-xcb.h>
#include <X11/xshmfence.h>
#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <xcb/present.h>
#endif

#include "extutil.h"
#include "glxclient.h"
#include "xf86dri.h"
#include "sarea.h"
#include <stdio.h>
#include <dlfcn.h>
#include "dri_util.h"
#include <sys/types.h>
#include <stdarg.h>


#ifndef DEFAULT_DRIVER_DIR
/* this is normally defined in the Imakefile */
#define DEFAULT_DRIVER_DIR "/usr/lib/dri"
#endif


static __DRIdriver *Drivers = NULL;


/*
 * printf wrappers
 */
static GLvoid InfoMessageF(const char *f, ...)
{
    va_list args;
    const char *env;

    if ((env = getenv("LIBGL_DEBUG")) && strstr(env, "verbose")) {
        fprintf(stderr, "libGL: ");
        va_start(args, f);
        vfprintf(stderr, f, args);
        va_end(args);
    }
}

static GLvoid ErrorMessageF(const char *f, ...)
{
    va_list args;

    if (getenv("LIBGL_DEBUG")) {
        fprintf(stderr, "libGL error: ");
        va_start(args, f);
        vfprintf(stderr, f, args);
        va_end(args);
    }
}


/*
 * We'll save a pointer to this function when we couldn't find a
 * direct rendering driver for a given screen.
 */
static GLvoid *DummyCreateScreen(Display *dpy, int scrn, __DRIscreen *psc,
                               int numConfigs, __GLXvisualConfig *config)
{
    (GLvoid) dpy;
    (GLvoid) scrn;
    (GLvoid) psc;
    (GLvoid) numConfigs;
    (GLvoid) config;
    return NULL;
}



/*
 * Extract the ith directory path out of a colon-separated list of
 * paths.
 * Input:
 *   index - index of path to extract (starting at zero)
 *   paths - the colon-separated list of paths
 *   dirLen - max length of result to store in <dir>
 * Output:
 *   dir - the extracted directory path, dir[0] will be zero when
 *         extraction fails.
 */
static GLvoid ExtractDir(int index, const char *paths, int dirLen, char *dir)
{
   int i, len;
   const char *start, *end;

   /* find ith colon */
   start = paths;
   i = 0;
   while (i < index) {
      if (*start == ':') {
         i++;
         start++;
      }
      else if (*start == 0) {
         /* end of string and couldn't find ith colon */
         dir[0] = 0;
         return;
      }
      else {
         start++;
      }
   }

   while (*start == ':')
      start++;

   /* find next colon, or end of string */
   end = start + 1;
   while (*end != ':' && *end != 0) {
      end++;
   }

   /* copy string between <start> and <end> into result string */
   len = end - start;
   if (len > dirLen - 1)
      len = dirLen - 1;
   strncpy(dir, start, len);
   dir[len] = 0;
}


/*
 * Try to dlopen() the named driver.  This function adds the
 * "_dri.so" suffix to the driver name and searches the
 * directories specified by the LIBGL_DRIVERS_PATH env var
 * in order to find the driver.
 * Input:
 *   driverName - a name like "tdfx", "i810", "mga", etc.
 * Return:
 *   handle from dlopen, or NULL if driver file not found.
 */
static __DRIdriver *OpenDriver(const char *driverName)
{
   char *libPaths = NULL;
   char *error;
   int i;
   __DRIdriver *driver;

   /* First, search Drivers list to see if we've already opened this driver */
   for (driver = Drivers; driver; driver = driver->next) {
      if (strcmp(driver->name, driverName) == 0) {
         /* found it */
         return driver;
      }
   }

   if (geteuid() == getuid()) {
      /* don't allow setuid apps to use LIBGL_DRIVERS_PATH */
      libPaths = getenv("LIBGL_DRIVERS_PATH");
   }
   if (!libPaths)
      libPaths = DEFAULT_DRIVER_DIR;

   for (i = 0; ; i++) {
      char libDir[1000], realDriverName[200];
      GLvoid *handle;
      ExtractDir(i, libPaths, 1000, libDir);
      if (!libDir[0])
         break; /* ran out of paths to search */
      snprintf(realDriverName, 200, "%s/%s_dri.so", libDir, driverName);
      InfoMessageF("OpenDriver: trying %s\n", realDriverName);
      handle = dlopen(realDriverName, RTLD_NOW | RTLD_GLOBAL);
      if ((error = dlerror()) != NULL) {
         fprintf (stderr, "%s\n", error);
      }
      if (handle) {
         /* allocate __DRIdriver struct */
         driver = (__DRIdriver *) Xmalloc(sizeof(__DRIdriver));
         if (!driver)
         {
            dlclose(handle);
            return NULL; /* out of memory! */
         }
         /* init the struct */
         driver->name = __glXstrdup(driverName);
         if (!driver->name) {
            Xfree(driver);
            dlclose(handle);
            return NULL; /* out of memory! */
         }
         dlerror();    /* Clear any existing error */

         driver->createScreenFunc = (CreateScreenFunc)
            dlsym(handle, "__driCreateScreen");
         driver->createNewScreenFunc = (CreateNewScreenFunc)
            dlsym(handle, "__driCreateNewScreen");

         if ((error = dlerror()) != NULL) {
            fprintf (stderr, "%s\n", error);
         }
         if ( (driver->createScreenFunc == NULL) &&
              (driver->createNewScreenFunc == NULL) ) {
            /* If the driver doesn't have this symbol then something's
             * really, really wrong.
             */
            ErrorMessageF("Neither __driCreateScreen or __driCreateNewScreen "
                          "are defined in %s_dri.so!\n", driverName);
            Xfree(driver->name);
            Xfree(driver);
            dlclose(handle);
            continue;
         }
         driver->handle = handle;
         driver->refCount = 0;
         /* put at head of linked list */
         driver->next = Drivers;
         Drivers = driver;
         return driver;
      }
      else {
         ErrorMessageF("dlopen %s failed (%s)\n", realDriverName, dlerror());
      }
   }

   ErrorMessageF("unable to find driver: %s_dri.so\n", driverName);
   return NULL;
}


/*
 * Given a display pointer and screen number, determine the name of
 * the DRI driver for the screen. (I.e. "r128", "tdfx", etc).
 * Return True for success, False for failure.
 */
static Bool GetDriverName(Display *dpy, int scrNum, char **driverName)
{

   *driverName = __glXstrdup("vivante");

   return True;
}


/*
 * Given a display pointer and screen number, return a __DRIdriver handle.
 * Return NULL if anything goes wrong.
 */
static __DRIdriver *GetDriver(Display *dpy, int scrNum)
{
   char *driverName;
   __DRIdriver *ret;

   if (GetDriverName(dpy, scrNum, &driverName)) {
      ret = OpenDriver(driverName);
      if (driverName)
          Xfree(driverName);
      return ret;
   }

   return NULL;
}

/*
 * Exported function for querying the DRI driver for a given screen.
 *
 * The returned char pointer points to a static array that will be
 * overwritten by subsequent calls.
 */
const char *glXGetScreenDriver (Display *dpy, int scrNum) {
    char *ret = NULL;
    char *driverName = NULL;

    if (GetDriverName(dpy, scrNum, &driverName) && driverName)
    {
        static char buf[32];
        int len = strlen (driverName);

        if (len < sizeof(buf) - 1)
        {
            memcpy (buf, driverName, len + 1);
            ret = buf;
        }
        Xfree(driverName);
    }

    return ret;
}


/*
 * Exported function for obtaining a driver's option list (UTF-8 encoded XML).
 *
 * The returned char pointer points directly into the driver. Therefore
 * it should be treated as a constant.
 *
 * If the driver was not found or does not support configuration NULL is
 * returned.
 *
 * Note: The driver remains opened after this function returns.
 */
const char *glXGetDriverConfig (const char *driverName) {
    __DRIdriver *driver = OpenDriver (driverName);
    if (driver)
        return dlsym (driver->handle, "__driConfigOptions");
    else
        return NULL;
}


/* This function isn't currently used.
 */
static GLvoid driDestroyDisplay(Display *dpy, GLvoid *private)
{
    __DRIdisplayPrivate *pdpyp = (__DRIdisplayPrivate *)private;

    if (pdpyp) {
        const int numScreens = ScreenCount(dpy);
        int i;
        for (i = 0; i < numScreens; i++) {
            __DRIdriver *driver = GetDriver(dpy, i);
            driver->refCount--;
            if(!driver->refCount)
                dlclose(driver->handle);
        }
        Xfree(pdpyp->libraryHandles);
        Xfree(pdpyp);
    }
}


/*
 * Allocate, initialize and return a __DRIdisplayPrivate object.
 * This is called from __glXInitialize() when we are given a new
 * display pointer.
 */
typedef GLvoid (*TDESDISP)(__DRInativeDisplay *dpy, GLvoid *displayPrivate);

#ifdef X11_DRI3
static GLboolean check_dri3(xcb_connection_t *con) 
{
    const xcb_query_extension_reply_t *ext;

    if (!con)
    {
        return GL_FALSE;
    }

    xcb_prefetch_extension_data (con, &xcb_dri3_id);
    xcb_prefetch_extension_data (con, &xcb_present_id);
    ext = xcb_get_extension_data(con, &xcb_dri3_id);
    if (!(ext && ext->present))
    {
        return GL_FALSE;
    }
    ext = xcb_get_extension_data(con, &xcb_present_id);
    if (!(ext && ext->present))
    {
        return GL_FALSE;
    }
    return GL_TRUE;
}
#endif

GLvoid *driCreateDisplay(Display *dpy, __DRIdisplay *pdisp)
{
    const int numScreens = ScreenCount(dpy);
    __DRIdisplayPrivate *pdpyp;
    int eventBase, errorBase;
    int major = 4, minor = 0, patch = 0;
    int scrn;
    GLboolean dri3;

#ifdef X11_DRI3
    xcb_connection_t                     *c = XGetXCBConnection(dpy);
    xcb_dri3_query_version_cookie_t      dri3_cookie;
    xcb_dri3_query_version_reply_t       *dri3_reply;
    xcb_generic_error_t                  *error;
    GLboolean hasdri3;
#endif

    /* Initialize these fields to NULL in case we fail.
     * If we don't do this we may later get segfaults trying to free random
     * addresses when the display is closed.
     */
    pdisp->private = NULL;
    pdisp->destroyDisplay = NULL;
    pdisp->createScreen = NULL;

#ifndef X11_DRI3

    dri3 = GL_FALSE;
    if (!XF86DRIQueryExtension(dpy, &eventBase, &errorBase)) {
        return NULL;
    }

    if (!XF86DRIQueryVersion(dpy, &major, &minor, &patch)) {
        return NULL;
    }


#else

    hasdri3 = check_dri3(c);
    if (!hasdri3){

        if (!XF86DRIQueryExtension(dpy, &eventBase, &errorBase)) {
            return NULL;
        }

        if (!XF86DRIQueryVersion(dpy, &major, &minor, &patch)) {
            return NULL;
        }
        dri3 = GL_FALSE;

    } else {
        dri3 = GL_TRUE;

        dri3_cookie = xcb_dri3_query_version(c,
                                        XCB_DRI3_MAJOR_VERSION,
                                        XCB_DRI3_MINOR_VERSION);
        dri3_reply = xcb_dri3_query_version_reply(c, dri3_cookie, &error);
        if (!dri3_reply) {
            free(error);
            return NULL;
        }
        major = dri3_reply->major_version;
        minor = dri3_reply->minor_version;
        free(dri3_reply);
    }

#endif

    pdpyp = (__DRIdisplayPrivate *)Xmalloc(sizeof(__DRIdisplayPrivate));
    if (!pdpyp) {
        return NULL;
    }
    pdpyp->driMajor = major;
    pdpyp->driMinor = minor;
    pdpyp->driPatch = patch;
    pdpyp->dri3 = dri3;

    pdisp->destroyDisplay = (TDESDISP)driDestroyDisplay;

    /* allocate array of pointers to createScreen funcs */
    pdisp->createScreen = (CreateScreenFunc *) Xmalloc(numScreens * sizeof(GLvoid *));
    if (!pdisp->createScreen) {
        XFree(pdpyp);
        return NULL;
    }

    /* allocate array of pointers to createScreen funcs */
    pdisp->createNewScreen = (CreateNewScreenFunc *) Xmalloc(numScreens * sizeof(GLvoid *));
    if (!pdisp->createNewScreen) {
        Xfree(pdisp->createScreen);
        Xfree(pdpyp);
        return NULL;
    }

    /* allocate array of library handles */
    pdpyp->libraryHandles = (GLvoid **) Xmalloc(numScreens * sizeof(GLvoid*));
    if (!pdpyp->libraryHandles) {
        Xfree(pdisp->createScreen);
        XFree(pdpyp);
        return NULL;
    }

    /* dynamically discover DRI drivers for all screens, saving each
     * driver's "__driCreateScreen" function pointer.  That's the bootstrap
     * entrypoint for all DRI drivers.
     */
    for (scrn = 0; scrn < numScreens; scrn++) {
        __DRIdriver *driver = GetDriver(dpy, scrn);
        driver->refCount++;
        if (driver) {
            pdisp->createScreen[scrn] = driver->createScreenFunc;
            pdisp->createNewScreen[scrn] = driver->createNewScreenFunc;
            pdpyp->libraryHandles[scrn] = driver->handle;
        }
        else {
            pdisp->createScreen[scrn] = (CreateScreenFunc)DummyCreateScreen;
            pdisp->createNewScreen[scrn] = NULL;
            pdpyp->libraryHandles[scrn] = NULL;
        }
    }

    return (GLvoid *)pdpyp;
}
