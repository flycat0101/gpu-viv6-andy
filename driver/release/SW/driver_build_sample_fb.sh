#!/bin/bash

export BUILD_OPTION_EGL_API_FB=1

./driver_build_sample.sh $*

unset BUILD_OPTION_EGL_API_FB
