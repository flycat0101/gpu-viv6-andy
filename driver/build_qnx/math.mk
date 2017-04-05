ifeq ($(CPU), aarch64)
  LIBS += m
else
  ifneq ($(filter v7, $(VARIANT_LIST)), v7)
    CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
    LIBS += m-vfp
  else
    LIBS += m
  endif
endif
