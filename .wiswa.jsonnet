local utils = import 'utils.libjsonnet';

{
  project_type: 'c',
  project_name: 're3-installer',
  version: '0.2.2',
  security_policy_supported_versions: { '0.2.x': ':white_check_mark:' },
  description: 'Install GTA III files from ISO or directories for use with re3.',
  keywords: ['gta iii', 'gta vice city', 're3'],
  want_main: false,
  want_codeql: false,
  want_tests: false,
  copilot+: {
    intro: 're3-installer extracts GTA III game content from ISOs or directories for use with re3.',
  },
  // C/C++ only
  vcpkg+: {
    dependencies: ['libarchive', {
      name: 'cmocka',
      platform: 'linux|mingw',
    }],
  },
  cz+: {
    commitizen+: {
      version_files+: ['man/re3-installer.1'],
    },
  },
  package_json+: {
    scripts+: {
      'check-formatting': "cmake-format --check CMakeLists.txt src/CMakeLists.txt src/tests/CMakeLists.txt; clang-format -n src/*.c src/*.h src/tests/*.c && prettier -c . && markdownlint-cli2 '**/*.md' '#node_modules'",
      format: 'cmake-format -i CMakeLists.txt src/CMakeLists.txt src/tests/CMakeLists.txt && clang-format -i src/*.c src/*.h src/tests/*.c && yarn prettier -w .',
    },
  },
  vscode+: {
    c_cpp+: {
      configurations: [
        {
          cStandard: 'gnu23',
          compilerPath: '/usr/bin/gcc',
          cppStandard: 'gnu++23',
          includePath: ['${workspaceFolder}/src/**'],
          name: 'Linux',
        },
      ],
    },
  },
}
