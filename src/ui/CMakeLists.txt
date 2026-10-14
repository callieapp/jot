# The QML files sit at this module's root, which is what Qt expects. Keeping
# them in a subdirectory would require a hand-written qmldir (policy QTP0004).
qt_add_qml_module(jotui
    URI Jot.Ui
    VERSION 1.0
    STATIC
    # qmllint and qmlls expect the output path to mirror the module URI.
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Jot/Ui
    DEPENDENCIES QtQuick
    QML_FILES
        Main.qml
)

target_link_libraries(jotui PUBLIC Qt6::Quick Qt6::QuickControls2 jotcore)
