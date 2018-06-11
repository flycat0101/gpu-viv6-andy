#!/bin/bash

platform=$1
bits=$2
dstDir=$3
conf=$4

# Folder with compiled GPU binaries
binDir32="platform_binaries/armle-v7/iMX"
binDir64="platform_binaries/aarch64le/iMX"

# Folder with default graphics.conf files
cfgDir="platform_config/iMX"

# Common file list for all platforms
fileListCommon=(libCLC.so libEGL_viv.so libGalcore.so libGAL.so libGLES_CL_viv.so libGLES_CM_viv.so libGLESv2_viv.so libGLSLC.so libLLVM_viv.so libOpenCL.so libOpenVG_viv.so libVDK.so libVSC.so libOpenVG_viv.so.3d)

# OpenVG libs for VG core (only for iMX6X)
fileListVG=(libOpenVG_viv.so.2d)

# OpenVX for iMX8
fileListVX=(libOpenVX.so libOpenVXU.so)

# These files will be renamed to .so.1 and link .so will be created (if it exists in destination directory)
linkList=(libOpenCL.so libVSC.so libEGL_viv.so libGAL.so libCLC.so libOpenVX.so libOpenVXU.so libLLVM_viv.so libVDK.so)


function printHelp {
  echo
  echo "Install script for common GPU driver."
  echo "Usage:"
  echo "   install_common_driver.sh platform bits destination config"
  echo "   platform    : iMX6X, iMX6DQP, iMX6DLS, iMX6SX, iMX8DV, iMX8QM, iMX8QXP, s32v234"
  echo "   bits        : 32 or 64"
  echo "   destination : install dir (will be created automatically)"
  echo "   config      : keep: don\'t overwrite graphics conf"
  echo "                 new : graphics.conf will be replaced with default one"
  echo "Warning:"
  echo "GPU binaries in destination folder will be overwritten!"
  echo
  echo "Example:"
  echo "./install_common_driver.sh 32 iMX6X /nfs_root/usr/lib/graphics/iMX6X new"
  echo
}


function makeLink {
  local fname=$1

  if [ -e $fname ]; then
    mv $fname $fname.1
    ln -sv $fname.1 $fname
  fi
}

function errorStop {
  local text=$1

  echo $text
  printHelp
  exit 1
}

#check bits
if [ "$bits" == "32" ]; then
  binDir=$binDir32
elif [ "$bits" == "64" ]; then
  binDir=$binDir64
else
  errorStop "Error: unknown bits parameter "$bits"."
fi



#check platform
case "$platform" in
  "iMX6X" | "iMX6DQP")
    fileList=("${fileListCommon[@]}" "${fileListVG[@]}")
    hasVG=1
  ;;
  "iMX6DLS" | "iMX6SX" | "s32v234")
    fileList=("${fileListCommon[@]}")
    hasVG=0
  ;;
  "iMX8DV" | "iMX8QM" | "iMX8QXP")
    fileList=("${fileListCommon[@]}" "${fileListVX[@]}")
    hasVG=0
  ;;
  *)
  errorStop "Error: unknown platform parameter "$platform"."
esac

#check dirs
[[ ! -d "$binDir" ]] && errorStop "Error: GPU binaries folder not found ($binDir)"
[[ ! -d "$cfgDir" ]] && errorStop "Error: GPU platform config folder not found ($binDir)";

echo "============================================="
echo "         GPU driver installer                "
echo "============================================="
echo "Platform: $platform"
echo "Source: $binDir"
echo "Config: $cfgDir"
echo "Destination: $platform"
echo

echo "Creating destination directory $dstDir..."
if ! mkdir -pv $dstDir; then
  errorStop "Error: Failed to create $dstDir directory."
fi
echo

echo "Cleaning destination directory $dstDir:"
for f in "${fileList[@]}"; do
  rm -vf $dstDir/$f
  rm -vf $dstDir/$f.1
done
echo

echo "Creating new files in $dstDir:"
for f in "${fileList[@]}"; do
  cp -vf $binDir/$f $dstDir/$f
done
echo

echo Updating graphics.conf...
if [ "$conf" == "new" ]; then
  cp -v $cfgDir/graphics.conf.$platform $dstDir/graphics.conf
elif [ "$conf" == "keep" ]; then
  echo "...no change."
else
  errorStop "Error: Unknown parameter config."
fi
echo

echo Configuring VG...
if [ "$hasVG" == "1" ]; then
  cp -v $binDir/libOpenVG_viv.so.2d $dstDir/libOpenVG_viv.so
elif [ "$hasVG" == "0" ]; then
  cp -v $binDir/libOpenVG_viv.so.3d $dstDir/libOpenVG_viv.so
fi
echo

echo "Driver installation done."

exit 0
