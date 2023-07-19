if(${IDF_TARGET} STREQUAL esp8266)
    set(COMPONENT_REQUIRES esp8266 esp_timer)
elseif(${IDF_VERSION_MAJOR} STREQUAL 4 AND ${IDF_VERSION_MINOR} STREQUAL 1 AND ${IDF_VERSION_PATCH} STREQUAL 3)
    set(COMPONENT_REQUIRES driver)
else()    
    set(COMPONENT_REQUIRES driver esp_timer)
endif()

set(COMPONENT_ADD_INCLUDEDIRS
    src
    )
file(GLOB SRCS
     src/*.c
     )

set(COMPONENT_SRCS ${SRCS})

message(STATUS "button use components = ${COMPONENT_REQUIRES}")

register_component()
