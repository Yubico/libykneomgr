#!/bin/sh

set -e
set -x

touch ChangeLog
autoreconf -i

if [ "x$ARCH" != "x" ]; then
    version=`cat NEWS  | grep unreleased | cut -d' ' -f3`
    set +e
    tar --exclude .git --transform="s/^\./libykneomgr-${version}/" -czf libykneomgr-${version}.tar.gz .
    set -e

    make -f windows.mk ${ARCH}bit VERSION=$version EXTRA=""
else
    ./configure
    make check
fi
