include $(LIBRARIES)/scheduler/rules.mk
include $(LIBRARIES)/rf24/rules.mk
include $(LIBRARIES)/rf24_app/rules.mk
include $(LIBRARIES)/rf24_callbacks/rules.mk
include $(LIBRARIES)/temperature/rules.mk

ifeq ($(USE_ST775),yes)
	include $(LIBRARIES)/st7735/st7735.mk  
endif

include $(LIBRARIES)/fonts/rules.mk