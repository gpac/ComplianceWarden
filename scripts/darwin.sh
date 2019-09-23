#!/bin/sh
set -eu

params=()
for p in "$@" ; do
  case $p in
    -Wl,-gc-sections)
      ;;
    *)
      params+=("$p")
      ;;
  esac
done

"/usr/bin/g++" "${params[@]}"

