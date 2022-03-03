#!/bin/bash
for dname in ./prt/scenes/cubemap/*
do
    filename=$(basename $dname)
    cp $dname/*.txt homework2/assets/cubemap/$filename/
done
