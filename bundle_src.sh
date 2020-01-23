#!/bin/bash
# bundle_src.sh - bundle source code into a release package
# written by Christian Vogelgsang <chris@vogelgsang.org>
# under the GNU Public License V2

APP=DiskImagery64
VERSION_FILE=imagery/version.h
VERSION=`awk '{ print $3 }' $VERSION_FILE | sed -e 's/"//g'`
echo "-- Bundling Source for Version: $VERSION"

if [ -e Makefile ]; then
  echo "make distclean"
  make distclean
fi

ARCHIVE="BUILD/$APP-$VERSION-src.zip"
find . -not -path '*/.*' -and -not -path './BUILD*' | zip -9 $ARCHIVE -@

echo "ready: $ARCHIVE"
du -sh $ARCHIVE
md5 -q $ARCHIVE
 