project(re3-installer)
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)
pkg_check_modules(UNSHIELD REQUIRED IMPORTED_TARGET libunshield)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG "-ggdb -O0 -Wall -Wdeprecated-declarations \
-Wempty-body -Werror -Werror=float-conversion \
-Werror=implicit-function-declaration \
-Werror=incompatible-pointer-types -Werror=int-conversion \
-Werror=return-type -Wmissing-braces -Wmissing-field-initializers \
-Wno-missing-prototypes -Wno-strict-prototypes -Wno-trigraphs \
-Wno-unknown-pragmas -Wparentheses -Wpointer-sign -Wshadow -Wswitch \
-Wuninitialized -Wunknown-pragmas -Wunreachable-code \
-Wunused-function -Wunused-label -Wunused-parameter -Wunused-value \
-Wunused-variable -Wwrite-strings")

option(WITH_XDG_ROOT "Use XDG standard directory for installation" ON)
if (WITH_XDG_ROOT)
  add_compile_definitions(XDG_ROOT)
endif()

add_executable(re3-installer env.c helpers.c main.c mkdir_p.c)
target_link_libraries(re3-installer PUBLIC PkgConfig::UNSHIELD)

install(FILES re3-installer DESTINATION "${CMAKE_INSTALL_BINDIR}")
