# The QML files sit at this module's root, which is what Qt expects. Keeping
# them in a subdirectory would require a hand-written qmldir (policy QTP0004).
qt_add_qml_module(jotui
    URI Jot.Ui
    VERSION 1.0
    STATIC
    # qmllint and qmlls expect the output path to mirror the module URI.
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Jot/Ui
    # Theme exposes QColor properties; older qmllint only maps them to QML's
    # color type when QtQuick is a declared dependency.
    DEPENDENCIES QtQuick
    SOURCES
        ThemeController.h
        ThemeController.cpp
    QML_FILES
        Main.qml
)

target_include_directories(jotui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(jotui PUBLIC Qt6::Quick Qt6::QuickControls2 jotcore)
