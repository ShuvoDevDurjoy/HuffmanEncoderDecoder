# HuffmanEncoderDecoder

A command-line Huffman encoder and decoder for binary files.

## Install

Linux:

```bash
curl -fsSL https://raw.githubusercontent.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/main/install.sh | bash
```

Windows PowerShell:

```powershell
irm https://raw.githubusercontent.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/main/install.ps1 | iex
```

The installer downloads the latest GitHub Release binary. On Linux it places `huff` at `~/.local/bin/huff`. Make sure that directory is in your PATH:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

On Windows it places `huff.exe` in `%LOCALAPPDATA%\Programs\huff\bin` and adds that directory to your user PATH. Open a new PowerShell window after installing.

The release installers currently support Linux x86_64 and Windows x86_64.

To install a specific Linux release or use another location:

```bash
curl -fsSL https://raw.githubusercontent.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/main/install.sh | HUFF_VERSION=v1.0.0 PREFIX=/usr/local bash
```

For Windows:

```powershell
$env:HUFF_VERSION = "v1.0.0"; irm https://raw.githubusercontent.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/main/install.ps1 | iex
```

## Usage

```bash
huff encode <file-or-folder>
huff decode <file.huff> <output-directory>
huff
```

Encoding writes a `.huff` file next to the input. A folder is stored as one archive, including its nested directory and file names.

```bash
huff encode notes.txt
huff encode project/
```

Decoding requires the encoded input file and an output directory. For a folder archive, the original folder is recreated inside that directory.

```bash
huff decode notes.txt.huff restored/
huff decode project.huff restored/
```

Payload data is processed in fixed-size buffers, so memory use does not grow with file size. Folder metadata uses memory proportional to the number and total length of entry names. Encoding scans file contents twice to build a Huffman table and then write the encoded stream. Folder archives are limited to 100,000 entries and 16 MiB of combined paths and symlink targets. Relative symlinks that stay within the folder are preserved; absolute or escaping symlinks and special filesystem entries are not supported.

On Linux, you can make a long-running operation yield more to other work by lowering its scheduling priority:

```bash
nice -n 10 ionice -c2 -n7 huff encode project/
```

Run `huff` with no arguments to use the interactive mode.

## Development

```bash
git clone https://github.com/ShuvoDevDurjoy/HuffmanEncoderDecoder.git
cd HuffmanEncoderDecoder
make
./huff --help
make install
make clean
```
# Download Linux
- ## Version 1.0.2
- [huff-linux-x86_64.targ.gz](https://github.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/releases/download/v1.0.2/huff-linux-x86_64.tar.gz)
- [huff-linux-x86_64.tar.gz.sha256](https://github.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/releases/download/v1.0.2/huff-linux-x86_64.tar.gz.sha256)
# Download Windows
- ## Version 1.0.2
- [huff-windows-x86_64.zip](https://github.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/releases/download/v1.0.2/huff-windows-x86_64.zip)
- [huff-windows-x86_64.zip.sha256](https://github.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/releases/download/v1.0.2/huff-windows-x86_64.zip.sha256)

# Source Code
- [Source Code Zip](https://github.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/archive/refs/tags/v1.0.2.zip)
- [Source Code tar.gz](https://github.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/archive/refs/tags/v1.0.2.tar.gz)
## Release

Create and push a version tag to publish a release binary:

```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub Actions will build `huff-linux-x86_64.tar.gz` and `huff-windows-x86_64.zip`, attach them to the release, and the one-line installers will start using that release as the latest version.
