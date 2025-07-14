cmake_minimum_required(VERSION 3.31)
project(re3-installer)

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)
pkg_check_modules(UNSHIELD REQUIRED IMPORTED_TARGET libunshield)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS_DEBUG
    "-ggdb -O0 -Wall -Wdeprecated-declarations \
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
if(WITH_XDG_ROOT)
  add_compile_definitions(XDG_ROOT)
endif()

add_subdirectory(src)
