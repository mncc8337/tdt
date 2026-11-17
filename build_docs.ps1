# build_docs.ps1 — download the remote images referenced by docs.typ into a
# temporary bin\ folder, compile the PDF, then delete bin\.
#
# Usage: powershell -ExecutionPolicy Bypass -File build_docs.ps1 [-Source docs.typ] [-Output docs.pdf]
#
# How it works: Typst cannot fetch URLs, so this script compiles a temporary
# copy: the images are saved under bin\img\ and the copy inside bin\ references
# them with local relative paths. The original source file is never modified.

param(
    [string]$Source = "docs.typ",
    [string]$Output = "docs.pdf"
)

$ErrorActionPreference = "Stop"

# Always operate from the directory the script lives in (project root).
Set-Location -LiteralPath $PSScriptRoot

$Bin = Join-Path $PSScriptRoot "bin"

try {
    $sourcePath = Join-Path $PSScriptRoot $Source
    if (-not (Test-Path -LiteralPath $sourcePath)) {
        throw "error: $Source not found"
    }

    $text = [System.IO.File]::ReadAllText($sourcePath)

    # 1. Collect the remote image URLs from the document.
    $urls = [regex]::Matches($text, 'image\("(https?://[^"]+)"') |
        ForEach-Object { $_.Groups[1].Value }

    if ($urls.Count -eq 0) {
        Write-Host "No remote images found — compiling directly."
        & typst compile $Source $Output
        if ($LASTEXITCODE -ne 0) { throw "typst failed" }
        Write-Host "Built $Output"
        return
    }

    # 2. Download each image into bin\ and rewrite the URL to a local path.
    New-Item -ItemType Directory -Force -Path (Join-Path $Bin "img") | Out-Null

    $i = 0
    foreach ($url in $urls) {
        $i++
        $base = ($url -split '\?', 2)[0]                 # strip query string
        $ext = [System.IO.Path]::GetExtension($base)
        if ([string]::IsNullOrWhiteSpace($ext)) { $ext = ".png" }
        $local = "img/{0:D3}{1}" -f $i, $ext.ToLower()

        Write-Host "Downloading $url -> bin/$local"
        & curl.exe -fsSL --retry 2 -A "Mozilla/5.0 (typst-doc-builder)" `
            -o (Join-Path $Bin $local) $url
        if ($LASTEXITCODE -ne 0) { throw "curl failed for $url" }

        $text = $text.Replace($url, $local)
    }

    # 3. Write the rewritten copy next to the downloaded images and compile.
    $copy = Join-Path $Bin (Split-Path -Leaf $Source)
    [System.IO.File]::WriteAllText($copy, $text)

    & typst compile $copy $Output
    if ($LASTEXITCODE -ne 0) { throw "typst failed" }
    Write-Host "Built $Output"
}
finally {
    if (Test-Path -LiteralPath $Bin) {
        Remove-Item -Recurse -Force $Bin
    }
}
