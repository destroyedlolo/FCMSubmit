#!/bin/bash

cd src

LFMakeMaker -v +f=Makefile --opts="-Wall -DxDEBUG" *.c -t=../FCMSubmit > Makefile

