# re3-installer

## Usage

```
re3-installer ISO1|DIR1 ISO2|DIR2
```

_ISO1_ must be the first disc as an ISO image.

_ISO2_ must be the second disc as an ISO image.

_DIR1_ must be the directory containing data1.cab, data1.hdr, and data2.cab from the first disc.

_DIR2_ must be the directory containing the Audio directory from the second disc.

To support ISO extraction, `7z` from p7zip must be in `PATH`.

Both arguments must be of the same type.

This project supports Linux and macOS, and possibly BSD. Does not yet support Windows.
