#!/usr/bin/env bash
set -euo pipefail

REPO="${HUFF_REPO:-ShuvoDevDurjoy/HuffmanEncoderDecoder}"
VERSION="${HUFF_VERSION:-latest}"
PREFIX="${PREFIX:-$HOME/.local}"
BINDIR="${BINDIR:-$PREFIX/bin}"
BIN_NAME="${BIN_NAME:-huff}"

fail() {
    printf 'error: %s\n' "$1" >&2
    exit 1
}

download() {
    local url="$1"
    local output="$2"

    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$url" -o "$output"
        return
    fi

    if command -v wget >/dev/null 2>&1; then
        wget -qO "$output" "$url"
        return
    fi

    fail "curl or wget is required"
}

platform_asset() {
    local os
    local arch
    os="$(uname -s)"
    arch="$(uname -m)"

    case "$os:$arch" in
        Linux:x86_64|Linux:amd64)
            printf 'huff-linux-x86_64.tar.gz'
            ;;
        *)
            fail "unsupported platform: $os $arch"
            ;;
    esac
}

release_url() {
    local asset="$1"

    if [ "$VERSION" = "latest" ]; then
        printf 'https://github.com/%s/releases/latest/download/%s' "$REPO" "$asset"
    else
        printf 'https://github.com/%s/releases/download/%s/%s' "$REPO" "$VERSION" "$asset"
    fi
}

main() {
    command -v tar >/dev/null 2>&1 || fail "tar is required"
    command -v install >/dev/null 2>&1 || fail "install is required"

    local asset
    local url
    asset="$(platform_asset)"
    url="$(release_url "$asset")"
    HUFF_TMPDIR="$(mktemp -d)"
    trap 'rm -rf "$HUFF_TMPDIR"' EXIT

    download "$url" "$HUFF_TMPDIR/$asset"

    if command -v sha256sum >/dev/null 2>&1; then
        if download "$url.sha256" "$HUFF_TMPDIR/$asset.sha256"; then
            (cd "$HUFF_TMPDIR" && sha256sum -c "$asset.sha256" >/dev/null)
        fi
    fi

    tar -xzf "$HUFF_TMPDIR/$asset" -C "$HUFF_TMPDIR"
    [ -f "$HUFF_TMPDIR/huff" ] || fail "release archive did not contain huff"

    install -d "$BINDIR"
    install -m 755 "$HUFF_TMPDIR/huff" "$BINDIR/$BIN_NAME"

    printf 'installed %s to %s\n' "$BIN_NAME" "$BINDIR/$BIN_NAME"

    case ":$PATH:" in
        *":$BINDIR:"*) ;;
        *) printf 'add this to your shell profile: export PATH="%s:$PATH"\n' "$BINDIR" ;;
    esac
}

main "$@"
