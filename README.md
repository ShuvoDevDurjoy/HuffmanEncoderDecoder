# HuffmanEncoderDecoder

A command-line Huffman encoder and decoder for binary files.

## Install

```bash
curl -fsSL https://raw.githubusercontent.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/main/install.sh | bash
```

The installer downloads the latest GitHub Release binary and places it at `~/.local/bin/huff`. Make sure that directory is in your PATH:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

The release installer currently supports Linux x86_64.

To install a specific release or use another location:

```bash
curl -fsSL https://raw.githubusercontent.com/ShuvoDevDurjoy/HuffmanEncoderDecoder/main/install.sh | HUFF_VERSION=v1.0.0 PREFIX=/usr/local bash
```

## Usage

```bash
huff encode <file>
huff decode <file.huff> <output-file>
huff
```

Encoding writes `<file>.huff` next to the input file.

```bash
huff encode notes.txt
```

Decoding requires the encoded input file and an output path.

```bash
huff decode notes.txt.huff notes.decoded.txt
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

## Release

Create and push a version tag to publish a release binary:

```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub Actions will build `huff-linux-x86_64.tar.gz`, attach it to the release, and the one-line installer will start using it as the latest release.
