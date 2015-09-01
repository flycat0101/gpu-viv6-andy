#!/bin/sh

#Build Configuration
#-------------------
export AQROOT=$PWD/../../..
export CFLAGS='-Wa,-mimplicit-it=thumb -lm -ldl -ldrm -lX11'

cp xf86drm.h /usr/include

./autogen.sh --prefix=/usr --libdir '/usr/lib'  --disable-static

echo "End of configuration"
