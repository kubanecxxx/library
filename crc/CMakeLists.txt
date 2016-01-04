
set (CRC_SOURCES

crc16.c
)

set (CRC_HEADERS
crc16.h
)

add_library(crc OBJECT ${CRC_SOURCES} ${CRC_HEADERS})
target_include_directories(${USER_CODE} PUBLIC .)

