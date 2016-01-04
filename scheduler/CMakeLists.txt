
set (SH_SOURCES
scheduler.c

)

set (SH_HEADERS
scheduler.h
scheduler.hpp
)

add_library(scheduler OBJECT ${SH_SOURCES} ${SH_HEADERS})
target_include_directories(${USER_CODE} PUBLIC .)

