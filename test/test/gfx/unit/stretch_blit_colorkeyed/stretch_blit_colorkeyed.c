/****************************************************************************
*
*    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
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
	DFBResult			    err;
	DFBSurfaceDescription   dsc;
	DFBAccelerationMask     mask;
	DFBSurfacePixelFormat   format;
	DFBRectangle			drect;
	DFBImageDescription 	image_dsc;
	IDirectFBDisplayLayer  *layer;
	IDirectFBSurface       *primary, *source;
	IDirectFBImageProvider *provider;

	int width, height, src_width, src_height;

	/* Initialize the core. */
	DFBCHECK(DirectFBInit( &argc, &argv ));

	/* Create the super interface. */
	DFBCHECK(DirectFBCreate( &dfb ));

	/* Get the Layer Interface for the primary layer. */
	DFBCHECK(dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ));

	/* Set the layer cooperative level. */
	DFBCHECK(layer->SetCooperativeLevel( layer, DLSCL_ADMINISTRATIVE ));
	DFBCHECK(layer->SetBackgroundColor( layer, 0, 0, 0, 0xFF ));
	DFBCHECK(layer->SetBackgroundMode( layer, DLBM_COLOR ));

	/* Get the layer's surface. */
	DFBCHECK(layer->GetSurface( layer, &primary ));

	primary->GetSize( primary, &width, &height );
	primary->Clear( primary, 0, 0, 0, 0x80 );

	primary->SetBlittingFlags( primary, DSBLIT_SRC_COLORKEY );

	/* create a surface and render an image to it */
	DFBCHECK(dfb->CreateImageProvider( dfb,
									   DATADIR"/colorkeyed.gif",
									   &provider ));

	DFBCHECK(provider->GetSurfaceDescription( provider, &dsc ));
	DFBCHECK(primary->GetPixelFormat( primary, &format ));

	dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
	dsc.pixelformat = format;

	src_width  = dsc.width;
	src_height = dsc.height;

	DFBCHECK(dfb->CreateSurface( dfb, &dsc, &source ));

	DFBCHECK(provider->RenderTo( provider, source, NULL ));

	DFBCHECK(provider->GetImageDescription( provider, &image_dsc ));

	if (image_dsc.caps & DICAPS_COLORKEY) {
		DFBCHECK(source->SetSrcColorKey( source,
										 image_dsc.colorkey_r,
                    					 image_dsc.colorkey_g,
                    					 image_dsc.colorkey_b ));
	}

	DFBCHECK(provider->Release( provider ));

	DFBCHECK(primary->GetAccelerationMask( primary, source, &mask ));

	if (mask & DFXL_STRETCHBLIT) {
		printf( "DFXL_STRETCHBLIT is accelerated.\n" );
	} else {
		printf( "DFXL_STRETCHBLIT is NOT accelerated.\n" );
	}

	drect.x = 0;
	drect.y = 0;
	drect.w = src_width / 2;
	drect.h = src_height / 2;

	DFBCHECK(primary->StretchBlit( primary, source, NULL, &drect ));

	drect.x = src_width / 2;
	drect.y = src_height / 2;
	drect.w = src_width * 3 / 2;
	drect.h = src_height * 3 / 2;

	DFBCHECK(primary->StretchBlit( primary, source, NULL, &drect ));

	DFBCHECK(primary->Flip( primary, NULL, DSFLIP_NONE ));

	/* Save the picture. */
	DFBCHECK(primary->Dump( primary, ".", "stretch_blit_colorkeyed" ));

	/* Make sure all of the hardware operations have completed. */
	DFBCHECK(dfb->WaitIdle( dfb ));

	/* Release the resources. */
	DFBCHECK(source->Release( source ));
	DFBCHECK(primary->Release( primary ));
	DFBCHECK(layer->Release( layer ));
	DFBCHECK(dfb->Release( dfb ));

	return EXIT_SUCCESS;
}

