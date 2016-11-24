#!/bin/bash
#

# See README.md
# Requires vulkan header file and libraries to be copied (or linked)
# from android-24 to android-23

# NDK
#export PATH=$PATH:/path/to/android-ndk

# SDK
#export PATH=$PATH:/path/to/android-sdk/tools

# ANT
#export PATH=$PATH:/path/to/apache-ant/bin

# Specify ABIs to support
#export APP_ABI="x86 armeabi-v7a-hard arm64-v8a"
export APP_ABI="armeabi-v7a-hard"

cd VKTS/Android/
./Build.sh
cd -

cd VKTS_Example01/Android
./Init.sh
./Build.sh
cd -

cd VKTS_Example02/Android
./Init.sh
./Build.sh
cd -

cd VKTS_Example03/Android
./Init.sh
./Build.sh
cd -

cd VKTS_Example04/Android
./Init.sh
./Build.sh
cd -

cd VKTS_Example05/Android
./Init.sh
./Build.sh
cd -

cd VKTS_Example06/Android
./Init.sh
./Build.sh
cd -

cd VKTS_Example07/Android
./Init.sh
./Build.sh
cd -

cd VKTS_Example08/Android
./Init.sh
./Build.sh
cd -

cd VKTS_NXP_BirdSwarmDemo/Android
./Init.sh
./Build.sh
cd -

mkdir bin
cp ./VKTS_Example01/Android/bin/NativeActivity-debug.apk Android_APKs/Example01-debug.apk
cp ./VKTS_Example02/Android/bin/NativeActivity-debug.apk Android_APKs/Example02-debug.apk
cp ./VKTS_Example03/Android/bin/NativeActivity-debug.apk Android_APKs/Example03-debug.apk
cp ./VKTS_Example04/Android/bin/NativeActivity-debug.apk Android_APKs/Example04-debug.apk
cp ./VKTS_Example05/Android/bin/NativeActivity-debug.apk Android_APKs/Example05-debug.apk
cp ./VKTS_Example06/Android/bin/NativeActivity-debug.apk Android_APKs/Example06-debug.apk
cp ./VKTS_Example07/Android/bin/NativeActivity-debug.apk Android_APKs/Example07-debug.apk
cp ./VKTS_Example08/Android/bin/NativeActivity-debug.apk Android_APKs/Example08-debug.apk
cp ./VKTS_NXP_BirdSwarmDemo/Android/bin/NativeActivity-debug.apk Android_APKs/BirdSwarmDemo-debug.apk

