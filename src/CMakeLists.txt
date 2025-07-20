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
target_compile_options(
  re3i
  PRIVATE $<$<CONFIG:Debug>:-ggdb
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
target_include_directories(re3i PRIVATE ${CDIO_INCLUDE_DIRS} ${ISO9660_INCLUDE_DIRS}
                                        ${UNSHIELD_INCLUDE_DIRS})

add_executable(re3-installer main.c)
add_min_winnt(re3-installer)
add_asan(re3-installer)
add_ubsan(re3-installer)
add_flags(re3-installer)
target_compile_options(
  re3-installer
  PRIVATE $<$<CONFIG:Debug>:-ggdb
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
target_link_libraries(re3-installer PRIVATE re3i PkgConfig::CDIO PkgConfig::ISO9660
                                            PkgConfig::UNSHIELD)

install(TARGETS re3-installer)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
