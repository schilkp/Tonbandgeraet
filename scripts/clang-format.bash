#!/usr/bin/env bash
set -euo pipefail

# Process from parent folder (repo root):
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

CLANG_FORMAT="uvx --with clang-format==22.1.0 clang-format"

CHECK=0
if [[ "${1:-}" == "--check" ]]; then
  CHECK=1
fi

# Only format Tonbandgeraet's own sources. Vendored code is excluded.
mapfile -d '' -t files < <(
  git ls-files -z --cached --others --exclude-standard -- \
      'tband' \
      'tests' \
      'examples' \
    | grep -z -E '\.(c|h)$' \
    | grep -z -v -E '^tests/resources/(Unity|fff)/' \
    | grep -z -v -E '^examples/[^/]+/Drivers/' \
    || true
)

if [[ ${#files[@]} -eq 0 ]]; then
  echo "No c(++) source files to format"
  exit 1
fi

if [[ $CHECK -eq 1 ]]; then
  $CLANG_FORMAT --dry-run --Werror --ferror-limit=0 "${files[@]}"
else
  $CLANG_FORMAT -i "${files[@]}"
fi
