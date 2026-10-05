#!/bin/sh
set -eu

# Resolve the sources relative to this script, regardless of the caller's cwd.
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$project_dir"

if ! command -v nasm >/dev/null 2>&1; then
    echo 'Error: NASM is required. Install nasm and try again.' >&2
    exit 1
fi

if command -v dosbox >/dev/null 2>&1; then
    emulator=dosbox
elif command -v dosbox-staging >/dev/null 2>&1; then
    emulator=dosbox-staging
else
    echo 'Error: DOSBox is required. Install DOSBox or DOSBox Staging.' >&2
    exit 1
fi

echo 'Building dodge.com...'
nasm -f bin main.asm -o dodge.com
exec "$emulator" -c 'mount c .' -c 'c:' -c 'dodge.com' -c 'exit'
