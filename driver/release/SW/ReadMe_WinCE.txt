WinCE Build for ARM

Contents
========
1. Prerequisite
2. Installation
3. Building
4. Generate VIVANTE_SDK

1. Prerequisite
===============

Windows Embedded Compact solution with target BSP


2. Installation
===============

I. create %WINCEROOT%/public/gchal directory;

II. Extract the source code from VIVANTE_GAL_Unified_Src_drv_<version>.tgz package to %WINCEROOT%/public/gchal;

III. Change the gchal.reg under %WINCEROOT%/public/gchal to add system information for GPU driver.

3. Building
===========

I. Open the Windows Embedded Compact solution with target BSP;

II. Add Vivante drvier to the solution by selecting "Catalog/Third Party/Device Drivers/Displays/GCHAL" in Catalog Items View window;

III. Build gchal project and do sysgen;

IV. Make runtime image;

V. Attach the device and download image;

4. Generate VIVANTE_SDK
=======================

This step is used to help build test suite. Once the driver has been built
sucessfully, you need to copy them to several target folders then the test
project could be compiled correctly. You can specify the target folders using
these three Environment Variables, VIVANTE_SDK_DIR VIVANTE_SDK_INC and
VIVANTE_SDK_LIB. If you don't set them, we will use their default values:

    VIVANTE_SDK_DIR=%AQROOT%/VIVANTE_SDK
    VIVANTE_SDK_INC=%VIVANTE_SDK_DIR%/inc
    VIVANTE_SDK_LIB=%VIVANTE_SDK_DIR%/lib

Then you can use cegenvivsdk.bat, which is lied in the same folder with this
documentation, to copy driver and
some necessary head files to specified target folders.

I. Open "Platform Builder"

II. Click "Build OS"/"Open Release Directory"

III. Run cegenvivsdk.bat in the opened command window.

Then you can go to build test suite if you want.

