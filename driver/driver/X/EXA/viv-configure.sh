#!/bin/sh

#Build Configuration
#-------------------
export AQROOT=$PWD/../../..
export LDFLAGSVIV="${AQROOT}/hal/user/$(OBJ_DIR) -lGAL -lm -ldl"
export CFLAGS='-I${AQROOT}/hal/inc -L${LDFLAGSVIV}'

./autogen.sh --prefix=/usr --libdir '/usr/lib'  --disable-static

echo "End of configuration"

