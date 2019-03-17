# pfederc

Documentation at [https://codefionn.github.io/pfederc]. Language documentation
at [https://codefionn.github.io/federlang].

## Build

Build Release target:

```bash
mkdir build ; cd build
cmake -DCMAKE_BUILD_TYPE=Release .. ; cmake --build . -j
```

Testing:

```bash
cd build/test
ctest
```

## Documentation

Online at [https://codefionn.github.io/pfederc]. Build and open with:

```bash
doxygen .doxyconf
xdg-open docs/index.html
```
