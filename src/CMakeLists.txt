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
  else()
    target_compile_options(${target} PRIVATE /Zc:forScope /Zc:inline /external:W0 /utf-8)
  endif()
endfunction()

set(LIBRE3_INSTALLER_SOURCES
    env.c
    env.h
    extractor.c
    extractor.h
    helpers.c
    helpers.h
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

install(TARGETS re3-installer RUNTIME)
install(FILES ../man/re3-installer.1 TYPE MAN)
# FIXME include license file in zip

# FIXME install path should not have version number
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

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
