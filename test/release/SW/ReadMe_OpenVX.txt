Linux Build for Vivante's Graphics Testsuite

Contents

1. Quick start
2. Build

1. Quick start
==============

1) Uncompress the source code, and set the root directory of test souce code.
   # Please make sure driver and test soruce package are unzipped in different
   # directory.
   # <PROJECTS_DIR> must be a full path.
   export PROJECTS_TEST_DIR=<PROJECTS_DIR>/TEST
   mkdir -p $PROJECTS_TEST_DIR
   tar zxvf VIVANTE_GAL_Unified_Src_tst_<version>.tgz -C $PROJECTS_TEST_DIR.
   tar zxvf VIVANTE_GAL_Unified_Src_tst_OVX-addon_<version>.tgz -C $PROJECTS_TEST_DIR.

   # Just set the root directory of test souce code.
   # Do not care $AQARCH any more for test building.
   export AQROOT=$PROJECTS_TEST_DIR

   # Notice: the path of the AQROOT at here is different with the path where
   # driver soruce code was installed.

2) Set Vivante driver SDK envionment variables.
   # Tests building depends on Vivante driver SDK.
   # Set 3 envionment variables according to the path where your driver SDK
   # was installed to make sure the tool chain can find Vivante driver SDK.
   # Make sure you have build out OpenCL driver successfully.
   export VIVANTE_SDK_DIR=<DRIVER_SDK_DIR>
   export VIVANTE_SDK_INC=<DRIVER_SDK_DIR>/include
   export VIVANTE_SDK_LIB=<DRIVER_SDK_DIR>/drivers

3) Setup build environment;
   please refer to the part of '3. Build' in ReadMe_Linux.txt;

2. Build

   One sepecial build options for OpenVX test need be enabled as following:

   make -f makefile.linux USE_OPENVX=1 ovx_tst V_TARGET=clean
   make -f makefile.linux USE_OPENVX=1 ovx_tst V_TARGET=install

   You can also add this build options in test_build_sample.sh;

   The binaries are installed at <SDK_DIR>.
   More build options please see 'Build commands'.

