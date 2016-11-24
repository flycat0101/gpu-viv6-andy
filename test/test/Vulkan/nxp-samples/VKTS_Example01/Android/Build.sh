#!/bin/bash
#

# Copy assets.
# Nothing for this example.

cd jni
if test -n "$APP_ABI"; then
  ndk-build APP_ABI="$APP_ABI" -j4
else
  ndk-build -j4
fi

cd ..
ant debug

# adb install -r bin/NativeActivity-debug.apk
