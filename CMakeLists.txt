include(CheckSourceCompiles)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckTypeSize)

cmake_minimum_required(VERSION 3.31)
project(
  re3-installer
  VERSION 0.1.0
  LANGUAGES C)

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CDIO REQUIRED IMPORTED_TARGET libcdio)
pkg_check_modules(ISO9660 REQUIRED IMPORTED_TARGET libiso9660)
pkg_check_modules(UNSHIELD REQUIRED IMPORTED_TARGET libunshield)

set(CMAKE_C_STANDARD 23)

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
check_struct_has_member("struct iso9660_stat_t" total_size "cdio/cdio.h" ISO9660_STAT_T_TOTAL_SIZE)

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
