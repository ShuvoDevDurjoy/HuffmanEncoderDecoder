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

_tmpdir=""

main() {
    command -v tar >/dev/null 2>&1 || fail "tar is required"
    command -v install >/dev/null 2>&1 || fail "install is required"

    local asset
    local url
    asset="$(platform_asset)"
    url="$(release_url "$asset")"
    _tmpdir="$(mktemp -d)"
    trap 'rm -rf "$_tmpdir"' EXIT

    download "$url" "$_tmpdir/$asset"

    if command -v sha256sum >/dev/null 2>&1; then
        if download "$url.sha256" "$_tmpdir/$asset.sha256"; then
            (cd "$_tmpdir" && sha256sum -c "$asset.sha256" >/dev/null)
        fi
    fi

    tar -xzf "$_tmpdir/$asset" -C "$_tmpdir"
    [ -f "$_tmpdir/huff" ] || fail "release archive did not contain huff"

    install -d "$BINDIR"
    install -m 755 "$_tmpdir/huff" "$BINDIR/$BIN_NAME"

    printf 'installed %s to %s\n' "$BIN_NAME" "$BINDIR/$BIN_NAME"

    case ":$PATH:" in
        *":$BINDIR:"*) ;;
        *) printf 'add this to your shell profile: export PATH="%s:$PATH"\n' "$BINDIR" ;;
    esac
}

main "$@"
