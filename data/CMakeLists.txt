install(FILES ${JOT_APP_ID}.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES ${JOT_APP_ID}.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)

# The logo doubles as the application icon. The small sizes are drawn apart from
# it, with fewer and bolder lines, so they stay legible in a panel.
install(FILES ${CMAKE_SOURCE_DIR}/assets/logo.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
    RENAME ${JOT_APP_ID}.svg
)
foreach(size 16 32)
    install(FILES ${CMAKE_SOURCE_DIR}/assets/logo-${size}.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${size}x${size}/apps
        RENAME ${JOT_APP_ID}.png
    )
endforeach()
