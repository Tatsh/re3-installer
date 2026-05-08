local utils = import 'utils.libjsonnet';

{
  uses_user_defaults: true,
  project_type: 'c',
  project_name: 're3-installer',
  version: '0.2.6',
  want_winget: false,
  security_policy_supported_versions: { '0.2.x': ':white_check_mark:' },
  description: 'Install GTA III files from ISO or directories for use with re3.',
  social+: {
    custom_badges: [
      '[![Tests](https://github.com/Tatsh/re3-installer/actions/workflows/tests.yml/badge.svg)](https://github.com/Tatsh/re3-installer/actions/workflows/tests.yml)',
      '[![Coverage Status](https://coveralls.io/repos/github/Tatsh/re3-installer/badge.svg?branch=master)](https://coveralls.io/github/Tatsh/re3-installer?branch=master)',
    ],
  },
  keywords: ['gta iii', 'gta vice city', 're3'],
  license: 'GPL-3',
  want_main: false,
  want_codeql: false,
  want_tests: false,
  want_snap: true,
  want_flatpak: true,
  publishing+: { flathub: 'sh.tat.re3-installer' },
  snapcraft+: {
    apps+: {
      're3-installer'+: {
        command: 'usr/bin/re3-installer',
        plugs: ['home', 'removable-media'],
      },
    },
    parts+: {
      're3-installer': {
        'build-packages': [
          'libcdio-dev',
          'libiso9660-dev',
          'libunshield-dev',
          'pkg-config',
          'zlib1g-dev',
        ],
        'cmake-parameters': [
          '-DCMAKE_BUILD_TYPE=Release',
          '-DCMAKE_INSTALL_PREFIX=/usr',
        ],
        plugin: 'cmake',
        source: 'https://github.com/Tatsh/re3-installer.git',
        'source-tag': 'v' + $.version,
        'source-type': 'git',
        'stage-packages': [
          'libcdio19t64',
          'libiso9660-11t64',
          'libunshield0',
          'zlib1g',
        ],
      },
    },
  },
  flatpak+: {
    'finish-args': [
      '--device=all',
      '--filesystem=home',
    ],
    cleanup: [
      '/include',
      '/lib/cmake',
      '/lib/pkgconfig',
      '/share/aclocal',
      '/share/doc',
      '/share/man',
      '/share/pkgconfig',
      '*.a',
      '*.la',
    ],
    modules: [
      {
        name: 'libcdio',
        buildsystem: 'autotools',
        sources: [{
          type: 'archive',
          url: 'https://github.com/libcdio/libcdio/releases/download/2.3.0/libcdio-2.3.0.tar.gz',
          sha256: '37bcb13296febbcff9dc4485834bac09212cb463c31fcea52f70ee1dd3a5a5de',
        }],
      },
      {
        name: 'libcdio-paranoia',
        buildsystem: 'autotools',
        sources: [{
          type: 'archive',
          url: 'https://github.com/libcdio/libcdio-paranoia/releases/download/release-10.2%2B2.0.2/libcdio-paranoia-10.2%2B2.0.2.tar.gz',
          sha256: '99488b8b678f497cb2e2f4a1a9ab4a6329c7e2537a366d5e4fef47df52907ff6',
        }],
      },
      {
        name: 'libunshield',
        buildsystem: 'cmake-ninja',
        sources: [{
          type: 'git',
          url: 'https://github.com/twogood/unshield.git',
          tag: '1.6.2',
        }],
      },
      {
        name: 're3-installer',
        builddir: true,
        buildsystem: 'cmake-ninja',
        'config-opts': ['-DCMAKE_BUILD_TYPE=Release'],
        sources: [{
          type: 'git',
          url: 'https://github.com/Tatsh/re3-installer.git',
          tag: 'v' + $.version,
          'x-checker-data': {
            type: 'git',
            'tag-pattern': '^v([\\d.]+)$',
          },
        }],
      },
    ],
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
      version_files+: [
        'NSIS.template.in',
        'man/re3-installer.1',
        'sh.tat.re3-installer.yml',
        'snapcraft.yaml',
      ],
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
