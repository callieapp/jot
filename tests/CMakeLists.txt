find_package(Qt6 REQUIRED COMPONENTS Test)

# TZ is pinned so that wall-clock assertions do not depend on the host zone.
function(jot_add_test name)
    qt_add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE Qt6::Test jotcore)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "TZ=UTC;QT_QPA_PLATFORM=offscreen")
endfunction()

jot_add_test(tst_frontmatter)
