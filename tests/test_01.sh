#!/bin/bash

FILES="../images/dwarf_small.bmp
../images/dwarf_gray.bmp
../images/dwarf.bmp"
CONTAINER="../images/weber.bmp"

for file in $FILES; do
    ../stg $file $CONTAINER
    ../stg merged_image.bmp
    cmp obr1_separated.bmp $file && echo "file $file OK"
    rm obr1_separated.bmp
    rm obr2_separated.bmp
    rm merged_image.bmp
done