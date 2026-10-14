# Deliberately not a QML module. The CLI links this, and pulling QtQml in would
# cost it startup time for no benefit. QML exposure happens in src/ui.
add_library(jotcore STATIC
    jot/FrontMatter.h
    FrontMatter.cpp
)

target_include_directories(jotcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(jotcore PUBLIC Qt6::Core)
