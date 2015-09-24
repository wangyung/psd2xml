#! /bin/sh

if [ $# -eq 0 ] ; then
    echo "Usage: $0 FILE.psd..."
    exit 0
fi

# Set up prog to be the path of this script, including following symlinks,
# and set up progdir to be the fully-qualified pathname of its directory.
prog="$0"
while [ -h "${prog}" ]; do
    newProg=`/bin/ls -ld "${prog}"`
    newProg=`expr "${newProg}" : ".* -> \(.*\)$"`
    if expr "x${newProg}" : 'x/' >/dev/null; then
        prog="${newProg}"
    else
        progdir=`dirname "${prog}"`
        prog="${progdir}/${newProg}"
    fi
done
oldwd=`pwd`
progdir=`dirname "${prog}"`
cd "${progdir}"
progdir=`pwd`
prog="${progdir}"/`basename "${prog}"`
cd "${oldwd}"

for f ; do
    n="${f##*/}"
    n="${n%.*}"
    if "$progdir/libexec/psdhtml" "$f" ; then
        cp -a "$progdir"/static/* "${n}_html/${n}.files/"
    fi
done
