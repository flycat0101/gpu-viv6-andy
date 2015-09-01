	IF :DEF: DSB_AVAILABLE
	CODE32

	EXPORT	|DSB|

	AREA	 |.text|, CODE, ARM
|DSB| PROC
    dsb
    bx          lr

	ENDP
	ENDIF
    END
