How to build Nany
=================


Building Nany C++/bootstrap
---------------------------

### Dependencies

 * `>=CMake-3.0`
 * C++14 compliant compiler: `>=gcc-4.9`, `>=clang-3.7`, `>=VS2015`

Make sure that submodules are present (if the repository has not been cloned
with `--recursive` option):
```
$ git submodule update --init --recursive
```


### UNIXes / Makefile

Generate the makefiles:
```
$ cd src/bootstrap
$ cmake .

```

Build:
```
$ make -j $(nproc)
```


### Windows / Visual Studio IDE


Generate the MS Projects:
```
$ cd src/bootstrap
$ cmake . -G "Visual Studio 14 2015 Win64"
```

Build the bootstrap from the command line:
```
$ msbuild nany-boostrap.sln
```

**Note**: For automating the build process with Visual Build Tools, please refer
to the `appveyor.yml` file at the root directory of the repository.


Generating packages for Debian/Redhat
-------------------------------------

**notes**: [fpm](https://github.com/jordansissel/fpm) (Effing Package Management) is required for generating deb/rpm packages

### Debian packages
Installing `fpm`:
```
$ apt-get install ruby-dev gcc make
$ gem install fpm
```

```
$ cd src/bootstrap
$ make package-deb
```

### RedHat/CentOS packages:

Installing `fpm`:
```
$ yum install ruby-devel gcc make
$ gem install fpm
```

```
$ cd src/bootstrap
$ make package-rpm
```
