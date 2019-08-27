#!/bin/bash

cd src

LFMakeMaker -v +f=Makefile --opts="-Wall -DxDEBUG -Wno-parentheses" *.c -t=../FCMSubmit > Makefile

