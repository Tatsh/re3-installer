# re3-installer

[![GitHub tag (with filter)](https://img.shields.io/github/v/tag/Tatsh/re3-installer)](https://github.com/Tatsh/re3-installer/tags)
[![License](https://img.shields.io/github/license/Tatsh/re3-installer)](https://github.com/Tatsh/re3-installer/blob/master/LICENSE.txt)
[![GitHub commits since latest release (by SemVer including pre-releases)](https://img.shields.io/github/commits-since/Tatsh/re3-installer/v0.1.0/master)](https://github.com/Tatsh/re3-installer/compare/v0.1.0...master)
[![CodeQL](https://github.com/Tatsh/re3-installer/actions/workflows/codeql.yml/badge.svg)](https://github.com/Tatsh/re3-installer/actions/workflows/codeql.yml)
[![QA](https://github.com/Tatsh/re3-installer/actions/workflows/qa.yml/badge.svg)](https://github.com/Tatsh/re3-installer/actions/workflows/qa.yml)
[![Tests](https://github.com/Tatsh/re3-installer/actions/workflows/tests.yml/badge.svg)](https://github.com/Tatsh/re3-installer/actions/workflows/tests.yml)
[![Coverage Status](https://coveralls.io/repos/github/Tatsh/re3-installer/badge.svg?branch=master)](https://coveralls.io/github/Tatsh/re3-installer?branch=master)
[![GitHub Pages](https://github.com/Tatsh/re3-installer/actions/workflows/pages/pages-build-deployment/badge.svg)](https://tatsh.github.io/re3-installer/)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)](https://github.com/pre-commit/pre-commit)
[![Stargazers](https://img.shields.io/github/stars/Tatsh/re3-installer?logo=github&style=flat)](https://github.com/Tatsh/re3-installer/stargazers)

[![@Tatsh](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor%3Ddid%3Aplc%3Auq42idtvuccnmtl57nsucz72%26query%3D%24.followersCount%26style%3Dsocial%26logo%3Dbluesky%26label%3DFollow%2520%40Tatsh&query=%24.followersCount&style=social&logo=bluesky&label=Follow%20%40Tatsh)](https://bsky.app/profile/Tatsh.bsky.social)
[![Mastodon Follow](https://img.shields.io/mastodon/follow/109370961877277568?domain=hostux.social&style=social)](https://hostux.social/@Tatsh)

Install GTA III files from ISO or directories for use with re3.

## Usage

```shell
re3-installer ISO1|DIR1 ISO2|DIR2
```

_ISO1_ must be the first disc as an ISO image.

_ISO2_ must be the second disc as an ISO image.

_DIR1_ must be the directory containing data1.cab, data1.hdr, and data2.cab from the first disc.

_DIR2_ must be the directory containing the Audio directory from the second disc.

To support ISO extraction, `7z` from p7zip must be in `PATH`.

Both arguments must be of the same type.

This project supports Linux and macOS, and possibly BSD. Does not yet support Windows.

## How to build

**Requirements**: CMake and libunshield in a standard library path (i.e. `/usr/lib64`).

```shell
cd re3-installer/  # cloned repository
cmake -S . -B build
cmake --build build
```
