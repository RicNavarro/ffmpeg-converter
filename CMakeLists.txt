cmake_minimum_required(VERSION 3.20)
project(FFmpegWrapper VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# -----------------------------------------------------------------------
# Qt6
# -----------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# -----------------------------------------------------------------------
# Source files (shared across all platforms)
# -----------------------------------------------------------------------
set(SOURCES
    src/main.cpp

    src/core/FormatProfile.cpp
    src/core/FFmpegPresets.cpp
    src/core/ConversionWorker.cpp
    src/core/ConversionController.cpp
    src/core/AppSettings.cpp

    src/ui/MainWindow.cpp
    src/ui/DropZone.cpp
    src/ui/SettingsPanel.cpp
    src/ui/ConversionLog.cpp

    src/utils/FileUtils.cpp
)

set(HEADERS
    src/core/FormatProfile.h
    src/core/FFmpegPresets.h
    src/core/ConversionWorker.h
    src/core/ConversionController.h
    src/core/AppSettings.h

    src/ui/MainWindow.h
    src/ui/DropZone.h
    src/ui/SettingsPanel.h
    src/ui/ConversionLog.h

    src/utils/FileUtils.h
)

# Qt resources (.qrc) — assets compiled into the binary
set(RESOURCES resources/resources.qrc)

# -----------------------------------------------------------------------
# Platform-specific setup
# -----------------------------------------------------------------------
if(WIN32)
    # Suppress the console window on Windows Release builds
    set(GUI_TYPE WIN32)
    # .rc file embeds icon + version metadata into the .exe
    set(WIN_RC resources/app.rc)
else()
    set(GUI_TYPE "")
    set(WIN_RC   "")
endif()

# -----------------------------------------------------------------------
# Executable
# -----------------------------------------------------------------------
add_executable(FFmpegWrapper ${GUI_TYPE}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${WIN_RC}
)

target_link_libraries(FFmpegWrapper PRIVATE Qt6::Widgets)
target_include_directories(FFmpegWrapper PRIVATE src)

# -----------------------------------------------------------------------
# Windows post-build: copy bundled ffmpeg.exe + run windeployqt
# -----------------------------------------------------------------------
if(WIN32)
    add_custom_command(TARGET FFmpegWrapper POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:FFmpegWrapper>/ffmpeg"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/ffmpeg/ffmpeg.exe"
            "$<TARGET_FILE_DIR:FFmpegWrapper>/ffmpeg/ffmpeg.exe"
        COMMENT "Copying bundled ffmpeg.exe"
    )

    find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT)
        add_custom_command(TARGET FFmpegWrapper POST_BUILD
            COMMAND "${WINDEPLOYQT}"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                "$<TARGET_FILE:FFmpegWrapper>"
            COMMENT "Running windeployqt"
        )
    endif()
endif()

# -----------------------------------------------------------------------
# Linux install rules (used by both direct install and .deb packaging)
#
# Standard FHS layout:
#   /usr/bin/FFmpegWrapper
#   /usr/share/applications/ffmpeg-converter.desktop
#   /usr/share/icons/hicolor/256x256/apps/ffmpeg-converter.png
#   /usr/share/doc/ffmpeg-converter/changelog.gz
# -----------------------------------------------------------------------
if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)

    install(TARGETS FFmpegWrapper
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(FILES
        linux/ffmpeg-converter.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )

    install(FILES
        linux/icons/ffmpeg-converter-256.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
        RENAME ffmpeg-converter.png
    )

    install(FILES
        linux/icons/ffmpeg-converter-128.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps
        RENAME ffmpeg-converter.png
    )

    # -----------------------------------------------------------------------
    # CPack — generates a .deb package via `cmake --build . --target package`
    # -----------------------------------------------------------------------
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME            "ffmpeg-converter")
    set(CPACK_PACKAGE_VERSION         "1.0.0")
    set(CPACK_PACKAGE_CONTACT         "you@example.com")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "A graphical FFmpeg wrapper for media file conversion")
    set(CPACK_DEBIAN_PACKAGE_SECTION  "multimedia")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

    # Runtime dependencies — ffmpeg + Qt6 system libs
    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "ffmpeg (>= 4.0), libqt6widgets6 (>= 6.2), libqt6gui6 (>= 6.2), libqt6core6 (>= 6.2)"
    )

    # Post-install script: update icon cache and .desktop database
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
        "${CMAKE_SOURCE_DIR}/linux/deb/postinst"
        "${CMAKE_SOURCE_DIR}/linux/deb/postrm"
    )

    include(CPack)
endif()
