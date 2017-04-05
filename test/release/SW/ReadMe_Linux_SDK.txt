Files layout
============
<SDK>/
|
\---samples
    |
    +---es20
    |      |
    |      \---vv_launcher
    |              vv_launcher
    |
    +---gfx (DirectFB case)
    |        blit
    |        blit_blend_alpha_chanel
    |        blit_blend_global_alpha
    |        blit_colorkeyed
    |        blit_convert
    |        blit_rotate180
    |        blit_rotate270
    |        blit_rotate90
    |        blit_xor
    |        draw_line
    |        draw_rectangle
    |        draw_xor
    |        fill_rectangle
    |        smooth_scale
    |        stretch_blit
    |        stretch_blit_colorkeyed
    |        stretch_blit_rotate180
    |        stretch_blit_rotate270
    |        stretch_blit_rotate90
    |
    +---hal
    |      |  tvui (2D case)
    |      |
    |      \---unit_test (2D case)
    |
    +---tiger
    |       tiger
    |
    \---vdk
        |  tutorial1
        |  tutorial2
        |  tutorial3
        |  tutorial4
        |  tutorial5
        |  tutorial6
        |  tutorial7
        |  tutorial1_es20
        |  tutorial2_es20
        |  tutorial3_es20
        |  tutorial4_es20
        |  tiger
        |
        \---samples_es20
                vdksample10_es20
                vdksample1_es20
                vdksample2_es20
                vdksample3_es20
                vdksample4_es20
                vdksample5_es20
                vdksample6_es20
                vdksample7_es20
                vdksample8_es20
                vdksample9_es20

Running applications on the target machine
==========================================

1. Copy the libraries to the target
	On the target machine:
	cp galcore.ko /
	cp libEGL.so libGLESv1_CM.so libGAL.so libVSC.so libGLSLC.so libGLESv2.so /lib

2. Install the kernel driver
	insmod /galcore.ko registerMemBase=<REG_MEM_BASE> irqLine=<IRQ> contiguousSize=<CONTIGUOUS_MEM_SIZE>

	eg. On ARM EB development board:
	insmod /galcore.ko registerMemBase=0x80000000 irqLine=104 contiguousSize=0x400000

3. Run the application
	eg.
	cd $SDK_DIR/samples/vdk; ./tutorial1


