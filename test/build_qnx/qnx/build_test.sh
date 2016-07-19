#mkdir qnx-build-screen-imx6x-2014-10-SVN758947_JBN175
#tar xjf /data/home/jin.shen/qnxfiles/QNX-SDP-6.6.0/qnx-build-screen-imx6x-2014-10-SVN758947_JBN175.tar.bz2 -C qnx-build-screen-imx6x-2014-10-SVN758947_JBN175
cd /opt/qnx660/
. ./qnx660-env.sh
cd -

cd /LocalBigBox/local/fpga/ubuser/qnx-build-screen-imx6x-2014-10-SVN758947_JBN175
. ./setup_build_env.sh
cd -

export cc=make CPULIST=arm OSLIST=nto VARIANTLIST=v7

function build_driver()
{

    export AQROOT=$1

    cd $AQROOT/build_qnx
    if [ -z "./gen_reg.sh" ]; then
        ./gen_reg.sh
    fi

    if [ -e "$AQROOT/build_qnx/screen/Makefile.dnm" ]; then
        cd $AQROOT/build_qnx/screen
        #mv Makefile.dnm Makefile
    fi
    cd $AQROOT/build_qnx

    #$cc clean
    $cc install
}


function build_test_sample()
{
    # to access driver header files under $AQROOT/sdk/inc & $AQROOT/hal/inc
    export driver_root=$1
    export test_root=$2
    export qnx_build_dir=$3

    # to access driver libraries for link
    export VIVANTE_SDK_DIR=$driver_root/build_qnx/platform_binaries
    export VIVANTE_SDK_LIB=$VIVANTE_SDK_DIR/drivers

    if [ ! -e $VIVANTE_SDK_LIB ]; then
        echo
        echo ERROR: not found $VIVANTE_SDK_LIB
        echo
    fi

    # use the following environment variables to speicify the location of test source code;
    export HAL2D_UNIT_TEST_DIR=$test_root/test/hal/common/UnitTest
    export VDK_TEST_DIR=$test_root/sdk/samples/vdk
    export ES11_TEST_DIR=$test_root/test/es11
    export ES20_TEST_DIR=$test_root/test/es20
    export VG11_TEST_DIR=$test_root/test/vg11
    export CL11_TEST_DIR=$test_root/test/cl11
    export OVX_TEST_DIR=$test_root/test/ovx

    # enter test makefile location to build test
    cd $qnx_build_dir
    $cc
    $cc install
}

#build_driver /LocalBigBox/local/fpga/ubuser/gpu_driver_src_dir
build_test_sample /LocalBigBox/local/fpga/ubuser/gpu_driver_src_dir /LocalBigBox/local/fpga/ubuser/gpu_test_src_dir `pwd`
