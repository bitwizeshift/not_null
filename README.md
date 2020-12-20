# not_null

[![Ubuntu Build Status](https://github.com/bitwizeshift/not_null/workflows/Ubuntu/badge.svg?branch=master)](https://github.com/bitwizeshift/not_null/actions?query=workflow%3AUbuntu)
[![macOS Build Status](https://github.com/bitwizeshift/not_null/workflows/macOS/badge.svg?branch=master)](https://github.com/bitwizeshift/not_null/actions?query=workflow%3AmacOS)
[![Windows Build Status](https://github.com/bitwizeshift/not_null/workflows/Windows/badge.svg?branch=master)](https://github.com/bitwizeshift/not_null/actions?query=workflow%3AWindows)
[![Coverage Status](https://coveralls.io/repos/github/bitwizeshift/not_null/badge.svg?branch=master)](https://coveralls.io/github/bitwizeshift/not_null?branch=master)
[![Github Issues](https://img.shields.io/github/issues/bitwizeshift/not_null.svg)](http://github.com/bitwizeshift/not_null/issues)
<br>
[![Github Releases](https://img.shields.io/github/v/release/bitwizeshift/not_null.svg?include_prereleases)](https://github.com/bitwizeshift/not_null/releases)
[![Bintray Releases](https://api.bintray.com/packages/bitwizeshift/public-conan/NotNull%3Anot_null/images/download.svg)](https://bintray.com/bitwizeshift/public-conan/NotNull%3Anot_null/_latestVersion)
<br>
[![Try online](https://img.shields.io/badge/try-online-blue.svg)](https://godbolt.org/z/nPrnc7)

**not_null** is a 0-overhead modern utility for ensuring non-nullability
in a simple and coherent way.

Unlike `gsl::not_null`, this type can work with move-only pointers like
`std::unique_ptr`, and does not require runtime checks unless explicitly
specified.

*Stop worrying about nulls, and use `not_null` today!*

## Teaser

```cpp
auto register_widget(cpp::not_null<std::unique_ptr<Widget>> p) -> void
{
  legacy_service.adopt_widget(std::move(p).as_nullable());
}

...

// use 'cpp::check_not_null' to check and validate that 'p' is not null
register_widget(cpp::check_not_null(std::move(p)));
```

<kbd>[Live Example](https://godbolt.org/z/nPrnc7)</kbd>

## Features

* [x] Works with `unique_ptr`, and still allows nullability
* [x] Zero-Overhead; `not_null` has no hidden additional runtime
      or storage overhead.
* [x] Written to be compatible with <kbd>C++11</kbd>
* [x] Single-header, **header-only** solution -- easily drops into any project
* [x] No dependencies

## Documentation

* [Background](#background) \
  A background on the problem **not_null** solves
* [Tutorial](doc/tutorial.md) \
  A quick pocket-guide to using **not_null**
* [Installation](doc/installing.md) \
  For a quick guide on how to install/use this in other projects
* [API Reference](https://bitwizeshift.github.io/not_null/api/latest/) \
  For doxygen-generated API information
* [Attribution](doc/legal.md) \
  Information about how to attribute this project

## Background

When working in a codebase, it's often difficult to know when any given pointer
may legally contain a null pointer value. This requires the user to do one of
two possible things:

* Make an assumption that the pointer can never be `nullptr`, which risks the
  posibility of a null-pointer dereference leading to undefined behavior, or
* Always check for `nullptr` before using -- resulting in more complicated code
  paths, more branches, and potential pessimizations

This is the driving force behind a `not_null` utility. The Guidelines Support
Library offers `gsl::not_null` to attempt to satisfy this, though this has
two notable limitations:

* Construction and assignment of `not_null` is implicit, and requires runtime
  checks unconditionally (which may result in a contract violation); for example:
  ```cpp
  // Performs check that the pointer is not null, when the expression can never
  // take on the form of a non-null pointer
  gsl::not_null<int*> p = new int{42};
  ```

* it does not work with move-only pointer types like `unique_ptr`, for example:
  ```cpp
  gsl::not_null<std::unique_ptr<int>> p = std::make_unique<int>(42);
  // This errors due to 'p' not being movable!
  gsl::not_null<std::unique_ptr<int>> q = std::move(p);
  ```

This is where this library comes in.

Rather than allow implicit construction and assignment from the underlying
pointer, this library takes the hard stance of **correct on construction**,
which uses factory functions to produce the `not_null` objects. Two factories
are presented:

* `check_not_null`, which checks a pointer for nullability before raising a
  contract violation, which by default is thrown as an exception, or
* `assume_not_null`, which does no checking to produce the `not_null` and has
  zero overhead, which is to be used for known non-null values

With these, consumers are _required_ to consent to a check, or explicitly
state that their pointers are non-null, to reduce errors. This leaves any
remaining uses of `not_null` as nothing more than a 0-overhead wrapper; making
any runtime checks a manully consented operation. For example:

```cpp
// No check performed, since the expression can never be null!
cpp::not_null<int*> p = cpp::assume_not_null(new int{42});

// Get pointer from API that we *expect* to be null, but cannot guarantee
auto q = cpp::check_not_null(get_pointer());
```

Additionally, `not_null` allows for move-constructors and move-assignment, since
reusing an object after a move is generally a logic-error, and there are many
pieces of tooling that will check for such a case. This allows better interop
with types like `unique_ptr`, and also allows for this `not_null` to communicate
with more legacy APIs by propagating such pointers out efficiently:

```cpp
// Should never be null, but not yet refactored to be 'not_null'
auto old_api(std::unique_ptr<Widget> p) -> void;

auto new_api(cpp::not_null<std::unique_ptr<Widget>> p) -> void
{
  // Extract the move-only unique_ptr, and push along to 'old_api'
  old_api(std::move(p).as_nullable());
}
```

## Optional Features

The defaults for this library should satisfy most consumers needs; however
**not_null** supports two optional features that may be controlled
through preprocessor symbols:

1. Using a custom namespace, and
2. Disabling all exceptions

### Using a Custom Namespace

The `namespace` that `not_null` is defined in is configurable. By default,
it is defined in `namespace cpp`; however this can be toggled by defining
the preprocessor token `NOT_NULL_NAMESPACE` to be the name of the desired
namespace.

This could be done either through a `#define` preprocessor directive prior to
inclusion:

```cpp
#define NOT_NULL_NAMESPACE example
#include <not_null.hpp>

auto test(example::not_null<int*> p) -> void;
```

<kbd>[Try Online](https://godbolt.org/z/1nd8Wj)</kbd>

Or it could also be defined using a compiler flag with `-D` or `/D`, such
as:

`g++ -std=c++11 -DNOT_NULL_NAMESPACE=example test.cpp`

```cpp
#include <not_null.hpp>

auto test(example::not_null<int*> p) -> void;
```

<kbd>[Try Online](https://godbolt.org/z/x6rjej)</kbd>

### Disabling Exceptions

By default, `not_null` will throw a `cpp::not_null_contract_violation` when
the contract has been violated (during factory construction).

However, it is realized that not all developers appreciate or use exceptions,
and that environments that disable exception handling specifically may
error due to the existence of a `throw` expression. To account for this
possibility, exceptions may be disabled through the introduction of a simple
macro preprocessor symbol: `NOT_NULL_DISABLE_EXCEPTIONS`.

In doing so, contract-violations will now behave differently:

* Contract violations will call `std::abort`, causing immediate termination
  (and often, core-dumps for diagnostic purposes)
* Contract violations will print directly to `stderr` to allow context for the
  termination
* Since exceptions are disabled, there is no way to perform a proper stack
  unwinding -- so destructors will _not be run_. There is simply no way to
  allow for proper RAII cleanup without exceptions in this case.

<kbd>[Try Online](https://godbolt.org/z/5na3KW)</kbd>


## Compiler Compatibility

**not_null** is compatible with any compiler capable of compiling valid
<kbd>C++11</kbd>. Specifically, this has been tested and is known to work
with:

* GCC 5, 6, 7, 8, 9, 10
* Clang 3.5, 3.6, 3.7, 3.8, 3.9, 4, 5, 6, 7, 8, 9, 10, 11
* Apple Clang (Xcode) 10.3, 11.2, 11.3, 12.3
* Visual Studio 2015, 2017, 2019

Latest patch level releases are assumed in the versions listed above.

## License

<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

**not_null** is licensed under the
[MIT License](http://opensource.org/licenses/MIT):

> Copyright &copy; 2019 Matthew Rodusek
>
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.

## References

* [`alloy::core::not_null`](https://github.com/bitwizeshift/Alloy/blob/78abb03feff35ead86888f445d48acbe72ada8c5/lib/alloy-core/include/alloy/core/utilities/not_null.hpp):
  the original implementation that this utility is based on.
* [`gsl::not_null`](https://raw.githubusercontent.com/microsoft/GSL/master/include/gsl/pointers):
  a competing implementation offered as part of the core-guidelines support
  library
* [`doxygen::oxygen::nn`](https://raw.githubusercontent.com/dropbox/nn/master/nn.hpp):
  an alternative to nullability offered by Dropbox (archived)
