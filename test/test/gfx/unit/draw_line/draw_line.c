/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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
	DFBAccelerationMask     mask;
	IDirectFBDisplayLayer  *layer;
	IDirectFBSurface       *primary;

	DFBRegion lines[8];
	int width, height;

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

	primary->SetDrawingFlags( primary, DSDRAW_NOFX );

	DFBCHECK(primary->GetAccelerationMask( primary, NULL, &mask ));

	if (mask & DFXL_DRAWLINE) {
		printf( "DFXL_DRAWLINE is accelerated.\n" );
	} else {
		printf( "DFXL_DRAWLINE is NOT accelerated.\n" );
	}

	lines[0].x1 = 0;
	lines[0].y1 = 0;
	lines[0].x2 = width;
	lines[0].y2 = height;

	lines[1].x1 = width;
	lines[1].y1 = 0;
	lines[1].x2 = 0;
	lines[1].y2 = height;

	lines[2].x1 = 0;
	lines[2].y1 = height/2;
	lines[2].x2 = width;
	lines[2].y2 = height/2;

	lines[3].x1 = width/2;
	lines[3].y1 = 0;
	lines[3].x2 = width/2;
	lines[3].y2 = height;

	lines[4].x1 = width/4;
	lines[4].y1 = 0;
	lines[4].x2 = width/4;
	lines[4].y2 = height;

	lines[5].x1 = width*3/4;
	lines[5].y1 = 0;
	lines[5].x2 = width*3/4;
	lines[5].y2 = height;

	lines[6].x1 = 0;
	lines[6].y1 = height/4;
	lines[6].x2 = width;
	lines[6].y2 = height/4;

	lines[7].x1 = 0;
	lines[7].y1 = height*3/4;
	lines[7].x2 = width;
	lines[7].y2 = height*3/4;

	DFBCHECK(primary->SetColor( primary, 0xFF, 0, 0, 0xFF ));

	DFBCHECK(primary->DrawLines( primary, lines, 8 ));

	DFBCHECK(primary->Flip( primary, NULL, DSFLIP_NONE ));

	/* Save the picture. */
	DFBCHECK(primary->Dump( primary, ".", "draw_line" ));

	/* Make sure all of the hardware operations have completed. */
	DFBCHECK(dfb->WaitIdle( dfb ));

	/* Release the resources. */
	DFBCHECK(primary->Release( primary ));
	DFBCHECK(layer->Release( layer ));
	DFBCHECK(dfb->Release( dfb ));

	return EXIT_SUCCESS;
}
