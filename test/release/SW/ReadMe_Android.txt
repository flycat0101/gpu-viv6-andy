Vivante's Graphics Tests Build Instruction for Android
======================================================

  1) Ensure you have installed Vivante driver source package and build out all drivers successfully;
      All tests depends on Vivante's graphics driver libraries;
      To build driver, please refert to another driver build instruction: 'Android driver Build for Vivante's Graphics Drivers';

  2) Exported $AQROOT environment variable which direct to Vivante driver source directory;
      All tests reference $AQROOT/sdk/inc and $AQROOT/hal/inc to search Vivante graphics driver header files;

  3) Install Vivante test: VIVANTE_GAL_Unified_Src_tst_<version>.tgz.
     Install Vivante test in one of directory like <PROJECTS_DIR>
     under Android sdk directory, e.g.

       mkdir <ANDROID_BUILD_TOP>/<PROJECTS_DIR>
       cd <ANDROID_SDK_TOP_DIR>
       tar zxvf VIVANTE_GAL_Unified_Src_tst_<version>.tgz -C <PROJECTS_DIR>

  5) Set up Android build environment as same as building Vivante's graphics driver, typicaly,

        . build/envsetup.sh
        chooseprodcut <productname>

  6) Build Vivante test.

        cd <PROJECTS_DIR>/sdk/samples
        mm -B

        cd <PROJECTS_DIR>/sdk/test
        mm -B
