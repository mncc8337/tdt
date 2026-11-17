#!/usr/bin/env bash
# build_docs.sh — download the remote images referenced by docs.typ into a
# temporary bin/ folder, compile the PDF, then delete bin/.
#
# Usage: ./build_docs.sh [source.typ] [output.pdf]
#   (defaults: docs.typ docs.pdf)
#
# How it works: Typst cannot fetch URLs, so this script mirrors the trick of
# compiling a temporary copy: the images are saved under bin/img/ and the copy
# inside bin/ references them with local relative paths. The original source
# file is never modified.

set -euo pipefail

source_file="${1:-docs.typ}"
output_pdf="${2:-docs.pdf}"
bin_dir="bin"

# Always operate from the directory the script lives in (project root).
cd "$(dirname "$0")"

cleanup() { rm -rf "$bin_dir"; }
trap cleanup EXIT

if [ ! -f "$source_file" ]; then
    echo "error: $source_file not found" >&2
    exit 1
fi

# 1. Collect the remote image URLs from the document.
mapfile -t urls < <(grep -oE 'image\("https?://[^"]+"' "$source_file" | sed -E 's/^image\("//; s/"$//')

if [ "${#urls[@]}" -eq 0 ]; then
    echo "No remote images found — compiling directly."
    typst compile "$source_file" "$output_pdf"
    echo "Built $output_pdf"
    exit 0
fi

# 2. Download each image into bin/ and rewrite the URL to a local path.
mkdir -p "$bin_dir/img"
content="$(<"$source_file")"

i=0
for url in "${urls[@]}"; do
    i=$((i + 1))
    base="${url%%\?*}"                       # strip query string
    ext="${base##*.}"
    case "$ext" in png|jpg|jpeg|gif|svg) ;; *) ext="png" ;; esac
    local_path="img/$(printf '%03d' "$i").$ext"

    echo "Downloading $url -> $bin_dir/$local_path"
    curl -fsSL --retry 2 -A "Mozilla/5.0 (typst-doc-builder)" \
        -o "$bin_dir/$local_path" "$url"

    content="${content//$url/$local_path}"
done

# 3. Write the rewritten copy next to the downloaded images and compile.
copy_name="$(basename "$source_file")"
printf '%s\n' "$content" > "$bin_dir/$copy_name"

typst compile "$bin_dir/$copy_name" "$output_pdf"
echo "Built $output_pdf"
