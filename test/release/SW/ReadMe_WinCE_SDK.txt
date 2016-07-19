Files layout
============
\VIVANTE_SDK
|   ReadMe.txt
|
\---samples
    |
    +---hal
    |    |
    |    +---tvui
    |    |     tvui.exe
    |    |
    |    \---unit_test
    |          galRunTest2.exe
    |
    +---tiger
    |     gctiger.exe
    |
    +---egl
    |    |
    |    \---WINCE
    |        pixmapDIBTex.exe
    |        pixmapDDBTex.exe
    |        pixmapDIB.exe
    |        pixmapDDB.exe
    |        ddsOverlay.exe
    |
    +---es20
    |    |
    |    +---vv_launcher
    |    |     es20_vv_launcher.exe
    |    |
    |    +---shareContext
    |    |     shareContext_es20.exe
    |    |
    |    +---tutorials
    |    |     tutorial1.exe
    |    |     tutorial2.exe
    |    |     tutorial3.exe
    |    |     tutorial4.exe
    |    |     tutorial5.exe
    |    |     tutorial6.exe
    |    |     tutorial7.exe
    |    |
    |    \---samples
    |          vdksample1_es20
    |          vdksample2_es20
    |          vdksample3_es20
    |          vdksample4_es20
    |          vdksample5_es20
    |          vdksample6_es20
    |          vdksample7_es20
    |          vdksample8_es20
    |          vdksample9_es20
    |          vdksample10_es20
    |
    \---es11
         |
         +---cover_flow
         |     cover_flow.exe
         |
         \---tutorials
               tutorial1.exe
               tutorial2.exe
               tutorial3.exe
               tutorial4.exe
               tutorial5.exe
               tutorial6.exe
               tutorial7.exe

Running applications on the target machine
==========================================

1. Added the kernel driver of HAL(libGalCore.dll) to WinCE OS image.
   1). Clean and build your WinCE OS project;
   2). Add the content of PBUserProjects.reg under sdk\bin to the %_FLATRELEASEDIR%\PBUserProjects.reg of WinCE OS project;
   3). Add the content of PBUserProjects.bib under sdk\bin to the %_FLATRELEASEDIR%\PBUserProjects.bib of WinCE OS project;
   4). Make run-time image;

2. Copy other VIVANTE driver to the %_FLATRELEASEDIR% directory of WinCE OS project.

3. Copy Samples to the path: %_FLATRELEASEDIR% of WinCE OS project.

4. Load WinCE OS image on your target machine, then run the application
        eg.
        cd \release\Samples\es11\tutorials
        tutorial1.exe
