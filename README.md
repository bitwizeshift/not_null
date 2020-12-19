# not_null

**not_null** is a **0-overhead**, modern utility for ensuring non-nullability
in a coherent way.

Unlike `gsl::not_null`, this type can work with move-only pointers like 
`std::unique_ptr`, and does not require runtime checks unless explicitly 
specified.

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

<kbd>[Live Example](#todo)</kbd>

## Features

* [x] Zero-Overhead; `not_null` has no hidden additional runtime 
      or storage overhead.
* [x] Written completely in <kbd>C++11</kbd>
* [x] Single-header, **header-only** solution -- easily drops into any project
* [x] No dependencies

## Documentation

* [Background](#background) \
  A background on the problem **not_null** solves
* [Installation](doc/installing.md) \
  For a quick guide on how to install/use this in other projects
* [API Reference](https://bitwizeshift.github.io/not_null/api/latest/) \
  For doxygen-generated API information
* [Attribution](doc/legal.md) \
  Information about how to attribute this project

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
