#!/bin/sh

LUABIN=$1

. tests/luaenv.sh
$LUABIN tests/testBcdArithmetics.lua -V


