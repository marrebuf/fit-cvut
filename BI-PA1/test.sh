#!/bin/bash
# Test script. Some of the homeworks in BI-PA1 have had a set of test
# inputs and outputs that were distributed in an archive.

die ()
{
	echo $1 >&2
	exit 1
}

[ $# -le 1 ]  && die "Použití: sample.sh archiv binárka"

[ ! -f "$1" ] && die "Archiv neexistuje."
[ ! -f "$2" ] && die "Binárka neexistuje."
[ ! -x "$2" ] && die "Binárka má špatná práva."

archive=`readlink -f $1 || which $1`
 binary=`readlink -f $2 || which $2`

tar tzf $archive | grep 'CZE/.*\.txt' -cq \
	|| die "S tímhle archivem si nevím rady."

cd /tmp && mkdir -p progtest && cd progtest \
	|| die "Nemůžu se dostat do /tmp/progtest."

# Well... let's test it now.
tar xzf $archive CZE \
	|| die "Nepovedlo se mi rozbalit archiv."

for file in CZE/*_in.txt; do
	echo "=== Ověřuju ${file%%_in.txt}..."
	diff -u <($binary < ${file}) ${file%%_in.txt}_out.txt
done

cd /tmp && rm -rf progtest \
	|| die "Nemůžu po sobě uklidit."

