## Installation with LuaRocks
----------------------------
```Bash
luarocks install penlight
luarocks install LuaBcd
```


## Ubuntu 20.04 on x86_64, i686
----------------------------

### Build system
Cmake with gcc or clang with C++11 support.

### Prerequisites
Install packages with 'apt-get'/aptitude.

#### Required packages
lua5.2 liblua5.2-dev luarocks llvm

#### Required luarocks packages
penlight

### Build from github
#### Fetch sources
```Bash
git clone https://github.com/patrickfrey/lua_bcd
cd lua_bcd
```
#### Configure to find Lua includes and to write the file Lua.inc included by make
```Bash
./configure
```

#### Build with GNU C/C++
```Bash
make -DCOMPILER=gcc -DRELEASE=true
```

#### Build with Clang C/C++
```Bash
make -DCOMPILER=clang -DRELEASE=true
```

#### Run tests
```Bash
make test
```

#### Install
```Bash
make PREFIX=/usr/local install
```

