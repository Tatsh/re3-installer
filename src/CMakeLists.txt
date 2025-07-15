set(LIBRE3_INSTALLER_SOURCES
    env.c
    env.h
    extractor.c
    extractor.h
    helpers.c
    helpers.h
    mkdir_p.c
    mkdir_p.h
    support.h)

add_library(re3i STATIC ${LIBRE3_INSTALLER_SOURCES})
target_compile_definitions(re3i PRIVATE _POSIX_C_SOURCE _DEFAULT_SOURCE _XOPEN_SOURCE=500)

add_executable(re3-installer main.c)
target_link_libraries(re3-installer PRIVATE re3i LibArchive::LibArchive PkgConfig::UNSHIELD)

install(FILES re3-installer DESTINATION ${CMAKE_INSTALL_BINDIR})
