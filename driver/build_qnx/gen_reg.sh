export AQROOT=`pwd`/..
export AQARCH=$AQROOT/arch/XAQ2
export AQVGARCH=$AQROOT/arch/GC350

if [ -e ${AQARCH}/reg ]; then
    cd ${AQARCH}/reg
    make -f makefile.linux
fi

if [ -e ${AQVGARCH}/reg ]; then
    cd ${AQVGARCH}/reg
    make -f makefile.linux
fi
