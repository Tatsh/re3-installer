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

set(installer_tests_mocked_functions
    is_dir_empty
    is_iso
    extract_iso_to_temp
    unshield_extract
    copy_tree
    remove_tree
    exists)
foreach(func ${installer_tests_mocked_functions})
  list(APPEND test_flags "-Wl,--wrap,${func}")
endforeach()
add_executable(installer_tests ../installer.c ../installer.h ../log.c ../log.h installer_tests.c)
target_link_libraries(installer_tests cmocka)
add_min_winnt(installer_tests)
add_asan(installer_tests)
add_ubsan(installer_tests)
add_flags(installer_tests)
target_include_directories(installer_tests PRIVATE ..)
target_compile_options(installer_tests PRIVATE -ftrivial-auto-var-init=zero)
if(ENABLE_COVERAGE)
  target_compile_options(installer_tests PRIVATE $<$<BOOL:${ENABLE_COVERAGE}>:--coverage>)
  target_link_libraries(installer_tests $<$<BOOL:${ENABLE_COVERAGE}>:gcov>)
endif()
target_link_options(installer_tests PRIVATE ${test_flags})
add_test(NAME installer_tests COMMAND installer_tests)

add_executable(env_tests ../env.c ../env.h env_tests.c)
target_link_libraries(env_tests cmocka)
add_min_winnt(env_tests)
add_asan(env_tests)
add_ubsan(env_tests)
add_flags(env_tests)
target_include_directories(env_tests PRIVATE ..)
target_compile_options(env_tests PRIVATE -ftrivial-auto-var-init=zero)
if(ENABLE_COVERAGE)
  target_compile_options(env_tests PRIVATE $<$<BOOL:${ENABLE_COVERAGE}>:--coverage>)
  target_link_libraries(env_tests $<$<BOOL:${ENABLE_COVERAGE}>:gcov>)
endif()
target_link_options(env_tests PRIVATE ${test_flags})
add_test(NAME env_tests COMMAND env_tests)

set(extractor_tests_mocked_functions
    _cdio_list_begin
    _cdio_list_node_data
    _cdio_list_node_next
    ends_with_dll
    ends_with_exe
    ends_with_url
    env
    iso9660_close
    iso9660_filelist_free
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
foreach(func ${extractor_tests_mocked_functions})
  list(APPEND extractor_tests_flags "-Wl,--wrap,${func}")
endforeach()
add_executable(extractor_tests ../extractor.c ../extractor.h ../log.c ../log.h extractor_tests.c)
target_link_libraries(extractor_tests cmocka)
add_min_winnt(extractor_tests)
add_asan(extractor_tests)
add_ubsan(extractor_tests)
add_flags(extractor_tests)
target_include_directories(extractor_tests PRIVATE ..)
target_compile_options(extractor_tests PRIVATE ${debug_flags})
if(ENABLE_COVERAGE)
  target_compile_options(extractor_tests PRIVATE $<$<BOOL:${ENABLE_COVERAGE}>:--coverage>)
  target_link_libraries(extractor_tests $<$<BOOL:${ENABLE_COVERAGE}>:gcov>)
endif()
target_link_options(extractor_tests PRIVATE ${extractor_tests_flags})
add_test(NAME extractor_tests COMMAND extractor_tests)
