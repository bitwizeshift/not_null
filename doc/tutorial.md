# Tutorial

`not_null` is a small, simple utility to use.

1. [Construction](#construction)
    1. [With checking](#with-checking)
    2. [Without checking](#without-checking)
3. [Assigning to a `not_null`](#assigning-to-a-not_null)
4. [Extracting out the nullable pointer](#extracting-out-the-nullable-pointer)
5. [Comparing with other pointers](#comparing-with-other-pointers)

## Construction

`not_null` does not offer constructors from the underlying pointer type.
This decision is deliberate, since offering such constructors mask where runtime
checks are occurring, and may lead to less obvious points-of-failure on contract
violation (such as when incrmentally adopting `not_null` in an API).

To get around this, named _factory functions_ are offered instead that
explicitly lets the user opt-in for runtime checks, or do the checking
themselves and guarantee that the input is non null. These factories functions
are:

* `check_not_null`, and
* `assume_not_null`

### With checking

To construct a `not_null` from a pointer that you _expect_ to be null, but
want to verify this first, you can use the `check_not_null` factory function:

```cpp
auto test(int* p) -> void
{
  // Check that 'p' does not contain a null first
  auto nn = cpp::check_not_null(p);

  ...
}
```

### Without checking

If you are dealing with an object that is already known not to contain a null,
and wish to avoid the checking -- such would be the case if the pointer is the
address of a value object, or if a pointer was already checked before -- this
can be done with the `assume_not_null` factory function:

```cpp
auto test(int* p) -> void
{
  if (!p) { return; }

  ...

  // We already checked above, so assume this is not null
  auto nn = cpp::assume_not_null(p);

  // addresses can never be null
  int x = 42;
  auto nn2 = cpp::assume_not_null(&x);

  // newly allocated pointers from `new`, `make_shared`, etc., are never null
  // by default.
  auto nn3 = cpp::assume_not_null(std::make_shared<Widget>());
}
```

**Note:** that if the assumption is _not_ valid, and the pointer _is_ actually
null, then this may result in **undefined behavior**. `assume_not_null` is only
meant for cases where there is no fundamental way for a pointer to take on a
null value; and exists only to avoid the overhead of an additional check. In
general, `check_not_null` should be the preferred API.

## Assigning to a `not_null`

Like the constructors, `not_null` is not directly constructible from the
underlying pointers. The only way to assign to a `not_null` is to assign
an existing `not_null` to it. Like the constructors, this means that you must
use either of the factory functions to construct the `not_null` first.

```cpp
// Create a not_null<unique_ptr<int>>;
auto nn = cpp::assume_not_null(std::make_unique<int>(42));

// Assign with a known non-null pointer
nn = cpp::assume_not_null(std::make_unique<int>(100));

// Assign with a checked non-null pointer
nn = cpp::check_not_null(std::move(p));
```

## Extracting out the nullable pointer

When using a `not_null` wrapper, one common requirement is that it must be
able to operate with legacy APIs to allow for incremental adoption. To do this,
the underlying pointer of the `not_null` must be extractable. This is possible
with the `as_nullable()` member function:

```cpp
auto legacy_api(std::shared_ptr<Widget> p) -> void;

auto new_api(cpp::not_null<std::shared_ptr<Widget>> nn) -> void
{
  // Extract out the shared_ptr, pass to legacy_api
  legacy_api(nn.as_nullable());
}
```

This even works with move-only types, although an explicit `std::move` is
required in order to indicate that the type is ready to transfer ownership:

```cpp
auto legacy_api(std::unique_ptr<Widget> p) -> void;

auto new_api(cpp::not_null<std::unique_ptr<Widget>> nn) -> void
{
  // Extract out the unique_ptr and move it to legacy_api
  legacy_api(std::move(nn).as_nullable());

  // After this call, 'nn' is in a 'moved-from' state.
}
```

-------------------------------------------------------------------------------

**Note:** Using the rvalue-qualified `as_nullable()` function will leave the
`not_null` in a "moved-from" state. Like any C++ standard type, a moved-from
state can only be used validly in select circumstances where any operations do
not have preconditions. In particular, observer and element accessing functions
will no longer operate correctly; though assignment will place the object back
into a valid state:

```cpp
auto nn = cpp::assume_not_null( std::make_unique<Widget>() );

// Take the unique_ptr out, leave 'nn' in a moved-from state
auto p = std::move(nn).as_nullable();

// Code like `if (nn)`, `nn.get()`, etc will no longer be valid

...

// Assign a new widget
nn = cpp::assume_not_null( std::make_unique<Widget>() );

// `if (nn)`, `nn.get()`, etc will behave correctly again
```

In general, static analyzers should detect and flag use-after-move as a logic
error, and so reusing such a pointer should be avoided.

## Comparing with other pointers

`not_null` is capable of performing any comparison that the underlying pointer
it wraps is capable of performing. This includes comparisons with `nullptr`,
although such comparisons are trivially defined to always assume that the
`not_null` never contains a pointer to `nullptr`.

```cpp
auto nn1 = cpp::assume_not_null( new int{} );
auto nn2 = cpp::assume_not_null( new int{} );
auto p1 = nn.get();

if (nn1 == p1) { ... } // valid check
if (nn1 == nullptr) { /* this branch can never be hit; not_null is never null */ }
if (nn1 == nn2) { ... } // valid check
```
