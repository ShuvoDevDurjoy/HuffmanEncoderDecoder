$ErrorActionPreference = "Stop"

$Repo = if ($env:HUFF_REPO) { $env:HUFF_REPO } else { "ShuvoDevDurjoy/HuffmanEncoderDecoder" }
$Version = if ($env:HUFF_VERSION) { $env:HUFF_VERSION } else { "latest" }
$InstallDir = if ($env:HUFF_INSTALL_DIR) { $env:HUFF_INSTALL_DIR } else { Join-Path $env:LOCALAPPDATA "Programs\huff\bin" }
$BinName = if ($env:HUFF_BIN_NAME) { $env:HUFF_BIN_NAME } else { "huff.exe" }

function Fail($Message) {
    Write-Error "error: $Message"
    exit 1
}

function Get-ReleaseUrl($Asset) {
    if ($Version -eq "latest") {
        return "https://github.com/$Repo/releases/latest/download/$Asset"
    }

    return "https://github.com/$Repo/releases/download/$Version/$Asset"
}

$Arch = $env:PROCESSOR_ARCHITECTURE
if ($Arch -ne "AMD64" -and $Arch -ne "x86_64") {
    Fail "unsupported architecture: $Arch"
}

$Asset = "huff-windows-x86_64.zip"
$Url = Get-ReleaseUrl $Asset
$TempDir = Join-Path ([System.IO.Path]::GetTempPath()) ("huff-install-" + [System.Guid]::NewGuid())
New-Item -ItemType Directory -Path $TempDir | Out-Null

try {
    $ArchivePath = Join-Path $TempDir $Asset
    Invoke-WebRequest -Uri $Url -OutFile $ArchivePath

    $ShaUrl = "$Url.sha256"
    $ShaPath = "$ArchivePath.sha256"
    try {
        Invoke-WebRequest -Uri $ShaUrl -OutFile $ShaPath
        $ExpectedHash = ((Get-Content $ShaPath -Raw).Trim() -split "\s+")[0].ToUpperInvariant()
        $ActualHash = (Get-FileHash $ArchivePath -Algorithm SHA256).Hash.ToUpperInvariant()

        if ($ExpectedHash -ne $ActualHash) {
            Fail "checksum verification failed"
        }
    }
    catch {
        Write-Warning "checksum file was not available; continuing without verification"
    }

    Expand-Archive -Path $ArchivePath -DestinationPath $TempDir -Force
    $ExePath = Join-Path $TempDir "huff.exe"

    if (!(Test-Path $ExePath)) {
        Fail "release archive did not contain huff.exe"
    }

    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    Copy-Item $ExePath (Join-Path $InstallDir $BinName) -Force

    Write-Host "installed huff to $(Join-Path $InstallDir $BinName)"

    $UserPath = [Environment]::GetEnvironmentVariable("Path", "User")
    $PathParts = $UserPath -split ";"

    if ($PathParts -notcontains $InstallDir) {
        [Environment]::SetEnvironmentVariable("Path", "$UserPath;$InstallDir", "User")
        Write-Host "added $InstallDir to your user PATH"
        Write-Host "open a new PowerShell window before running huff"
    }
}
finally {
    Remove-Item $TempDir -Recurse -Force -ErrorAction SilentlyContinue
}
