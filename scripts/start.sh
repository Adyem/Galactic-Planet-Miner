#!/usr/bin/env bash
set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if [ ! -d "libft/.git" ]; then
    git submodule update --init --recursive libft
else
    git submodule update --recursive --remote libft
fi

make "$@"
