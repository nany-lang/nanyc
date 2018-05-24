# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- nanyc: support for collections, via `uses` (ex: `uses std.digest.md5;`)
- nsl: add `std.math.equals(a, b)`
- nsl: add collection `nsl.selftest`, for NSL unittests
- nsl: add collection `std.disgest.md5`
- nsl: add hash functions (`std.hash()`)
- nsl: add integer cast via `as<:T:>()`
- nsl: add modulo operator for integers
- nsl: add std.asBuiltin, to convert high-level to low level compiler builtins
- nsl: add xor operator for integers
- nsl: C: add typedef `std.c.intptr_t` and `std.c.uintptr_t`
- nsl: C: add typedef `std.c.size_t` (for C `size_t`)
- nsl: C: add typedef `std.c.ssize_t` (for C `ssize_t`)
- nsl: C: add typedefs for floating point data types

### Changed
- ci: add ubuntu-18.04-lts
- language: `;` is now mandatory after a namespace declaration
- nanyc: Start using "changelog" based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
- nanyc: the version is now carried by the git tag (0.0.0 otherwise)
- tests: `nanyc-unittests` does no longer require an empty filename for running the NSL Selftest

### Removed
- ci: remove ubuntu 17.04 in favor of ubuntu 18.04
- TravisCI is no longer supported

### Fixed
* nanyc: a potential data corruption in `nanyc-unittest` in multithreaded mode
* nanyc: crash when parse error occurs with namespace declaration
* nanyc: data corruption with `std.env.*` functions
* nanyc: segv that may occur with a parse error within a class
- nanyc: fix generic parameters not considered when no normal parameter is present


## [0.3.0]

### Added
- language: Error handlers via `raise` and `on scope fail`
- language: Scope guard statement via `on scope {};`
- nsl: `std.Optional<:T:>`, `func std.optional()`
- nsl: unittests!
- tests: multithreading support for `nanyc-unittest`

### Changed
- Migrated to [gitlab](https://gitlab.com/nany-lang/nanyc).
- C-API, entirely revamped. Many features have been removed in the process
  and will be reintroduced later.
- New .deb packages for debian 8-9 and ubuntu 16-17

### Fixed
- Numerous bugs


## [0.2.0]

### Added
- language: new in-place constructor:

	new (inplace: ptr) T(param1, param2);

- language: attribute `nodiscard`, but produces no effect yet
- language: attribute `per`, but produces no effect yet
- language: operators can now be called as any other function:

	class A
	{
		func foo(x) -> self.operator() (x);
		operator () (x) -> x * 2;
	}

- language: ascii support
- language: for-else clause, when the view is empty
- nsl: `std.Ascii`
- nsl: `pointer<:T:>`, for using weak pointers
- nsl: std.memory, for low level memory allocations


### Changed
- language: new syntax for attributes: #[name: optional-value]. This change
  would avoid ambiguities with syntactic sugar for arrays.
- language: attributes are now accepted by all expressions and are no longer
  reserved to functions, classes and variables.
