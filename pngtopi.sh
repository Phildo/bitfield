#!/bin/bash

if [ $# -ne 1 ] ; then echo need arg; exit 1; fi
name=`echo $1 | sed 's/.png$//'`
w=`identify ~/Desktop/bug_3.png | awk '{ print $3}' | sed s/x.*$//`
h=`identify ~/Desktop/bug_3.png | awk '{ print $3}' | sed s/^.*x//`
convert $1 $name.bmp
./bf -i $name.bmp -o $name.${w}x${h}pi

