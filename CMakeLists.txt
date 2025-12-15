include(CheckSourceCompiles)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckTypeSize)

cmake_minimum_required(VERSION 3.31)
project(
  re3-installer
  HOMEPAGE_URL "https://tatsh.github.io/re3-installer/"
  VERSION 0.2.2
  LANGUAGES C)
set(CMAKE_C_STANDARD 23)

option(BUILD_TESTS "Build tests." OFF)
option(BUILD_STATIC "Build static executable." OFF)
option(ENABLE_ASAN "Enable address sanitiser." OFF)
option(ENABLE_COVERAGE "Link tests with gcov." OFF)
option(ENABLE_UBSAN "Enable undefined behaviour sanitiser." OFF)
if(MSVC)
  set(TARGET_VERSION
      0x600
      CACHE STRING "Target Windows version in 0xMMNN format.")
else()
  set(TARGET_VERSION 0x501)
endif()

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CDIO REQUIRED IMPORTED_TARGET libcdio)
pkg_check_modules(ISO9660 REQUIRED IMPORTED_TARGET libiso9660)
pkg_check_modules(UNSHIELD REQUIRED IMPORTED_TARGET libunshield)
if(BUILD_STATIC)
  set(ZLIB_USE_STATIC_LIBS ON)
  find_package(ZLIB REQUIRED)
endif()
if(BUILD_TESTS)
  find_package(cmocka REQUIRED)
endif()

check_type_size(_Bool STDBOOL BUILTIN_TYPES_ONLY LANGUAGE C)
check_source_compiles(
  C
  "#include <stddef.h>
int main(void) {
  nullptr_t p = nullptr;
  return 0;
}"
  C23_NULLPTR)
check_symbol_exists(fts_open "fts.h" FTS_OPEN)
check_symbol_exists(copyfile "copyfile.h" COPYFILE)
check_symbol_exists(sendfile "sys/sendfile.h" SENDFILE)
set(CMAKE_REQUIRED_INCLUDES "${CDIO_INCLUDE_DIRS} ${ISO9660_INCLUDE_DIRS}")
check_struct_has_member("iso9660_stat_t" total_size "cdio/iso9660.h" ISO9660_STAT_T_TOTAL_SIZE)

add_compile_definitions(
  $<$<BOOL:${C23_NULLPTR}>:HAVE_NULLPTR>
  $<$<BOOL:${STDBOOL}>:HAVE_STDBOOL>
  $<$<BOOL:${FTS_OPEN}>:HAVE_FTS_OPEN>
  $<$<BOOL:${COPYFILE}>:HAVE_COPYFILE>
  $<$<BOOL:${SENDFILE}>:HAVE_SENDFILE>
  $<$<BOOL:${ISO9660_STAT_T_TOTAL_SIZE}>:HAVE_ISO9660_STAT_T_TOTAL_SIZE>)
if(WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS _UNICODE UNICODE WIN32_LEAN_AND_MEAN)
endif()

add_subdirectory(src)

# CPack configuration for NSIS installer
if(WIN32)
  set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR};${CMAKE_MODULE_PATH}")
  
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_PACKAGE_NAME "re3-installer")
  set(CPACK_PACKAGE_VENDOR "re3")
  set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GTA III/VC file installer for re3")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
  
  set(CPACK_NSIS_PACKAGE_NAME "re3-installer")
  set(CPACK_NSIS_DISPLAY_NAME "re3 Installer")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/installer.ico")
  set(CPACK_NSIS_MUI_UNICO "${CMAKE_SOURCE_DIR}/installer.ico")
  
  # Custom template path
  set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
  
  # Extra install commands for DLL dependencies
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    ; Install DLL dependencies
    File /nonfatal \"${CMAKE_INSTALL_PREFIX}\\\\bin\\\\libcdio-*.dll\"
    File /nonfatal \"${CMAKE_INSTALL_PREFIX}\\\\bin\\\\libiso9660-*.dll\"
    File /nonfatal \"${CMAKE_INSTALL_PREFIX}\\\\bin\\\\libunshield-*.dll\"
    File /nonfatal \"${CMAKE_INSTALL_PREFIX}\\\\bin\\\\libiconv-*.dll\"
    File /nonfatal \"${CMAKE_INSTALL_PREFIX}\\\\bin\\\\libwinpthread-*.dll\"
    File /nonfatal \"${CMAKE_INSTALL_PREFIX}\\\\bin\\\\zlib*.dll\"
  ")
  
  include(CPack)
endif()
