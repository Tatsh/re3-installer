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
