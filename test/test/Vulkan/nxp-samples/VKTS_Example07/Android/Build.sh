#!/bin/bash
#

# Copy assets.
mkdir -p ./assets/shader/SPIR/V/
mkdir -p ./assets/valley_terrain/
cp -f ../../VKTS_Binaries/shader/SPIR/V/phong_displace.vert.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/shader/SPIR/V/phong_displace.tesc.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/shader/SPIR/V/phong_displace.tese.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/shader/SPIR/V/phong_displace.geom.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/shader/SPIR/V/phong_displace.frag.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/valley_terrain/*.vkts ./assets/valley_terrain/
cp -f ../../VKTS_Binaries/valley_terrain/*.tga ./assets/valley_terrain/

cd jni
if test -n "$APP_ABI"; then
  ndk-build APP_ABI="$APP_ABI" -j4
else
  ndk-build -j4
fi

cd ..
ant debug

#adb install -r bin/NativeActivity-debug.apk
