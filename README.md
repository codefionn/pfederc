![](./logo.png)

# pfederc

*pfederc* (Passau Feder compiler) is a program for managing and compiling Feder
source code. Written in Standard C++14 and using LLVM. This project doesn't
specify a Feder standard library.

*Passau is a city in Bavaria, Germany*.

## Resources

About pfederc:

- API Documentation:
  [https://codefionn.github.io/pfederc](https://codefionn.github.io/pfederc)
- Language documentation:
  [https://codefionn.github.io/federlang](https://codefionn.github.io/federlang)

Some online resources used directly or as reference:

- Precedence climbing method:
  [https://en.wikipedia.org/wiki/Operator-precedence\_parser](https://en.wikipedia.org/wiki/Operator-precedence_parser)
- Mangling:
  [https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)
- Tracing garbage collection:
  [https://en.wikipedia.org/wiki/Tracing\_garbage\_collection](https://en.wikipedia.org/wiki/Tracing_garbage_collection)

And books:

- [The Garbage Collection Handbook](http://www.gchandbook.org/)
- [Compilers: Principles, Techniques, and Tools](https://www.worldcat.org/title/compilers-principles-techniques-and-tools/oclc/12285707) (Dragonbook)

## Build

Clone & build:

```bash
git clone https://github.com/codefionn/pfederc
cd pfederc
mkdir build ; cd build
cmake .. ; cmake --build .
```

Testing (in pfederc main directory):

```bash
cd build/test
ctest
```

If memory checks should also be done, install *valgrind*.

## Build with Docker

Clone, build & run:

```bash
git clone https://github.com/codefionn/pfederc
cd pfederc
docker build --rm -t pfederc .
docker run --rm -ti pfederc
```

## Documentation

Online at
[https://codefionn.github.io/pfeder](https://codefionn.github.io/pfederc).
Build and open with:

```bash
doxygen .doxyconf
xdg-open docs/index.html
```
