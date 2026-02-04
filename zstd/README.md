# Zstandard Library for MUSHclient

## Version Information
**Current Version:** 1.5.7  
**Release Date:** February 20th, 2025
**Implementation Type:** Source code (compiled directly into MUSHclient)

## Source
These source files were obtained from the official Zstandard source release:
- **Download URL:** https://github.com/facebook/zstd/releases/download/v1.5.7/zstd-1.5.7.tar.gz
- **GitHub Repository:** https://github.com/facebook/zstd
- **Note:** We are compiling from source rather than using precompiled binaries for Visual Studio compatibility

## Files Included

### Header Files
- `zstd.h` - Main API header
- `zstd_errors.h` - Error code definitions

### Common Source Files (in `common/` directory)
- `zstd_common.c` - Common utility functions
- `entropy_common.c` - Entropy coding common functions
- `fse_decompress.c` - FSE decompression
- `xxhash.c` - XXHash algorithm implementation
- `error_private.c` - Error handling
- `debug.c` - Debug utilities
- Various header files for internal use

### Decompression Source Files (in `decompress/` directory)
- `huf_decompress.c` - Huffman decompression
- `zstd_ddict.c` - Decompression dictionary
- `zstd_decompress.c` - Main decompression implementation
- `zstd_decompress_block.c` - Block decompression
- Various header files for internal use

### License
- `LICENSE` - BSD license for Zstandard

## Build Configuration
The source files are compiled directly into MUSHclient via the Visual Studio project file (MUSHclient.vcxproj). This provides:
- Better optimization opportunities
- No external DLL dependencies
- Consistent behavior across different Windows versions

## How to Update

To update to a newer version of Zstandard:

1. **Download the latest source release:**
   - Visit https://github.com/facebook/zstd/releases
   - Download the source code (ZIP or tar.gz)

2. **Extract required files:**
   - From `lib/common/`: Extract all `.c` and `.h` files to `zstd/common/`
   - From `lib/decompress/`: Extract all `.c` and `.h` files to `zstd/decompress/`
   - From `lib/`: Extract `zstd.h` and `zstd_errors.h` to `zstd/`
   - **Note:** Only decompression files are needed (MCCP4 only requires decompression, not compression)

3. **Update version information:**
   - Update the version number and release date in this README

4. **Update Visual Studio project if needed:**
   - If new source files were added, add them to MUSHclient.vcxproj
   - Remove any obsolete files from the project

5. **Test compatibility:**
   - Rebuild MUSHclient
   - Test MCCP4 compression functionality
   - Verify no API breaking changes affect mccp4.cpp

## Implementation Notes
- MUSHclient only uses Zstandard's **decompression** functionality for now, for MCCP4 support
- Compression files are not included as MUSHclient does not compress data to send to MUDs, but COULD in the future via client-side compression.
- The implementation provides MCCP4 (MUD Client Compression Protocol version 4) support with better compression ratios and performance compared to zlib-based MCCP versions 1-3

## License
Zstandard is licensed under the BSD license. See the LICENSE file in this directory for full terms.

Copyright (c) Meta Platforms, Inc. and affiliates. All rights reserved.