#!/bin/sh
set -eu

params=()
for p in "$@" ; do
  case $p in
    -Wl,-gc-sections)
      ;;
    -s)
      ;;
    *)
      params+=("$p")
      ;;
  esac
done

"/usr/bin/g++" "${params[@]}"

