Linux Build for Vivante's Graphics Testsuite

Contents

1. Quick start
2. Prerequisite
3. Build
4. Run the applications

1. Quick start
==============

1) Uncompress the source code, and set the root directory of test source code.
   # Please make sure driver and test source package are unzipped in different
   # directory.
   # <PROJECTS_DIR> must be a full path.
   export PROJECTS_TEST_DIR=<PROJECTS_DIR>/TEST
   mkdir -p $PROJECTS_TEST_DIR
   tar zxvf VIVANTE_GAL_Unified_Src_tst_<version>.tgz -C $PROJECTS_TEST_DIR.

   # Just set the root directory of test source code.
   # Do not care $AQARCH any more for test building.
   export AQROOT=$PROJECTS_TEST_DIR

   # Notice: the path of the AQROOT at here is different with the path where
   # driver source code was installed.

2) Set Vivante driver SDK environment variables.
   # Tests building depends on Vivante driver SDK.
   # Set 3 environment variables according to the path where your driver SDK
   # was installed to make sure the tool chain can find Vivante driver SDK.
   export VIVANTE_SDK_DIR=<DRIVER_SDK_DIR>
   export VIVANTE_SDK_INC=<DRIVER_SDK_DIR>/include
   export VIVANTE_SDK_LIB=<DRIVER_SDK_DIR>/drivers

3) Set other environment variables.

   # Set cross compile toolchain.
   export CROSS_COMPILE=<CROSS_COMPILER_PREFIX>

   # Set the Linux kernel directory.
   export KERNEL_DIR=<PATH_TO_LINUX_KERNEL_SOURCE>

   # Set X11 installation path.
   # If you are not using X11 as the GUI system, this variable is not necessary.
   export X11_ARM_DIR=<PATH_TO_X11_STUFF>

   # Set DirectFB installation path.
   # Exported this variable to support DirectFB accelerator building;
   # Set it to empty to skip DirectFB accelerator building
   export DFB_DIR=<PATH_TO_DIRECTFB_INSTALLATION_DIRECTORY>

   # Set the toolchain path.
   export TOOLCHAIN=<PATH_TO_TOOLCHAIN_TOPDIR>

   # Set the static library path which contains libc.a, etc.
   # If you are not building the package with static link, this variable is not necessary.
   export LIB_DIR=<PATH_TO_STATIC_LIBRARIES>

   # The binaries will be installed at $SDK_DIR.
   # By default, SDK_DIR=$AQROOT/build/sdk, you can modify it as following:
   TEST_SDK_DIR=<PATH_TO_INSTALL_BINARIES>
   export SDK_DIR=$TEST_SDK_DIR

   # Add the toolchain to the PATH.
   export PATH=$TOOLCHAIN/bin:$PATH

   # We use arm-2010q1 toolchain to compile this package.

3) Build.

   cd $AQROOT

   make -f makefile.linux
   make -f makefile.linux install

   The binaries are installed at <SDK_DIR>.
   More build options please see 'Build commands'.

4) Run the applications.

   Move to the target board.

   a) Copy <SDK_DIR> to the target board.

   b) Create device node.
      mknod /dev/galcore c 199 0

   c) Insert the kernel driver.
      insmod <VIVANTE_SDK_LIB>/galcore.ko registerMemBase=<REG_MEM_BASE> irqLine=<IRQ> contiguousSize=<CONTIGUOUS_MEM_SIZE>

   d) Set environment variable.

      export LD_LIBRARY_PATH=<VIVANTE_SDK_LIB>

   e) Run the application.

	  eg. Run tutorial1.

	  cd <SDK_DIR>/samples/tutorial
	  ./tutorial1

2. Prerequisite
===============

1) Get the Vivante driver SDK

   Before building the Vivante test suites, you need to get or build Vivante driver SDK.


3. Build
=========

1) Build commands.

   a) Build
   make -f makefile.linux <OPTIONS>

   b) Install
   make -f makefile.linux <OPTIONS> install

   c) Clean

   make -f makefile.linux <OPTIONS> clean

   Please see section 3) for the details on OPTIONS.

2) The targets

   There are a lot of modules in the package.

   Use the following the commands to build, install and clean a specific module.

   a) Build
   make -f makefile.linux <OPTIONS> <MODULE>

   b) Install
   make -f makefile.linux <OPTIONS> <MODULE> V_TARGET=install

   c) Clean

   make -f makefile.linux <OPTIONS> <MODULE> V_TARGET=clean

   MODULE list:

   hal_drv:         Build HAL driver, including galcore.ko, libGAL.so.
   gfx_test:		Build DirectFB samples.
   vdktest:         Build VDK samples, including OpenGL ES 1.1/2.0 tutorials.
   tiger:           Build OpenVG 1.1 sample: tiger.

   Please see the section 3) for the details on OPTIONS.

3) The options

   There are a lot of OPTIONS to control how to build the driver.
   To enable/disable an option, set <OPTION>=<value> in the command line.
    option                 value      description
    -------------------------------------------------
    DEBUG                    0        Disable debugging;default value;
                             1        Enable debugging;
    CPU_TYPE           [CPU type]     Set -mcpu=[CPU type] in the build command line.	0

   	                         0        Use the default value - arm920.

    NO_DMA_COHERENT          0        Enable coherent DMA;default value;
                             1        Disable coherent DMA;

    ABI                      0        Change application binary interface;default is 0 which means no setting;
                       aapcs-linux    For example, build driver for Aspenite board;

    USE_VDK                  0        Don't use the VDK programming interface;default value;
                             1        Use the VDK programming interface;
                                      Don't support static linking;
                                      Supports FBDEV and X11 system;

    EGL_API_FB               0        Use X11 system as the GUI system for the EGL;default value;
                             1        Use the FBDEV as the GUI system for the EGL;
                                      Supports static linking with STATLIC_LINK=1 build option;

    gcdSTATIC_LINK              0        Disable static linking;default value;
                                      Don't affects on DirectFB accelerator.
                             1        Enable static linking;
                                      Don't affects on DirectFB accelerator.

    CUSTOM_PIXMAP            0        Don't use special pixmap surface format;default value;
                             1        Use special pixmap surface format;
                                      Only affects on EGL driver;


4. Run the applications
=======================

Move to the target board.

1) Copy <SDK_DIR> to the target board.

2) Create the device node.

   mknod /dev/galcore c 199 0

3) Insert the kernel driver.

   There are two kinds of ways to manage the video memory: pre-reserved and
   dynamically allocated.

   We must pass the correct parameters to the driver according to these two conditions.

   a) Reserve a piece of memory from the system.

      insmod <SDK_DIR>/drivers/galcore.ko registerMemBase=<REG_MEM_BASE> irqLine=<IRQ> contiguousBase=<CONTIGUOUS_MEM_BASE> contiguousSize=<CONTIGUOUS_MEM_SIZE>

   b) Allocate a piece of memory dynamically when the driver boots up.

      insmod <SDK_DIR>/drivers/galcore.ko registerMemBase=<REG_MEM_BASE> irqLine=<IRQ> contiguousSize=<CONTIGUOUS_MEM_SIZE>

4) Set environment variable.

   export LD_LIBRARY_PATH=<SDK_DIR>/drivers

   Add additional paths to LD_LIBRARY_PATH if you are use other extra libraries.

5) Run the application.

   eg. Run tutorial1.

   cd <SDK_DIR>/samples/tutorial
   ./tutorial1

