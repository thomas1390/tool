#!/bin/sh
for name in `ls ./ | grep FUZZY`
do
./bunzip2-exec-exitcode -d $name
done
