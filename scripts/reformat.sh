#!/bin/bash

scriptDir=$(dirname $0)

ver=$(uncrustify --version)
if [ ! "$ver" = "Uncrustify-0.72.0_f" ] ; then
  echo "Bad version of uncrustify, skipping formatting"
  exit 0
fi

echo "Reformatting..."

function list_files
{
  find src -name '*.cpp' -or -name '*.h'
}

list_files | while read f; do
  uncrustify -c "$scriptDir/uncrustify.cfg" -f "$f" -o "$f.tmp" -q
  if ! diff -Naur "$f" "$f.tmp" ; then
    echo "Formatting $f"
    mv "$f.tmp" "$f"
  else
    rm "$f.tmp"
  fi
done
