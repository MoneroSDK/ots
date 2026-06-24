# About the OTS library {#mainpage}

Static library complete in itself to quick write a offline signing application for Monero.
All monero related source is capsuled into the library. The target is to enable a developer
to write a offline transaction signing device/app/application with all documentation an batteries
included.

The static library comes with `ots.hpp` which is the interface for C++, and a `ots.h` which uses
internally the C++ implementation, but wraps actually the whole functionallity into a C ABI interface
which can be used by C, but main purpose is to use it via FFI.

## Build
In the monero repository:
```sh
mkdir build && cd build
cmake ../ots
make
make docs # to create this very documentation from the source
```

## C++ library
All you need is:
 - include/ots.hpp
 - include/ots-exceptions.hpp
 - libmonero-ots.a

## C ABI library
 - include/ots.h
 - libmonero-ots.a

## Other languages
Libraries for other languages can provided by a wrapper arround the C ABI

### Python
For the python implementation see [Monero OTS python](https://github.com/MoneroOTS/monero-ots-python).

### Java/Kotlin
For the JNI implementation see [Monero OTS Java](https://github.com/MoneroOTS/monero-ots-java).

## Using in build root

### OTS library
See [buildroot-monero-ots](https://github.com/MoneroOTS/buildroot-monero-ots).

### OTS library for Python
See [buildroot-monero-ots-python](https://github.com/MoneroOTS/buildroot-monero-ots-python).

## Devices, Applications and Apps using this library

### XmrSigner
The main purpose this library is developed. [XmrSigner github](https://github.com/XmrSigner).

## Further extensive documentation
The main documentation you will find hopefully in the future on [docs.getmonero.org](https://otslib.monerodevs.org/sdk/ots/) after the library got merged into the [monero source tree](https://github.com/monero-project/monero), until then you will find it while in development on [otslib.monerodevs.org](https://otslib.monerodevs.org/sdk/ots/) (which is friendly provided by [ofrnxmr(nahuhh on github)](https://github.com/nahuhh)
