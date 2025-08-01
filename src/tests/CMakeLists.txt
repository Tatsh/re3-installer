set(debug_flags
    -ggdb
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
    -Wno-error=unused-function
    -Wparentheses
    -Wpointer-sign
    -Wshadow
    -Wswitch
    -Wuninitialized
    -Wno-unused-function
    -Wunknown-pragmas
    -Wunreachable-code
    -Wunused-label
    -Wunused-parameter
    -Wunused-value
    -Wunused-variable
    -Wwrite-strings)
function(create_test target mocked_functions)
  set(test_flags "")
  foreach(func ${mocked_functions})
    list(APPEND test_flags "-Wl,--wrap,${func}")
  endforeach()
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} cmocka::cmocka $<$<BOOL:${ENABLE_COVERAGE}>:gcov>)
  add_min_winnt(${target})
  add_asan(${target})
  add_ubsan(${target})
  add_flags(${target})
  target_include_directories(${target} PRIVATE ..)
  target_compile_options(${target} PRIVATE -fprofile-update=atomic -ftrivial-auto-var-init=zero
                                           ${debug_flags} $<$<BOOL:${ENABLE_COVERAGE}>:--coverage>)
  target_link_options(${target} PRIVATE ${test_flags})
  add_test(NAME ${target} COMMAND ${target})
endfunction()

set(installer_tests_mocked_functions
    copy_tree
    ends_with_iso
    exists
    extract_iso_to_temp
    is_dir_empty
    remove_tree
    unshield_extract)
create_test(
  installer_tests
  "${installer_tests_mocked_functions}"
  ../installer.c
  ../installer.h
  ../log.c
  ../log.h
  installer_tests.c)

create_test(env_tests "" ../env.c ../env.h env_tests.c)

set(extractor_tests_mocked_functions
    _cdio_list_begin
    _cdio_list_node_data
    _cdio_list_node_next
    ends_with_dll
    ends_with_exe
    ends_with_url
    env
    fclose
    fopen
    ferror
    fwrite
    iso9660_close
    iso9660_ifs_get_joliet_level
    iso9660_ifs_readdir
    iso9660_iso_seek_read
    iso9660_name_translate_ext
    iso9660_open_ext
    mkdtemp
    mkdir_p
    unshield_close
    unshield_directory_name
    unshield_file_directory
    unshield_file_group_find
    unshield_file_is_valid
    unshield_file_name
    unshield_file_save
    unshield_open
    unshield_set_log_level)
create_test(
  extractor_tests
  "${extractor_tests_mocked_functions}"
  ../extractor.c
  ../extractor.h
  ../log.c
  ../log.h
  extractor_tests.c)
target_link_libraries(extractor_tests PkgConfig::ISO9660)

set(mkdir_p_tests_mocked_functions mkdir)
create_test(mkdir_p_tests "${mkdir_p_tests_mocked_functions}" ../mkdir_p.c ../mkdir_p.h
            mkdir_p_tests.c)

create_test(
  helpers_tests
  ""
  ../env.c
  ../env.h
  ../helpers.c
  ../helpers.h
  ../log.c
  ../log.h
  ../mkdir_p.c
  ../mkdir_p.h
  helpers_tests.c)

set(helpers_mock_tests_mocked_functions
    close
    env
    fts_open
    fts_read
    fts_close
    mkdir_p
    open
    rmdir
    sendfile
    stat
    unlink)
create_test(
  helpers_mock_tests
  "${helpers_mock_tests_mocked_functions}"
  ../helpers.c
  ../helpers.h
  ../log.c
  ../log.h
  helpers_mock_tests.c)
