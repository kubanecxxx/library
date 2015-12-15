
set (SH_SOURCES
scheduler/scheduler.c

)

set (SH_HEADERS
scheduler/scheduler.h
scheduler/scheduler.hpp
)

add_library(library OBJECT ${SH_SOURCES} ${SH_HEADERS})
target_include_directories(user_code PUBLIC scheduler)
