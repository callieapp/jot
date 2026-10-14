qt_add_executable(jot
    main.cpp
)

target_compile_definitions(jot PRIVATE JOT_VERSION="${PROJECT_VERSION}")
target_link_libraries(jot PRIVATE jotcore)

install(TARGETS jot RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
