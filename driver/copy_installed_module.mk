TARGET_VIV_LIBRARY := $(subst $(PRODUCT_OUT),$(VIV_OUT),$(LOCAL_INSTALLED_MODULE))

ifeq ($(filter 6 7 8,$(PLATFORM_SDK_VERSION)),)
ALL_MODULES += $(TARGET_VIV_LIBRARY)
endif

$(TARGET_VIV_LIBRARY) : $(LOCAL_INSTALLED_MODULE) $(ACP)
	@echo "Copy: $< -> $@"
	@mkdir -p $(dir $@)
	@$(ACP) $< $@

