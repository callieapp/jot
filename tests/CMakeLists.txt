find_package(Qt6 REQUIRED COMPONENTS Test)

# TZ is pinned so that wall-clock assertions do not depend on the host zone.
function(jot_add_test name)
    qt_add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE Qt6::Test jotcore)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "TZ=UTC;QT_QPA_PLATFORM=offscreen")
endfunction()

jot_add_test(tst_frontmatter)
jot_add_test(tst_identity)
target_compile_definitions(tst_identity PRIVATE JOT_EXPECT_DEVEL=$<BOOL:${JOT_DEVEL}>)
jot_add_test(tst_logfile)
jot_add_test(tst_color)
jot_add_test(tst_theme)
jot_add_test(tst_themelibrary)
jot_add_test(tst_themecontroller)
target_link_libraries(tst_themecontroller PRIVATE jotui)
jot_add_test(tst_windowbuttons)
target_link_libraries(tst_windowbuttons PRIVATE jotui)
jot_add_test(tst_dropdown)
target_link_libraries(tst_dropdown PRIVATE jotui jotuiplugin Qt6::QuickControls2)
jot_add_test(tst_liveqml)
target_link_libraries(tst_liveqml PRIVATE jotui)
jot_add_test(tst_paint)
target_link_libraries(tst_paint PRIVATE jotcli)
jot_add_test(tst_completion)
target_link_libraries(tst_completion PRIVATE jotcli)
jot_add_test(tst_upgrade)
target_link_libraries(tst_upgrade PRIVATE jotcli Qt6::Network)
jot_add_test(tst_updatecheck)
target_link_libraries(tst_updatecheck PRIVATE jotcli Qt6::Network)
jot_add_test(tst_diagnostics)

# The bash completion script, against the built jot.
find_program(BASH bash)
if(BASH)
    add_test(NAME completion_script
        COMMAND ${BASH} ${CMAKE_CURRENT_SOURCE_DIR}/completion_test.sh
            ${PROJECT_SOURCE_DIR}/data/completions/jot.bash $<TARGET_FILE:jot>)
    set_tests_properties(completion_script PROPERTIES ENVIRONMENT "TZ=UTC;NO_COLOR=1")
endif()

# The installer each release publishes, against stand-in systems and a stand-in
# GitHub release. Release tarballs leave it out, and it only runs on x86_64.
if(EXISTS ${PROJECT_SOURCE_DIR}/scripts/install.sh AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    add_test(NAME install_script
        COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/install_test.sh
            ${PROJECT_SOURCE_DIR}/scripts/install.sh ${CMAKE_CURRENT_SOURCE_DIR}/data/github-release.json)
endif()
