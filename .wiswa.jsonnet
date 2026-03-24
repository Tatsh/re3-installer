local utils = import 'utils.libjsonnet';

{
  project_type: 'c',
  project_name: 're3-installer',
  version: '0.2.3',
  want_winget: false,
  security_policy_supported_versions: { '0.2.x': ':white_check_mark:' },
  description: 'Install GTA III files from ISO or directories for use with re3.',
  keywords: ['gta iii', 'gta vice city', 're3'],
  license: 'GPL-3',
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
      version_files+: ['NSIS.template.in', 'man/re3-installer.1'],
    },
  },
  clang_format_args: 'src/*.c src/*.h src/tests/*.c',
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
