<p align="center">
  <img src=".github/banner.svg" alt="IW4x (x64) Banner"/>
</p> <hr>

## Building from Source

IW4x is built using the [build2](https://build2.org/build2-toolchain/doc/build2-toolchain-intro.xhtml#preface) build system. To get started, you'll need the staged toolchain, which you can grab as either source or binary packages. The pre-built binaries are available here:

* [Staged Toolchain](https://stage.build2.org/0/0.19.0-a.0/bindist/)
* [SHA256 Checksums](https://stage.build2.org/0/toolchain-bindist.sha256)

You'll also need a working MinGW++ compiler. If you need help setting it up, you can find installation instructions below:

* [MSYS2](https://www.msys2.org/)
* [MinGW-w64](https://www.mingw-w64.org/)

### Consumption

> [!NOTE]
> **Consumption** is just `build2`'s term for building the package without actively modifying the code. These steps are great if you want to test the latest development builds, but we don't recommend them for regular users just looking to install and use IW4x.

#### Windows

```powershell
# Create the build configuration:
#
bpkg create -d iw4x -@mingw32                                    ^
  config.cxx=x86_64-w64-mingw32-g++                              ^
  cc                                                             ^
  config.bin.lib=static                                          ^
  config.install.bin=/path/to/game/folder                        ^
  config.install.root=/path/to/game/folder                       ^
  config.install.filter='include/@false lib/@false share/@false'

cd iw4x

# To build:
#
bpkg build libiw4x@https://github.com/iw4x-x64/iw4x.git#main

# To test:
#
bpkg test libiw4x

# To install:
#
bpkg install libiw4x

# To uninstall:
#
bpkg uninstall libiw4x

# To upgrade:
#
bpkg fetch
bpkg status libiw4x
bpkg uninstall libiw4x
bpkg build --upgrade --recursive libiw4x
bpkg install libiw4x
```

#### Linux

```bash
# Create the build configuration:
#
bpkg create -d iw4x @mingw32                                     \
  config.cxx=x86_64-w64-mingw32-g++                              \
  cc                                                             \
  config.bin.lib=static                                          \
  config.install.bin=/path/to/game/folder                        \
  config.install.root=/path/to/game/folder                       \
  config.install.filter='include/@false lib/@false share/@false'

cd iw4x

# To build:
#
bpkg build libiw4x@https://github.com/iw4x-x64/iw4x.git#main

# To test:
#
bpkg test libiw4x

# To install:
#
bpkg install libiw4x

# To uninstall:
#
bpkg uninstall libiw4x

# To upgrade:
#
bpkg fetch
bpkg status libiw4x
bpkg uninstall libiw4x
bpkg build --upgrade --recursive libiw4x
bpkg install libiw4x
```

If you want to dive deeper into how these commands work, check out the [build2 package consumption guide](https://build2.org/build2-toolchain/doc/build2-toolchain-intro.xhtml#guide-consume-pkg).

### Development

> [!NOTE]
> These instructions are meant for developers looking to modify and contribute to IW4x's codebase. If you just want to compile and test the latest build, follow the **Consumption** steps above instead.

#### Linux

```bash
# Clone the repository:
#
git clone https://github.com/iw4x-x64/iw4x.git

cd iw4x

# Create the build configuration:
#
bdep init -C @mingw32                               \
  config.cxx=x86_64-w64-mingw32-g++                 \
  cc                                                \
  config.cc.compiledb=./                            \
  config.cc.coptions="-ggdb                         \
                      -grecord-gcc-switches         \
                      -fno-omit-frame-pointer       \
                      -mno-omit-leaf-frame-pointer"

# To build:
#
b

# To test:
#
b test
```

#### Windows

```powershell
# Clone the repository:
#
git clone https://github.com/wroyca/iw4x.git

cd iw4x

# Create the build configuration:
#
bdep init -C -@mingw32                              ^
  config.cxx=x86_64-w64-mingw32-g++                 ^
  cc                                                ^
  config.cc.compiledb=./                            ^
  config.cc.coptions="-ggdb                         ^
                      -grecord-gcc-switches         ^
                      -fno-omit-frame-pointer       ^
                      -mno-omit-leaf-frame-pointer"

# To build:
#
b

# To test:
#
b test
```

For more details on `bdep` and typical development workflows, take a look at the [build2 build system manual](https://build2.org/build2/doc/build2-build-system-manual.xhtml).

## Contributing

Contributions are welcome! If you are interested in helping expand IW4x, please feel free to submit a Pull Request.

## License

IW4x is licensed under the [GNU General Public License v3](LICENSE.md).
