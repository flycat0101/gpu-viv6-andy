#!/bin/bash
#

# Copy assets.
mkdir -p ./assets/shader/SPIR/V/
mkdir -p ./assets/astro_boy/
cp -f ../../VKTS_Binaries/shader/SPIR/V/phong_skinning.vert.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/shader/SPIR/V/phong_skinning.frag.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/astro_boy/*.vkts ./assets/astro_boy/
cp -f ../../VKTS_Binaries/astro_boy/*.tga ./assets/astro_boy/

cd jni
if test -n "$APP_ABI"; then
  ndk-build APP_ABI="$APP_ABI" -j4
else
  ndk-build -j4
fi

cd ..
ant debug

#adb install -r bin/NativeActivity-debug.apk
