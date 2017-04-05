#!/bin/bash

if [ -z "$AQROOT" ]; then
   AQROOT=$PWD/../../..
fi
GC_CL_PATH=$AQROOT/compiler/libCLC/compiler

#
# generate gc_cl_scanner.c, gc_cl_parser.c, and gc_cl_token_def.h
#
#cd $GC_CL_PATH; flex  -L gc_cl.l
cd $GC_CL_PATH; flex  gc_cl.l

GC_CL_SCANNER_C=$GC_CL_PATH/gc_cl_scanner.c
GC_CL_PARSER_C=$GC_CL_PATH/gc_cl_parser.c

#
# clean up the stuff about libc in gc_cl_scanner.c
#
if [ -e "$GC_CL_SCANNER_C" ]; then
	sed -i "s/.*stdio.h.*/#include \"gc_cl_scanner.h\"/" $GC_CL_SCANNER_C
	sed -i "s/.*string.h.*//" $GC_CL_SCANNER_C
	sed -i "s/.*errno.h.*//" $GC_CL_SCANNER_C
	sed -i "s/.*unistd.h.*//" $GC_CL_SCANNER_C
	sed -i "s/.*stdlib.h.*//" $GC_CL_SCANNER_C
	sed -i "s/.*<id.h>.*//" $GC_CL_SCANNER_C
	sed -i "s/.*fprint.*stderr.*msg.*//" $GC_CL_SCANNER_C
	sed -i "s/exit.*YY_EXIT_FAILURE.*/clReport(0, 0, clvREPORT_FATAL_ERROR, (char \*)msg);/" $GC_CL_SCANNER_C
	sed -i "s/\([ \t\r]\)malloc\([ \t]*(\)/\1clMalloc\2/" $GC_CL_SCANNER_C
	sed -i "s/\([ \t\r]\)free\([ \t]*(\)/\1clFree\2/" $GC_CL_SCANNER_C
	sed -i "s/\([ \t\r]\)realloc\([ \t]*(\)/\1clRealloc\2/" $GC_CL_SCANNER_C
	#sed -i "s/return.*[\r\n]*[ \t]*YY_BREAK/return&/" $GC_CL_SCANNER_C
	sed -i "s/return\(.*[\r\n]*[ \t]*\)YY_BREAK/return\1/" $GC_CL_SCANNER_C
	sed -i "s/typedef unsigned char YY_CHAR/typedef int YY_CHAR/" $GC_CL_SCANNER_C
else
	echo
	echo ERROR: not found $GC_CL_SCANNER_C
	echo
fi

cd $GC_CL_PATH; bison -v -d gc_cl.y
cd $GC_CL_PATH; echo "#ifndef __gc_cl_token_def_h_" > gc_cl_token_def.h
cd $GC_CL_PATH; echo "#define __gc_cl_token_def_h_" >> gc_cl_token_def.h
cd $GC_CL_PATH; cat gc_cl.tab.h >> gc_cl_token_def.h; rm -f gc_cl.tab.h
cd $GC_CL_PATH; echo "#endif /* __gc_cl_token_def_h_ */" >> gc_cl_token_def.h
cd $GC_CL_PATH; mv gc_cl.tab.c gc_cl_parser.c
cd $GC_CL_PATH; sed -i "s/gc_cl.tab.c/gc_cl_parser\.c/" gc_cl_parser.c

#
# clean up the stuff about libc in gc_cl_parser.c
#
if [ -e "$GC_CL_PARSER_C" ]; then
    cp $GC_CL_PARSER_C ${GC_CL_PARSER_C}.orig
	sed -i "s/#include.*gc_cl_parser.h.*/#include \"gc_cl_parser.h\"/" $GC_CL_PARSER_C
	sed -i "s/.*stdlib.h.*//" $GC_CL_PARSER_C
	sed -i "s/.*stdio.h.*//" $GC_CL_PARSER_C
	sed -i "s/\([ \t\r]\)malloc$/\1clMalloc/" $GC_CL_PARSER_C
	sed -i "s/\([ \t\r]\)malloc\([ \t\r]*(\)/\1clMalloc\2/" $GC_CL_PARSER_C
	sed -i "s/\([ \t\r]\)free$/\1clFree/" $GC_CL_PARSER_C
	sed -i "s/\([ \t\r]\)\(free(\[ \t\r]*(\)/\1clFree\2/" $GC_CL_PARSER_C
	sed -i "s/\([ \t\r]\)realloc$/\1clRealloc/" $GC_CL_PARSER_C
	sed -i "s/\([ \t\r]\)\(realloc[ \t\r]*(\)/\1clRealloc\2/" $GC_CL_PARSER_C
	sed -i "s/\([ \t\r]\)alloc$/\1clAlloc/" $GC_CL_PARSER_C
	sed -i "s/\([ \t\r]\)\(alloc[ \t\r]*(\)/\1clAlloc\2/" $GC_CL_PARSER_C
else
	echo
	echo ERROR: not found $GC_CL_PARSER_C
	echo
fi


