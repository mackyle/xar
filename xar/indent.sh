#!/bin/sh

for X
do
	indent -gnu -nut $X
	rm -f ${X}~ ${X}.BAK
done
