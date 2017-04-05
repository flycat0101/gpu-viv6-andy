#!/bin/bash

platform=$1
dstDir=$2
vg=$3
conf=$4

binDir="platform_binaries/armle-v7/iMX"
cfgDir="platform_config/iMX"
fileList=(libCLC.so libEGL_viv.so libGalcore.so libGAL.so libGLES_CL_viv.so libGLES_CM_viv.so libGLESv2_viv.so libGLSLC.so libLLVM_viv.so libOpenCL.so libOpenVG_viv.so libVDK.so libVSC.so)
fileListVX=(libOpenVX.so libOpenVXU.so)

function printHelp {
  echo
  echo "Install script for common GPU driver. Usage:"
  echo "   install_common_driver.sh platform destination VGtype configType"
  echo "   platform    : iMX6X or iMX8DV"
  echo "   destination : install dir (will be created automatically)"
  echo "   VGtype      : 2D (only for iMX6X) or 3D"
  echo "   configType  : keep : don\'t overwrite graphics conf"
  echo "                 otherwise: graphics.conf will be replaced with default one"
  echo
}

function makeLink {
  local fname=$1
  mv -v $fname $fname.1
  ln -sv $fname.1 $fname
}


function errorStop {
  local text
  echo $text
  printHelp
  exit 1
}


echo

# parameter check
[[ ! $platform == "iMX6X" ]] && [[ $platform != "iMX8DV" ]] && errorStop "Error: Unknown value for platform parameter!"
[[ ! $vg == "2D" ]] && [[ $vg != "3D" ]] && errorStop "Error: Unknown value for VGtype parameter!"
[[ ! $conf == "keep" ]] && [[ $conf != "new" ]] && errorStop "Error: Unknown value for configType parameter!"

mkdir -pv $dstDir

echo "Cleaning destination directory $dstDir:"
for f in "${fileList[@]}" "${fileListVX[@]}"; do
  rm -vf $dstDir/$f
  rm -vf $dstDir/$f.1
done
echo

echo "Creating new files in $dstDir:"
for f in "${fileList[@]}"; do
  cp -vf $binDir/$f $dstDir/$f
done

echo

if [ "$platform" == "iMX6X" ]; then

  echo Platform config iMX6X...

  if [ "$conf" != "keep" ]; then
    cp -v $cfgDir/graphics.conf.imx6x $dstDir/graphics.conf
  fi
  echo

  if [ "$vg" == "2D" ]; then
    echo VG library for VG core...
    cp -vf $binDir/libOpenVG_viv.so.2d $dstDir/libOpenVG_viv.so
  elif [ "$vg" == "3D" ]; then
    echo VG emulated on 3D core...
    cp -vf $binDir/libOpenVG_viv.so.3d $dstDir/libOpenVG_viv.so
  fi

elif [ "$platform" == "iMX8DV" ]; then

  echo Platform config iMX8DV...
  if [ "$conf" != "keep" ]; then
    cp -v $cfgDir/graphics.conf.imx8dv $dstDir/graphics.conf
  fi
  echo

  echo Parameter $vg ignored, VG library emulated on 3D core...
  cp -vf $binDir/libOpenVG_viv.so.3d $dstDir/libOpenVG_viv.so
  echo

  echo Adding OpenVX...
  for f in "${fileListVX[@]}"; do
    cp -vf $binDir/$f $dstDir/$f
  done

fi

echo
echo Creating links...

pushd $dstDir
makeLink libCLC.so
makeLink libEGL_viv.so
makeLink libGAL.so
makeLink libLLVM_viv.so
makeLink libOpenCL.so
makeLink libVSC.so
makeLink libVDK.so
if [ "$platform" == "iMX8DV" ]; then
  makeLink libOpenVX.so
  makeLink libOpenVXU.so
fi
popd
echo