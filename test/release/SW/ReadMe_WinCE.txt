WinCE Build for ARM

Contents
========
0. Prerequisite
1. Installation
2. Building


0. Prerequisite
===============

Before you build this test suite, you need to have a sdk packet, or you have built the driver suite already.


1. Installation
===============

I. set system wide environment variable for AQROOT to store the source tree

There is no restriction on what should the AQROOT should be set to, but normally user should
set it to be on the same disk as where WINCE is installed, such as C:\Vivante

	AQROOT=C:\Vivante

II. Extract the source code from VIVANTE_GAL_Unified_Src_tst_<version>.tgz package to be under $AQROOT


III. create %WINCEROOT%/public/gchal directory

IV. copy everything under $AQROOT\* to %WINCEROOT%/public/gchal

V. set VIVANTE_SDK_DIR in %WINCEROOT%/public/gchal
   1). after you build driver successfully, execute cevivsdkgen.bat to generate VIVANTE dirver SDK;
   2). make sure VIVANTE_SDK_DIR direct to VIVANTE dirver SDK;
     VIVANTE_SDK_DIR=$(AQROOT)\VIVANET_SDK
     VIVANTE_SDK_INC=$(VIVANTE_SDK_DIR)\inc
     VIVANTE_SDK_LIB=$(VIVANTE_SDK_DIR)\lib

2. Building
===========

I. Make sure your already build WinCE OS and get Vivante driver SDK ready;
   Refer to ReadMe_WinCE.txt in the river package.

II. Open the Windows Embedded Compact solution with target BSP;

III. Build Vivante test suite under gchal project;

Note: Make sure you do a clean build.

IV. Make runtime image in PlatformBuilder

V. Copy all resource directories under $AQROOT\resources to the release directory of OS design (defined by %_FLATRELEASEDIR%)

VI. Now you are ready to attach the device and to run the samples
