![](./logo.png)

# pfederc

*pfederc* (Passau Feder compiler) is a program for managing and compiling Feder
source code. Written in Standard C++14 and using LLVM8. This project doesn't
specify a Feder standard library.

*Passau is a city in Bavaria, Germany*.

## Download

Clone repository and jump into main directory of the project:

```bash
git clone https://github.com/federlang/pfederc
cd pfederc
```

## Build

CMake build:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Testing (could take some time):

```bash
cd build/test
ctest
```

If memory checks should also be done, install *valgrind*.

## Build with Docker

Build & run:

```bash
docker build --rm -t pfederc .
docker run --rm -ti pfederc
```

## Install on Linux

Install in ~/.local

```bash
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX='~/.local' -G 'Unix Makefiles' ..
make -j
make install
```

## Resources

About pfederc:

- API Documentation:
  [https://codefionn.github.io/pfederc](https://federlang.github.io/pfederc)
- Language documentation:
  [https://codefionn.github.io/feder-ref](https://federlang.github.io/feder-ref)

Some online resources used directly or as reference:

- [Precedence climbing method](https://en.wikipedia.org/wiki/Operator-precedence_parser)
- [C++-Itanium-ABI Mangling](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)
- [Tracing garbage collection](https://en.wikipedia.org/wiki/Tracing_garbage_collection)
- [C++ Operator Precedence](https://en.cppreference.com/w/cpp/language/operator_precedence)

And books:

- [The Garbage Collection Handbook](http://www.gchandbook.org/)
- [Compilers: Principles, Techniques, and Tools](https://www.worldcat.org/title/compilers-principles-techniques-and-tools/oclc/12285707) (Dragonbook)

## Documentation

Online at
[https://codefionn.github.io/pfeder](https://codefionn.github.io/pfederc).
Build and open with:

```bash
doxygen .doxyconf
xdg-open docs/index.html # Linux
```
