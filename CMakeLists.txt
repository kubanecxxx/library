set (LIBS)

if (NOT DEFINED LIBRARY_MODULES)

    set(LIBRARY_MODULES
        scheduler crc esp
        )

    message("library - select all modules by default (${LIBRARY_MODULES}). You can specify modules by variable LIBRARY_MODULES")
endif()

foreach(VAR ${LIBRARY_MODULES})
    add_subdirectory(${VAR})
    list (APPEND LIBS $<TARGET_OBJECTS:${VAR}>)
endforeach()

add_library(library STATIC ${LIBS})




