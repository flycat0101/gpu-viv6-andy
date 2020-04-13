/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include <directfb.h>
#include <directfb_strings.h>
#include <directfb_util.h>
#include <direct/util.h>

#include <stdio.h>     /* for fprintf()      */

/* the super interface */
static IDirectFB *dfb;

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...)                                                     \
          do {                                                             \
               err = x;                                                    \
               if (err != DFB_OK) {                                        \
                    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
                    DirectFBErrorFatal( #x, err );                         \
               }                                                           \
          } while (0)

int main( int argc, char *argv[] )
{
    DFBResult                err;
    DFBAccelerationMask     mask;
    IDirectFBDisplayLayer  *layer;
    IDirectFBSurface       *primary;

    int width, height, w, h, x, y;

    /* Initialize the core. */
    DFBCHECK(DirectFBInit( &argc, &argv ));

    /* Create the super interface. */
    DFBCHECK(DirectFBCreate( &dfb ));

    /* Get the Layer Interface for the primary layer. */
    DFBCHECK(dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ));

    /* Set the layer cooperative level. */
    DFBCHECK(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ));
    DFBCHECK(layer->SetBackgroundColor( layer, 0, 0, 0, 0xff ));
    DFBCHECK(layer->SetBackgroundMode( layer, DLBM_COLOR ));

    /* Get the layer's surface. */
    DFBCHECK(layer->GetSurface( layer, &primary ));

    DFBCHECK(primary->GetSize( primary, &width, &height ));
    DFBCHECK(primary->Clear( primary, 0, 0, 0, 0x80 ));

    DFBCHECK(primary->SetDrawingFlags( primary, DSDRAW_NOFX ));

    DFBCHECK(primary->GetAccelerationMask( primary, NULL, &mask ));

    if (mask & DFXL_DRAWRECTANGLE) {
        printf( "DFXL_DRAWRECTANGLE is accelerated.\n" );
    } else {
        printf( "DFXL_DRAWRECTANGLE is NOT accelerated.\n" );
    }

    w = width / 2;
    h = height / 2;
    x = (width - w) / 2;
    y = (height - h) / 2;

    DFBCHECK(primary->SetColor( primary, 0x0F, 0, 0xF0, 0xFF ));

    DFBCHECK(primary->DrawRectangle( primary, x, y, w, h ));

    DFBCHECK(primary->Flip( primary, NULL, DSFLIP_NONE ));

    /* Save the picture. */
    DFBCHECK(primary->Dump( primary, ".", "draw_rectangle" ));

    /* Make sure all of the hardware operations have completed. */
    DFBCHECK(dfb->WaitIdle( dfb ));

    /* Release the resources. */
    DFBCHECK(primary->Release( primary ));
    DFBCHECK(layer->Release( layer ));
    DFBCHECK(dfb->Release( dfb ));

    return EXIT_SUCCESS;
}
