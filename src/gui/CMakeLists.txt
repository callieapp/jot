qt_add_executable(jot-gui
    main.cpp
)

target_compile_definitions(jot-gui PRIVATE
    JOT_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(jot-gui PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    jotui
    jotuiplugin
    jotcore
)

install(TARGETS jot-gui RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
