set(LIBRE3_INSTALLER_SOURCES
    env.c
    env.h
    extractor.c
    extractor.h
    helpers.c
    helpers.h
    installer.c
    installer.h
    mkdir_p.c
    mkdir_p.h
    support.h)
add_library(re3i STATIC ${LIBRE3_INSTALLER_SOURCES})
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
