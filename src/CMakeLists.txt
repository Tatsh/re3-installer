add_executable(re3-installer env.c helpers.c main.c mkdir_p.c)
target_link_libraries(re3-installer PUBLIC PkgConfig::UNSHIELD)

install(FILES re3-installer DESTINATION "${CMAKE_INSTALL_BINDIR}")
