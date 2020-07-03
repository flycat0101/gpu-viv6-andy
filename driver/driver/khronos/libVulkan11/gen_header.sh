#! /bin/sh

cd ../../..
AQROOT=`pwd`
cd ${AQROOT}/driver/khronos/libVulkan11

python ${AQROOT}/tools/bin/ConvertFragToSpvToC.py -t ${AQROOT}/tools/bin/glslangValidator/Linux/glslangValidator -c src/chip -f src/chip/gc_halti5_patchlib.frag
python ${AQROOT}/tools/bin/ConvertFragToSpvToC.py -t ${AQROOT}/tools/bin/glslangValidator/Linux/glslangValidator -c src/chip -f src/chip/gc_halti5_blit.comp

