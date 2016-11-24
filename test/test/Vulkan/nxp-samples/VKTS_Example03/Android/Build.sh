#!/bin/bash
#

# Copy assets.
mkdir -p ./assets/shader/SPIR/V/
mkdir -p ./assets/texture/
cp -f ../../VKTS_Binaries/shader/SPIR/V/texture_ndc.vert.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/shader/SPIR/V/texture.frag.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/texture/crater_lake.tga ./assets/texture/

cd jni
if test -n "$APP_ABI"; then
  ndk-build APP_ABI="$APP_ABI" -j4
else
  ndk-build -j4
fi

cd ..
ant debug

#adb install -r bin/NativeActivity-debug.apk
