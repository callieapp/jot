# A development tool for designing themes: it renders every token and component
# from the active theme and reloads when the theme file changes. Not installed.
qt_add_executable(jot-gallery
    main.cpp
)

qt_add_qml_module(jot-gallery
    URI Jot.Gallery
    VERSION 1.0
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Jot/Gallery
    # Lets qmllint find Jot.Ui, which lives elsewhere in the build tree.
    IMPORT_PATH ${CMAKE_BINARY_DIR}
    DEPENDENCIES Jot.Ui
    QML_FILES
        Gallery.qml
)

target_link_libraries(jot-gallery PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    jotui
    jotuiplugin
)

if(JOT_LIVE_QML)
    target_compile_definitions(jot-gallery PRIVATE
        JOT_LIVE_QML
        JOT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        JOT_BINARY_DIR="${CMAKE_BINARY_DIR}"
    )
endif()
