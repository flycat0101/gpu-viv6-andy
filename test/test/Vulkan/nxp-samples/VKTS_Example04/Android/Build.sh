#!/bin/bash
#

# Copy assets.
mkdir -p ./assets/shader/SPIR/V/
mkdir -p ./assets/textured_cube/
cp -f ../../VKTS_Binaries/shader/SPIR/V/vertex_normal_texcoord.vert.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/shader/SPIR/V/texture_light.frag.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/textured_cube/*.vkts ./assets/textured_cube/
cp -f ../../VKTS_Binaries/textured_cube/*.tga ./assets/textured_cube/

cd jni
if test -n "$APP_ABI"; then
  ndk-build APP_ABI="$APP_ABI" -j4
else
  ndk-build -j4
fi

cd ..
ant debug

#adb install -r bin/NativeActivity-debug.apk
