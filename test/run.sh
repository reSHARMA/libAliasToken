#!/bin/bash

OPT="$LLVM_HOME/bin/opt"
CC="$LLVM_HOME/bin/clang"

for x in src/*; do
    $CC -S -emit-llvm -o /tmp/test.ll $x
    $OPT -instnamer -load /usr/local/lib/libAliasToken.so -load ./libTestPass.so -test /tmp/test.ll > /dev/null
done
