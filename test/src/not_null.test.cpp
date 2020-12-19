/*
  The MIT License (MIT)

  Copyright (c) 2020 Matthew Rodusek All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "not_null.hpp"

#include <catch2/catch.hpp>

namespace NOT_NULL_NAMESPACE_INTERNAL {
inline namespace bitwizeshift {

//=============================================================================
// class : not_null
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

TEST_CASE("not_null<T>::not_null(const not_null&)", "[ctor]") {
  SECTION("T is copy constructible") {
    SECTION("Copies the underlying pointer ") {
      const auto input = std::make_shared<int>(42);

      const auto sut = assume_not_null(input);
      const auto copy = sut;

      REQUIRE(copy.as_nullable() == sut.as_nullable());
    }
    SECTION("not_null is copy constructible") {
      using sut_type = not_null<std::shared_ptr<int>>;

      STATIC_REQUIRE(std::is_copy_constructible<sut_type>::value);
    }
  }
  SECTION("T is not copy constructible") {
    SECTION("not_null is not copy constructible") {
      using sut_type = not_null<std::unique_ptr<int>>;

      STATIC_REQUIRE_FALSE(std::is_copy_constructible<sut_type>::value);
    }
  }
  SECTION("T is trivially copy constructible") {
    SECTION("not_null<T> is trivially copy constructible") {
      STATIC_REQUIRE(std::is_trivially_copy_constructible<not_null<int*>>::value);
    }
  }
}

TEST_CASE("not_null<T>::not_null(not_null&&)", "[ctor]") {
  SECTION("T is move constructible") {
    SECTION("Moves the underlying pointer ") {
      auto input = std::unique_ptr<int>{
        new int{42}
      };
      auto* ptr = input.get();

      auto sut = assume_not_null(std::move(ptr));
      const auto result = std::move(sut);

      REQUIRE(result.as_nullable() == ptr);
    }
    SECTION("not_null is move constructible") {
      using sut_type = not_null<std::unique_ptr<int>>;

      STATIC_REQUIRE(std::is_move_constructible<sut_type>::value);
    }
  }
  SECTION("T is trivially move constructible") {
    SECTION("not_null<T> is trivially move constructible") {
      STATIC_REQUIRE(std::is_trivially_move_constructible<not_null<int*>>::value);
    }
  }
}

TEST_CASE("not_null<T>::not_null(const not_null<U>&) (implicit)", "[ctor]") {
  SECTION("U is convertible to T") {
    SECTION("not_null<U> is convertible to not_null<T>") {
      using input_type = not_null<std::shared_ptr<int>>;
      using sut_type = not_null<std::shared_ptr<void>>;

      STATIC_REQUIRE(std::is_convertible<const input_type&,sut_type>::value);
    }
    SECTION("Produces conversion-constructed not_null") {
      auto input = assume_not_null(
          std::make_shared<int>(42)
      );
      not_null<std::shared_ptr<void>> sut = input;

      REQUIRE(sut.as_nullable() == input.as_nullable());
    }
  }
}

TEST_CASE("not_null<T>::not_null(const not_null<U>&) (explicit)", "[ctor]") {
  SECTION("U is constructible from T") {
    SECTION("not_null<U> is constructible from not_null<T>") {
      using input_type = not_null<int*>;
      using sut_type = not_null<std::shared_ptr<int>>;

      STATIC_REQUIRE(std::is_constructible<sut_type,const input_type&>::value);
    }
    SECTION("Produces conversion-constructed not_null") {
      auto input = assume_not_null(
          new int{42}
      );
      not_null<std::shared_ptr<int>> sut{input};

      REQUIRE(sut.as_nullable().get() == input.as_nullable());
    }
  }
}

TEST_CASE("not_null<T>::not_null(not_null<U>&&) (implicit)", "[ctor]") {
  SECTION("U is convertible to T") {
    SECTION("not_null<U> is convertible to not_null<T>") {
      using input_type = not_null<std::unique_ptr<int>>;
      using sut_type = not_null<std::shared_ptr<int>>;

      STATIC_REQUIRE(std::is_convertible<input_type&&,sut_type>::value);
    }
    SECTION("Produces conversion-constructed not_null") {
      auto input = assume_not_null(
        std::unique_ptr<int>{new int{42}}
      );
      const auto* copy = input.get();
      not_null<std::shared_ptr<void>> sut = std::move(input);

      REQUIRE(sut.as_nullable().get() == copy);
    }
  }
}

TEST_CASE("not_null<T>::not_null(not_null<U>&&) (explicit)", "[ctor]") {
  SECTION("U is convertible to T") {
    SECTION("not_null<U> is convertible to not_null<T>") {
      using input_type = not_null<int*>;
      using sut_type = not_null<std::shared_ptr<int>>;

      STATIC_REQUIRE(std::is_constructible<sut_type,input_type&&>::value);
    }
    SECTION("Produces conversion-constructed not_null") {
      auto input = assume_not_null(new int{42});
      const auto* copy = input.get();
      not_null<std::shared_ptr<void>> sut{std::move(input)};

      REQUIRE(sut.as_nullable().get() == copy);
    }
  }
}

//-----------------------------------------------------------------------------

TEST_CASE("not_null<T>::~not_null()", "[dtor]") {
  SECTION("T is trivially destructible") {
    SECTION("not_null<T> is trivially destructible") {
      STATIC_REQUIRE(std::is_trivially_destructible<not_null<int*>>::value);
    }
  }
  SECTION("T is not trivially destructible") {
    SECTION("not_null<T> is not trivially destructible") {
      STATIC_REQUIRE_FALSE(std::is_trivially_destructible<not_null<std::shared_ptr<int>>>::value);
    }
  }
}

//-----------------------------------------------------------------------------

TEST_CASE("not_null<T>::operator=(const not_null&)", "[assign]") {
  SECTION("T is copy assignable") {
    SECTION("Copies the underlying pointer ") {
      const auto input = std::make_shared<int>(42);
      const auto value = std::make_shared<int>();

      auto sut = assume_not_null(input);
      sut = assume_not_null(value);

      REQUIRE(sut.as_nullable() == value);
    }
    SECTION("not_null is copy constructible") {
      using sut_type = not_null<std::shared_ptr<int>>;

      STATIC_REQUIRE(std::is_copy_assignable<sut_type>::value);
    }
  }
  SECTION("T is not copy assignable") {
    SECTION("not_null is not copy assignable") {
      using sut_type = not_null<std::unique_ptr<int>>;

      STATIC_REQUIRE_FALSE(std::is_copy_assignable<sut_type>::value);
    }
  }
  SECTION("T is trivially copy assignable") {
    SECTION("not_null<T> is trivially copy assignable") {
      STATIC_REQUIRE(std::is_trivially_copy_assignable<not_null<int*>>::value);
    }
  }
}

TEST_CASE("not_null<T>::operator=(not_null&&)", "[assign]") {
  SECTION("T is move assignable") {
    SECTION("Moves the underlying pointer ") {
      auto sut = assume_not_null(
        std::unique_ptr<int>{new int{42}}
      );
      auto input = std::unique_ptr<int>{new int{0}};
      const auto* ptr = input.get();

      sut = assume_not_null(std::move(input));

      REQUIRE(sut.get() == ptr);
    }
    SECTION("not_null is move assignable") {
      using sut_type = not_null<std::unique_ptr<int>>;

      STATIC_REQUIRE(std::is_move_assignable<sut_type>::value);
    }
  }
  SECTION("T is trivially move assignable") {
    SECTION("not_null<T> is trivially move assignable") {
      STATIC_REQUIRE(std::is_trivially_move_assignable<not_null<int*>>::value);
    }
  }
}

TEST_CASE("not_null<T>::operator=(const not_null<U>&)", "[assign]") {
  SECTION("not_null<U> is assignable to not_null<T>") {
    using input_type = not_null<std::shared_ptr<int>>;
    using sut_type = not_null<std::shared_ptr<void>>;

    STATIC_REQUIRE(std::is_assignable<sut_type,const input_type&>::value);
  }
  SECTION("Produces conversion-constructed not_null") {
    auto sut = not_null<std::shared_ptr<void>>{
      assume_not_null(
        std::make_shared<int>(42)
      )
    };
    auto input = assume_not_null(
      std::make_shared<int>()
    );
    sut = input;

    REQUIRE(sut.as_nullable() == input.as_nullable());
  }
}

TEST_CASE("not_null<T>::operator=(not_null<U>&&)", "[assign]") {
  SECTION("not_null<U> is convertible to not_null<T>") {
    using input_type = not_null<std::unique_ptr<int>>;
    using sut_type = not_null<std::shared_ptr<int>>;

    STATIC_REQUIRE(std::is_assignable<sut_type,input_type&&>::value);
  }
  SECTION("Produces conversion-constructed not_null") {
    auto sut = assume_not_null(std::make_shared<int>(42));
    auto input = assume_not_null(
      std::unique_ptr<int>{new int{42}}
    );
    const auto* copy = input.get();

    sut = std::move(input);

    REQUIRE(sut.as_nullable().get() == copy);
  }
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

TEST_CASE("not_null<T>::get()", "[observers]") {
  const auto input = std::make_shared<int>(42);
  const auto sut = assume_not_null(input);

  SECTION("Gets underlying pointer") {
    REQUIRE(sut.get() == input.get());
  }
  SECTION("Return type is pointer") {
    STATIC_REQUIRE(std::is_pointer<decltype(sut.get())>::value);
  }
}

TEST_CASE("not_null<T>::operator bool()", "[observers]") {
  SECTION("Returns true") {
    const auto sut = assume_not_null(std::make_shared<int>(42));

    REQUIRE(static_cast<bool>(sut));
  }
}

TEST_CASE("not_null<T>::as_nullable() const &", "[observers]") {
  const auto input = std::make_shared<int>(42);
  const auto sut = assume_not_null(input);

  SECTION("Gets underlying pointer") {
    REQUIRE(sut.as_nullable() == input);
  }
  SECTION("Returns underlying type") {
    STATIC_REQUIRE(std::is_same<decltype(input)&,decltype(sut.as_nullable())>::value);
  }
}

TEST_CASE("not_null<T>::as_nullable() &&", "[observers]") {
  auto input = std::unique_ptr<int>(new int{42});
  auto* p = input.get();
  auto sut = assume_not_null(std::move(input));

  SECTION("Gets underlying pointer") {
    REQUIRE(sut.as_nullable().get() == p);
  }
  SECTION("Returns underlying type") {
    STATIC_REQUIRE(std::is_same<decltype(input)&&,decltype(std::move(sut).as_nullable())>::value);
  }
}

TEST_CASE("not_null<T>::operator->()", "[observers]") {
  SECTION("Accesses underlying pointer") {
    auto sut = assume_not_null(std::make_shared<int>(42));

    auto* expected = sut.get();

    REQUIRE(sut.operator->() == expected);
  }
}

TEST_CASE("not_null<T>::operator*()", "[observers]") {
  SECTION("Accesses underlying reference") {
    auto sut = assume_not_null(std::make_shared<int>(42));

    auto* expected = sut.get();

    REQUIRE(&sut.operator*() == expected);
  }
}

//=============================================================================
// non-member functions : class : not_null
//=============================================================================

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

TEST_CASE("assume_not_null(U&&)", "[utilities]") {
  SECTION("Input is not null") {
    SECTION("Produces a not_null without a null pointer") {
      const auto input = std::make_shared<int>(42);
      auto sut = assume_not_null(input);

      REQUIRE(sut.as_nullable() != nullptr);
    }
  }
}

TEST_CASE("check_not_null(U&&)", "[utilities]") {
  SECTION("Input is null") {
    SECTION("Throws null contract violation") {
      const auto* input = static_cast<int*>(nullptr);

      REQUIRE_THROWS_AS(check_not_null(input), not_null_contract_violation);
    }
  }
  SECTION("Input is not null") {
    SECTION("Does not throw contract violation") {
      const auto input = std::make_shared<int>(42);

      REQUIRE_NOTHROW(check_not_null(input));
    }
    SECTION("Produces a not_null without a null pointer") {
      const auto input = std::make_shared<int>(42);
      auto sut = check_not_null(input);

      REQUIRE(sut.as_nullable() != nullptr);
    }
  }
}

//-----------------------------------------------------------------------------
// Comparison
//-----------------------------------------------------------------------------

TEST_CASE("operator==(std::nullptr_t, const not_null<T>&)", "[comparison]") {
  SECTION("Returns false") {
    const auto sut = assume_not_null(std::make_shared<int>(42));

    REQUIRE_FALSE(nullptr == sut);
  }
}

TEST_CASE("operator==(const not_null<T>&, std::nullptr_t)", "[comparison]") {
  SECTION("Returns false") {
    const auto sut = assume_not_null(std::make_shared<int>(42));

    REQUIRE_FALSE(sut == nullptr);
  }
}

TEST_CASE("operator==(const not_null<T>&, const not_null<U>&)", "[comparison]") {
  SECTION("lhs compares equal to rhs") {
    const auto value = std::make_shared<int>(42);
    const auto lhs = assume_not_null(value);
    const auto rhs = assume_not_null(value);

    SECTION("Returns true") {
      REQUIRE(lhs == rhs);
    }
  }

  SECTION("lhs does not compare equal to rhs") {
    const auto lhs = assume_not_null(std::make_shared<int>(42));
    const auto rhs = assume_not_null(std::make_shared<int>(0));

    SECTION("Returns false") {
      REQUIRE_FALSE(lhs == rhs);
    }
  }
}

TEST_CASE("operator==(const not_null<T>&, const U&)", "[comparison]") {
  SECTION("lhs compares equal to rhs") {
    const auto value = std::make_shared<int>(42);
    const auto lhs = assume_not_null(value);
    const auto rhs = value;

    SECTION("Returns true") {
      REQUIRE(lhs == rhs);
    }
  }

  SECTION("lhs does not compare equal to rhs") {
    const auto lhs = assume_not_null(std::make_shared<int>(42));
    const auto rhs = std::make_shared<int>(0);

    SECTION("Returns false") {
      REQUIRE_FALSE(lhs == rhs);
    }
  }
}

TEST_CASE("operator==(const T&, const not_null<U>&)", "[comparison]") {
  SECTION("lhs compares equal to rhs") {
    const auto value = std::make_shared<int>(42);
    const auto lhs = value;
    const auto rhs = assume_not_null(value);

    SECTION("Returns true") {
      REQUIRE(lhs == rhs);
    }
  }

  SECTION("lhs does not compare equal to rhs") {
    const auto lhs = std::make_shared<int>(42);
    const auto rhs = assume_not_null(std::make_shared<int>(0));

    SECTION("Returns false") {
      REQUIRE_FALSE(lhs == rhs);
    }
  }
}

//-----------------------------------------------------------------------------

TEST_CASE("operator!=(std::nullptr_t, const not_null<T>&)", "[comparison]") {
  SECTION("Returns true") {
    const auto sut = assume_not_null(std::make_shared<int>(42));

    REQUIRE(nullptr != sut);
  }
}

TEST_CASE("operator!=(const not_null<T>&, std::nullptr_t)", "[comparison]") {
  SECTION("Returns true") {
    const auto sut = assume_not_null(std::make_shared<int>(42));

    REQUIRE(sut != nullptr);
  }
}

TEST_CASE("operator!=(const not_null<T>&, const not_null<U>&)", "[comparison]") {
  SECTION("lhs compares equal to rhs") {
    const auto value = std::make_shared<int>(42);
    const auto lhs = assume_not_null(value);
    const auto rhs = assume_not_null(value);

    SECTION("Returns false") {
      REQUIRE_FALSE(lhs != rhs);
    }
  }

  SECTION("lhs does not compare equal to rhs") {
    const auto lhs = assume_not_null(std::make_shared<int>(42));
    const auto rhs = assume_not_null(std::make_shared<int>(0));

    SECTION("Returns true") {
      REQUIRE(lhs != rhs);
    }
  }
}

TEST_CASE("operator!=(const not_null<T>&, const U&)", "[comparison]") {
  SECTION("lhs compares equal to rhs") {
    const auto value = std::make_shared<int>(42);
    const auto lhs = assume_not_null(value);
    const auto rhs = value;

    SECTION("Returns false") {
      REQUIRE_FALSE(lhs != rhs);
    }
  }

  SECTION("lhs does not compare equal to rhs") {
    const auto lhs = assume_not_null(std::make_shared<int>(42));
    const auto rhs = std::make_shared<int>(0);

    SECTION("Returns true") {
      REQUIRE(lhs != rhs);
    }
  }
}

TEST_CASE("operator!=(const T&, const not_null<U>&)", "[comparison]") {
  SECTION("lhs compares equal to rhs") {
    const auto value = std::make_shared<int>(42);
    const auto lhs = value;
    const auto rhs = assume_not_null(value);

    SECTION("Returns false") {
      REQUIRE_FALSE(lhs != rhs);
    }
  }

  SECTION("lhs does not compare equal to rhs") {
    const auto lhs = std::make_shared<int>(42);
    const auto rhs = assume_not_null(std::make_shared<int>(0));

    SECTION("Returns true") {
      REQUIRE(lhs != rhs);
    }
  }
}

//-----------------------------------------------------------------------------

TEST_CASE("operator<(const not_null<T>& lhs, const not_null<U>& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is less than rhs") {
    const auto lhs = assume_not_null(&a[0]);
    const auto rhs = assume_not_null(&a[1]);

    SECTION("Returns true") {
      REQUIRE(lhs < rhs);
    }
  }
  SECTION("lhs is not less than rhs") {
    const auto lhs = assume_not_null(&a[1]);
    const auto rhs = assume_not_null(&a[0]);

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs < rhs);
    }
  }
}

TEST_CASE("operator<(const not_null<T>& lhs, const U& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is less than rhs") {
    const auto lhs = assume_not_null(&a[0]);
    const auto rhs = &a[1];

    SECTION("Returns true") {
      REQUIRE(lhs < rhs);
    }
  }
  SECTION("lhs is not less than rhs") {
    const auto lhs = assume_not_null(&a[1]);
    const auto rhs = &a[0];

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs < rhs);
    }
  }
}

TEST_CASE("operator<(const T& lhs, const not_null<U>& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is less than rhs") {
    const auto lhs = &a[0];
    const auto rhs = assume_not_null(&a[1]);

    SECTION("Returns true") {
      REQUIRE(lhs < rhs);
    }
  }
  SECTION("lhs is not less than rhs") {
    const auto lhs = &a[1];
    const auto rhs = assume_not_null(&a[0]);

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs < rhs);
    }
  }
}

//-----------------------------------------------------------------------------

TEST_CASE("operator>(const not_null<T>& lhs, const not_null<U>& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is greater than rhs") {
    const auto lhs = assume_not_null(&a[1]);
    const auto rhs = assume_not_null(&a[0]);

    SECTION("Returns true") {
      REQUIRE(lhs > rhs);
    }
  }
  SECTION("lhs is not less than rhs") {
    const auto lhs = assume_not_null(&a[0]);
    const auto rhs = assume_not_null(&a[1]);

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs > rhs);
    }
  }
}

TEST_CASE("operator>(const not_null<T>& lhs, const U& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is greater than rhs") {
    const auto lhs = assume_not_null(&a[1]);
    const auto rhs = &a[0];

    SECTION("Returns true") {
      REQUIRE(lhs > rhs);
    }
  }
  SECTION("lhs is not greater than rhs") {
    const auto lhs = assume_not_null(&a[0]);
    const auto rhs = &a[1];

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs > rhs);
    }
  }
}

TEST_CASE("operator>(const T& lhs, const not_null<U>& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is greater than rhs") {
    const auto lhs = &a[1];
    const auto rhs = assume_not_null(&a[0]);

    SECTION("Returns true") {
      REQUIRE(lhs > rhs);
    }
  }
  SECTION("lhs is not greater than rhs") {
    const auto lhs = &a[0];
    const auto rhs = assume_not_null(&a[1]);

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs > rhs);
    }
  }
}

//-----------------------------------------------------------------------------

TEST_CASE("operator<=(const not_null<T>& lhs, const not_null<U>& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is less than rhs") {
    const auto lhs = assume_not_null(&a[0]);
    const auto rhs = assume_not_null(&a[1]);

    SECTION("Returns true") {
      REQUIRE(lhs <= rhs);
    }
  }
  SECTION("lhs is not less than rhs") {
    const auto lhs = assume_not_null(&a[1]);
    const auto rhs = assume_not_null(&a[0]);

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs <= rhs);
    }
  }
}

TEST_CASE("operator<=(const not_null<T>& lhs, const U& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is less than rhs") {
    const auto lhs = assume_not_null(&a[0]);
    const auto rhs = &a[1];

    SECTION("Returns true") {
      REQUIRE(lhs <= rhs);
    }
  }
  SECTION("lhs is not less than rhs") {
    const auto lhs = assume_not_null(&a[1]);
    const auto rhs = &a[0];

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs <= rhs);
    }
  }
}

TEST_CASE("operator<=(const T& lhs, const not_null<U>& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is less than rhs") {
    const auto lhs = &a[0];
    const auto rhs = assume_not_null(&a[1]);

    SECTION("Returns true") {
      REQUIRE(lhs <= rhs);
    }
  }
  SECTION("lhs is not less than rhs") {
    const auto lhs = &a[1];
    const auto rhs = assume_not_null(&a[0]);

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs <= rhs);
    }
  }
}

//-----------------------------------------------------------------------------

TEST_CASE("operator>=(const not_null<T>& lhs, const not_null<U>& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is greater than rhs") {
    const auto lhs = assume_not_null(&a[1]);
    const auto rhs = assume_not_null(&a[0]);

    SECTION("Returns true") {
      REQUIRE(lhs >= rhs);
    }
  }
  SECTION("lhs is not less than rhs") {
    const auto lhs = assume_not_null(&a[0]);
    const auto rhs = assume_not_null(&a[1]);

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs >= rhs);
    }
  }
}

TEST_CASE("operator>=(const not_null<T>& lhs, const U& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is greater than rhs") {
    const auto lhs = assume_not_null(&a[1]);
    const auto rhs = &a[0];

    SECTION("Returns true") {
      REQUIRE(lhs >= rhs);
    }
  }
  SECTION("lhs is not greater than rhs") {
    const auto lhs = assume_not_null(&a[0]);
    const auto rhs = &a[1];

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs >= rhs);
    }
  }
}

TEST_CASE("operator>=(const T& lhs, const not_null<U>& rhs)", "[comparison]") {
  int a[2] {};

  SECTION("lhs is greater than rhs") {
    const auto lhs = &a[1];
    const auto rhs = assume_not_null(&a[0]);

    SECTION("Returns true") {
      REQUIRE(lhs >= rhs);
    }
  }
  SECTION("lhs is not greater than rhs") {
    const auto lhs = &a[0];
    const auto rhs = assume_not_null(&a[1]);

    SECTION("Returns true") {
      REQUIRE_FALSE(lhs >= rhs);
    }
  }
}

} // inline namespace bitwizeshift
} // namespace NOT_NULL_NAMESPACE_INTERNAL
