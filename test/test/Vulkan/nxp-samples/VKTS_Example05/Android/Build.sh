#!/bin/bash
#

# Copy assets.
mkdir -p ./assets/shader/SPIR/V/
mkdir -p ./assets/cartoon_tree_with_plane/
cp -f ../../VKTS_Binaries/shader/SPIR/V/phong.vert.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/shader/SPIR/V/phong.frag.spv ./assets/shader/SPIR/V/
cp -f ../../VKTS_Binaries/cartoon_tree_with_plane/*.vkts ./assets/cartoon_tree_with_plane/
cp -f ../../VKTS_Binaries/cartoon_tree_with_plane/*.tga ./assets/cartoon_tree_with_plane/

cd jni
if test -n "$APP_ABI"; then
  ndk-build APP_ABI="$APP_ABI" -j4
else
  ndk-build -j4
fi

cd ..
ant debug

#adb install -r bin/NativeActivity-debug.apk
