#!/bin/bash

set -e

while read f; do
    echo $f
    name=$( basename "$f" )
    echo $name...
    rm -f /tmp/music.wav
    mpg123 -w /tmp/music.wav "$f"
    cat /tmp/music.wav | ../bin/plot-wave -w 1920 -h 1080 ../graphics/"${name/mp3/png}"
done
