local utils = import 'utils.libjsonnet';

(import 'defaults.libjsonnet') + {
  local top = self,
  // General settings
  project_type: 'c',

  // Shared
  github_username: 'Tatsh',
  security_policy_supported_versions: { '0.2.x': ':white_check_mark:' },
  authors: [
    {
      'family-names': 'Udvare',
      'given-names': 'Andrew',
      email: 'audvare@gmail.com',
      name: '%s %s' % [self['given-names'], self['family-names']],
    },
  ],
  project_name: 're3-installer',
  version: '0.2.2',
  description: 'Install GTA III files from ISO or directories for use with re3.',
  keywords: ['gta iii', 're3'],
  want_main: false,
  copilot: {
    intro: "re3-installer extracts GTA III game content from ISOs or directories for use with re3.",
  },
  social+: {
    mastodon+: { id: '109370961877277568' },
  },

  // GitHub
  github+: {
    funding+: {
      ko_fi: 'tatsh2',
      liberapay: 'tatsh2',
      patreon: 'tatsh2',
    },
  },

  // C++ only
  vcpkg+: {
    dependencies: ['libarchive', {
      name: 'cmocka',
      platform: 'linux|mingw',
    }],
  },
}
