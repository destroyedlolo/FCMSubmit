#!/bin/bash

cd src

LFMakeMaker -v +f=Makefile --opts="\`pkg-config --cflags json-c\` \`pkg-config --libs json-c\` -lssl -lcrypto -lcurl -Wall -DxDEBUG -Wno-parentheses" *.c -t=../FCMSubmit > Makefile

