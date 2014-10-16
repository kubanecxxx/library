
#usage of software based timer scheduler module
ifeq ($(USE_SCHEDULER),yes)
	include $(LIBRARIES)/scheduler/rules.mk
endif

#use automatic packet processing module
#based on basic and advanced RF24 libraries
ifeq ($(USE_RF24_CALLBACKS),yes)
	include $(LIBRARIES)/rf24_callbacks/rules.mk
	USE_RF24_ADVANCED = yes
endif

#use advanced RF24 library
#based on basic RF24 library
ifeq ($(USE_RF24_ADVANCED),yes)
	include $(LIBRARIES)/rf24_app/rules.mk
	USE_RF24 = yes
endif

#use basic RF24 library - module api
ifeq ($(USE_RF24),yes)
	include $(LIBRARIES)/rf24/rules.mk
endif

#use LM85 I2C temperature module with powersaving
ifeq ($(USE_TEMPERATURE_I2C),yes)
	include $(LIBRARIES)/temperature/rules.mk
endif

#use ST775 small LCD display with SPI interface
ifeq ($(USE_ST775),yes)
	include $(LIBRARIES)/st7735/st7735.mk  
endif

#use font table used by GUI
ifeq ($(USE_FONTS),yes)
	include $(LIBRARIES)/fonts/rules.mk
endif