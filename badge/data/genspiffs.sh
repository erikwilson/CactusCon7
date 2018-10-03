#!/bin/bash

mkdir pub
mkdir fs
mkdir spiffs

for i in {1..542}; do
    echo badge$i
    mkdir fs/badge$i
    openssl genrsa -out fs/badge$i/private.pem 768
    openssl rsa -in fs/badge$i/private.pem -pubout > pub/$i
    echo $i > fs/badge$i/my.id
    cp cactuscoinapi.pem fs/badge$i/
    ~/Downloads/mkspiffs/mkspiffs-0.2.3-2-g4eae236-arduino-esp32-linux64/mkspiffs -c fs/badge$i/ -b 4096 -p 256 -s 0x16F000 spiffs/badge_$i.bin
done
