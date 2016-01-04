
set (ESP_SOURCES
esp8266.c
${CHIBIOS_DIR}/os/hal/lib/streams/chprintf.c
)

set (ESP_HEADERS
esp8266.h
)

add_library(esp OBJECT ${ESP_SOURCES} ${ESP_HEADERS})
include_directories(esp, ${CHIBIOS_DIR}/os/hal/lib/streams)


target_include_directories(${USER_CODE} PUBLIC .)

