
set (SSD_SOURCES
ssd1289_lld.c

)

set (SSD_HEADERS
ssd1289_lld.h
)


add_library(ssd1289 OBJECT ${SSD_SOURCES} ${SSD_HEADERS})
target_include_directories(${USER_CODE} PUBLIC .)

