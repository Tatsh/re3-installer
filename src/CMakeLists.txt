function(add_min_winnt target)
  if(NOT WIN32)
    return()
  endif()
  if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    get_property(IS_64BIT GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
    # mingw can do 2000 without extra help. 64-bit requires at least XP.
    if(IS_64BIT AND NOT TARGET_VERSION MATCHES "^(0x501|0x6)")
      message(
        FATAL_ERROR
          "Invalid version for 64-bit. Must be XP (0x501) or higher. Version: ${TARGET_VERSION}")
    endif()
    target_compile_definitions(${target} PRIVATE _WIN32_WINNT=${TARGET_VERSION})
  else() # MSVC gets Vista+
    target_compile_definitions(${target} PRIVATE _WIN32_WINNT=${TARGET_VERSION})
  endif()
endfunction()

function(add_asan target)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target} PRIVATE $<$<BOOL:${ENABLE_ASAN}>:-fsanitize=address>)
    target_link_options(${target} PRIVATE $<$<BOOL:${ENABLE_ASAN}>:-fsanitize=address>)
  endif()
endfunction()

function(add_ubsan target)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target} PRIVATE $<$<BOOL:${ENABLE_UBSAN}>:-fsanitize=undefined>)
    target_link_options(${target} PRIVATE $<$<BOOL:${ENABLE_UBSAN}>:-fsanitize=undefined>)
  endif()
endfunction()

function(add_flags target)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target} PRIVATE ${GCC_CLANG_SHARED_C_FLAGS}
                                             $<$<CONFIG:Debug>:${GCC_CLANG_DEBUG_C_FLAGS}>)
    if(WIN32)
      target_link_options(${target} PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
  else()
    target_compile_options(${target} PRIVATE /Zc:forScope /Zc:inline /external:W0 /utf-8)
  endif()
endfunction()

set(LIBRE3_INSTALLER_SOURCES
    env.c
    env.h
    extractor.c
    extractor.h
    utils.c
    utils.h
    installer.c
    installer.h
    log.c
    log.h
    mkdir_p.c
    mkdir_p.h
    support.h)
add_library(re3i STATIC ${LIBRE3_INSTALLER_SOURCES})
add_min_winnt(re3i)
add_asan(re3i)
add_ubsan(re3i)
add_flags(re3i)
if(NOT MSVC)
  target_compile_options(
    re3i
    PRIVATE $<$<CONFIG:Debug>:-ggdb
            -ftrivial-auto-var-init=zero
            -O0
            -Wall
            -Wdeprecated-declarations
            -Wempty-body
            -Werror
            -Werror=float-conversion
            -Werror=implicit-function-declaration
            -Werror=incompatible-pointer-types
            -Werror=int-conversion
            -Werror=return-type
            -Wmissing-braces
            -Wmissing-field-initializers
            -Wno-missing-prototypes
            -Wno-strict-prototypes
            -Wno-trigraphs
            -Wno-unknown-pragmas
            -Wparentheses
            -Wpointer-sign
            -Wshadow
            -Wswitch
            -Wuninitialized
            -Wunknown-pragmas
            -Wunreachable-code
            -Wunused-function
            -Wunused-label
            -Wunused-parameter
            -Wunused-value
            -Wunused-variable
            -Wwrite-strings>)
endif()
target_include_directories(re3i PRIVATE ${CDIO_INCLUDE_DIRS} ${ISO9660_INCLUDE_DIRS}
                                        ${UNSHIELD_INCLUDE_DIRS})

add_executable(re3-installer main.c)
add_min_winnt(re3-installer)
add_asan(re3-installer)
add_ubsan(re3-installer)
add_flags(re3-installer)
if(NOT MSVC)
  target_compile_options(
    re3-installer
    PRIVATE $<$<CONFIG:Debug>:-ggdb
            -ftrivial-auto-var-init=zero
            -O0
            -Wall
            -Wdeprecated-declarations
            -Wempty-body
            -Werror
            -Werror=float-conversion
            -Werror=implicit-function-declaration
            -Werror=incompatible-pointer-types
            -Werror=int-conversion
            -Werror=return-type
            -Wmissing-braces
            -Wmissing-field-initializers
            -Wno-missing-prototypes
            -Wno-strict-prototypes
            -Wno-trigraphs
            -Wno-unknown-pragmas
            -Wparentheses
            -Wpointer-sign
            -Wshadow
            -Wswitch
            -Wuninitialized
            -Wunknown-pragmas
            -Wunreachable-code
            -Wunused-function
            -Wunused-label
            -Wunused-parameter
            -Wunused-value
            -Wunused-variable
            -Wwrite-strings>)
endif()
target_link_libraries(re3-installer PRIVATE re3i PkgConfig::CDIO PkgConfig::ISO9660
                                            PkgConfig::UNSHIELD)
if(WIN32)
  configure_file("version.rc.in" version.rc @ONLY)
  target_sources(re3-installer PRIVATE version.rc)
endif()
if(BUILD_STATIC AND NOT MSVC)
  target_link_libraries(re3-installer PRIVATE ZLIB::ZLIB)
  target_compile_options(re3-installer PRIVATE -static)
endif()

install(
  TARGETS re3-installer
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RUNTIME_DEPENDENCY_SET re3-installer)
install(FILES ../man/re3-installer.1 DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/man/man1)
if(NOT LINUX)
  install(FILES ../LICENSE.txt ../README.md ../CHANGELOG.md ../SECURITY.md TYPE DOC)
  install(FILES ../CITATION.cff DESTINATION ${CMAKE_INSTALL_DATADIR}/re3-installer)
  if(NOT BUILD_STATIC)
    if(MINGW)
      get_filename_component(_mingw_bin_dir ${CMAKE_C_COMPILER} DIRECTORY)
      set(_mingw_lib_dir ${_mingw_bin_dir})
    endif()
    message(STATUS "Directories: ${CMAKE_INSTALL_PREFIX}/lib ${_mingw_lib_dir}")
    install(
      RUNTIME_DEPENDENCY_SET
      re3-installer
      PRE_EXCLUDE_REGEXES
      "."
      PRE_INCLUDE_REGEXES
      "gcc|iso9660|cdio|unshield|iconv|zlib|crypto|winpthread"
      DIRECTORIES
      ${CMAKE_INSTALL_PREFIX}/bin
      ${_mingw_bin_dir})
  endif()
endif()
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VENDOR Tatsh)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
set(__cpack_system_name ${CMAKE_SYSTEM_NAME})
if(__cpack_system_name MATCHES "Windows")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(__cpack_system_name win64)
  else()
    set(__cpack_system_name win32)
  endif()
endif()
set(CPACK_PACKAGE_FILE_NAME
    ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${__cpack_system_name}-${CMAKE_BUILD_TYPE})
include(CPack)

if(WIN32 AND BUILD_INSTALLER)
  find_program(MAKENSIS_EXECUTABLE makensis)
  if(MAKENSIS_EXECUTABLE)
    configure_file(${CMAKE_SOURCE_DIR}/installer.nsi.in ${CMAKE_BINARY_DIR}/installer.nsi @ONLY)
    add_custom_target(
      nsis_installer ALL
      COMMAND ${MAKENSIS_EXECUTABLE} ${CMAKE_BINARY_DIR}/installer.nsi
      DEPENDS re3-installer
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Building NSIS installer...")
    message(STATUS "NSIS installer will be built")
  else()
    message(
      FATAL_ERROR
        "makensis not found. NSIS installer cannot be built. Install NSIS to enable installer generation."
    )
  endif()
endif()

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
