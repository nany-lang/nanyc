How to build Nany
=================


Building Nany C++/bootstrap
---------------------------

### Dependencies

 * `>=CMake-3.0`
 * C++14 compliant compilers: `>=gcc-4.9`, `>=VS2015`

Make sure that submodules are present:
```
$ git submodule update --init --recursive
```


### UNIXes / Makefile

Generate the makefiles:
```
$ cd bootstrap
$ cmake .

```

Build:
```
$ make -j $(nproc)
```


### Windows / Visual Studio IDE


Generate the MS Projects:
```
$ cd bootstrap
$ cmake . -G "Visual Studio 14 2015 Win64"
```

Build the bootstrap from the command line:
```
$ msbuild nany-boostrap.sln
```

**Note**: For automating the build process with Visual Build Tools, please refer
to the `appveyor.yml` file at the root directory of the repository.
