/*****************************************************************************
 * \file not_null.hpp
 *
 * \brief This header defines a utility for asserting semantic correctness
 *        of non-nullable pointer types
 *****************************************************************************/

/*
  The MIT License (MIT)

  Copyright (c) 2019 Matthew Rodusek All rights reserved.

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
#ifndef CPP_BITWIZESHIFT_NOT_NULL_HPP
#define CPP_BITWIZESHIFT_NOT_NULL_HPP

#include <cstddef>     // std::nullptr_t
#include <utility>     // std::forward, std::move
#include <type_traits> // std::decay_t
#include <memory>      // std::pointer_traits
#if !defined(NOT_NULL_DISABLE_EXCEPTIONS)
# include <stdexcept> // std::logic_error
#else
# include <cstdio>  // std::printf
# include <cstdlib> // std::abort
#endif

#if __cplusplus >= 201402L
# define NOT_NULL_CPP14_CONSTEXPR constexpr
#else
# define NOT_NULL_CPP14_CONSTEXPR
#endif

#if defined(__clang__) || defined(__GNUC__)
# define NOT_NULL_INLINE_VISIBILITY __attribute__((visibility("hidden"), always_inline))
#elif defined(_MSC_VER)
# define NOT_NULL_INLINE_VISIBILITY __forceinline
#else
# define NOT_NULL_INLINE_VISIBILITY
#endif

#if defined(NOT_NULL_NAMESPACE)
# define NOT_NULL_NAMESPACE_INTERNAL NOT_NULL_NAMESPACE
#else
# define NOT_NULL_NAMESPACE_INTERNAL cpp
#endif
#define NOT_NULL_NS_IMPL NOT_NULL_NAMESPACE_INTERNAL::bitwizeshift

namespace NOT_NULL_NAMESPACE_INTERNAL {
inline namespace bitwizeshift {

  template <typename T>
  class not_null;

  //===========================================================================
  // trait : is_not_null
  //===========================================================================

  template <typename T>
  struct is_not_null : std::false_type{};
  template <typename T>
  struct is_not_null<not_null<T>> : std::true_type{};

#if !defined(NOT_NULL_DISABLE_EXCEPTIONS)

  //===========================================================================
  // class : null_pointer_error
  //===========================================================================

  /////////////////////////////////////////////////////////////////////////////
  /// \brief An exception thrown on null contract violations as part of
  ///        check_not_null
  /////////////////////////////////////////////////////////////////////////////
  class not_null_contract_violation : public std::logic_error
  {
    using this_type = not_null_contract_violation;
  public:

    not_null_contract_violation();
    not_null_contract_violation(const this_type& other) = default;
    not_null_contract_violation(this_type&& other) = default;

    auto operator=(const this_type& other) -> this_type& = default;
    auto operator=(this_type&& other) -> this_type& = default;
  };

#endif


  //===========================================================================
  // utilities : constexpr forward
  //===========================================================================

  // std::forward is not constexpr until C++14
  namespace detail {
    template <typename T>
    inline NOT_NULL_INLINE_VISIBILITY constexpr
    auto not_null_forward(typename std::remove_reference<T>::type& t)
      noexcept -> T&&
    {
      return static_cast<T&&>(t);
    }

    template <typename T>
    inline NOT_NULL_INLINE_VISIBILITY constexpr
    auto not_null_forward(typename std::remove_reference<T>::type&& t)
      noexcept -> T&&
    {
      return static_cast<T&&>(t);
    }
  } // namespace detail

  //===========================================================================
  // detail utilities : not_null
  //===========================================================================

  // std::forward is not constexpr until C++14
  namespace detail {
    /// \{
    /// \brief Recursively calls `operator->()` on `p` until a raw pointer is
    ///        retrieved
    ///
    /// \param p the pointer-like type
    /// \return the underlying raw pointer
    template <typename U>
    static constexpr auto not_null_to_address(U* p)
      noexcept -> U*;
    template <typename Ptr>
    static constexpr auto not_null_to_address(const Ptr& p)
      noexcept -> decltype(::NOT_NULL_NS_IMPL::detail::not_null_to_address(p.operator->()));
    /// \}

    /// \brief Throws a not_null_contract_violation in exception mode
    [[noreturn]] NOT_NULL_CPP14_CONSTEXPR auto throw_null_pointer_error() -> void;

    template <typename T, typename U>
    struct not_null_is_explicit_convertible : std::integral_constant<bool,(
      std::is_constructible<T,U>::value &&
      !std::is_convertible<U,T>::value
    )>{};

    template <typename T, typename U>
    struct not_null_is_implicit_convertible : std::integral_constant<bool,(
      std::is_constructible<T,U>::value &&
      std::is_convertible<U,T>::value
    )>{};

  } // namespace detail

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A wrapper type around a pointer to disallow null assignments
  ///
  /// This type can only be used with pointers that may be understood by
  /// std::pointer_traits. This requires a type to either define 'element_type'
  /// or specialize pointer_traits for their respective needs.
  ///
  /// This type is a type-wrapper, so that APIs can semantically indicate their
  /// nullability requirement in a concise and coherent way.
  ///
  /// ### Examples
  ///
  /// Basic Usage:
  ///
  /// ```cpp
  /// auto post(not_null<std::unique_ptr<Task>> task) -> void
  /// {
  ///   // pass to internal API that uses 'unique_ptr' directly
  ///   post_internal(std::move(task).as_nullable());
  /// }
  ///
  /// ...
  ///
  /// post(assume_not_null(std::move(p)));
  /// ````
  ///
  /// \tparam T the underlying pointer type
  /////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class not_null
  {
    static_assert(
      !std::is_reference<T>::value,
      "not_null<T&> is ill-formed. "
      "not_null may only work with non-CV qualified value types."
    );
    static_assert(
      !std::is_void<T>::value,
      "not_null<void> is ill-formed. "
      "not_null may only work with non-CV qualified value types."
    );
    static_assert(
      std::is_assignable<T&, std::nullptr_t>::value &&
      std::is_constructible<T, std::nullptr_t>::value,
      "T must satisfy NullablePointer, otherwise not_null<T> is ill-formed."
    );
    static_assert(
      !std::is_same<T,std::nullptr_t>::value,
      "not_null<std::nullptr_t> is ill-formed. I mean, really, come on."
    );
    static_assert(
      !is_not_null<T>::value,
      "not_null<not_null<T>> is ill-formed."
    );
    static_assert(
      !std::is_const<T>::value && !std::is_volatile<T>::value,
      "not_null<[const] [volatile] T> is ill-formed. "
      "not_null may only work with non-CV qualified value types."
    );

    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using element_type = typename std::pointer_traits<T>::element_type;
    using pointer      = element_type*;
    using reference    = typename std::add_lvalue_reference<element_type>::type;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    // not_null is not default-constructible, since this would result in a null
    // value
    not_null() = delete;

    /// \{
    /// \brief Constructs a not_null by converting a not_null from a different
    ///        underlying source
    ///
    /// This constructor allows for conversions between different underlying
    /// pointer types
    ///
    /// \note This constructor only participates in overload resolution if
    ///       `std::is_constructible<T,const U&>::value` is `true`
    ///
    /// \note This constructor is explicit if and only if
    ///       `std::is_convertible<const U&, T>::value` is `false`
    ///
    /// ### Examples
    ///
    /// Implicit use:
    ///
    /// ```cpp
    /// auto p = assume_not_null(
    ///     std::make_shared<Derived>(42)
    /// );
    /// not_null<std::shared_ptr<Base>> q = p;
    ///
    /// assert(p == q);
    /// ```
    ///
    /// Explicit Use:
    ///
    /// ```cpp
    /// // Uses 'p' to construct the underlying unique_ptr
    /// auto p = assume_not_null(
    ///     new int{42}
    /// );
    /// auto q = not_null<std::unique_ptr<int>>{p};
    ///
    /// assert(p == q);
    /// ```
    ///
    /// \param other the other not_null to convert
    template <typename U,
              typename std::enable_if<detail::not_null_is_implicit_convertible<T,const U&>::value,int>::type = 0>
    not_null(const not_null<U>& other)
      noexcept(std::is_nothrow_constructible<T,const U&>::value);
    template <typename U,
              typename std::enable_if<detail::not_null_is_explicit_convertible<T,const U&>::value,int>::type = 0>
    explicit not_null(const not_null<U>& other)
      noexcept(std::is_nothrow_constructible<T,const U&>::value);
    /// \}

    /// \{
    /// \brief Constructs a not_null by converting a not_null from a different
    ///        underlying source
    ///
    /// This constructor allows for conversions between different underlying
    /// pointer types
    ///
    /// \note This constructor only participates in overload resolution if
    ///       `std::is_constructible<T,U&&>::value` is `true`
    ///
    /// \note This constructor is explicit if and only if
    ///       `std::is_convertible<U&&, T>::value` is `false`
    ///
    /// ### Examples
    ///
    /// Implicit Use:
    ///
    /// ```cpp
    /// auto p = assume_not_null(
    ///     std::make_shared<Derived>(42)
    /// );
    /// not_null<std::shared_ptr<Base>> q = std::move(p);
    /// ```
    ///
    /// Explicit Use:
    ///
    /// ```cpp
    /// auto p = assume_not_null(
    ///     std::make_unique<int>(42)
    /// );
    /// auto q = not_null<std::shared_ptr<int>>{
    ///     std::move(p)
    /// };
    /// ```
    ///
    /// \param other the other not_null to convert
    template <typename U,
              typename std::enable_if<detail::not_null_is_implicit_convertible<T,U&&>::value,int>::type = 0>
    not_null(not_null<U>&& other)
      noexcept(std::is_nothrow_constructible<T,U&&>::value);
    template <typename U,
              typename std::enable_if<detail::not_null_is_explicit_convertible<T,U&&>::value,int>::type = 0>
    explicit not_null(not_null<U>&& other)
      noexcept(std::is_nothrow_constructible<T,U&&>::value);
    /// \}

    /// \brief Constructs this not_null by copying the contents of \p other
    ///
    /// \note This constructor does not paticipate in overload resolution
    ///       unless `std::is_copy_constructible<T>::value` is `true`
    ///
    /// \note This constructor is trivial if the underlying type `T`'s
    ///       copy constructor is trivial.
    ///
    /// \param other the other not_null to copy from
    not_null(const not_null& other) = default;

    /// \brief Constructs this not_null by moving the contents of \p other
    ///
    /// \note This constructor does not paticipate in overload resolution
    ///       unless `std::is_move_constructible<T>::value` is `true`
    ///
    /// \note This constructor is trivial if the underlying type `T`'s
    ///       copy constructor is trivial.
    ///
    /// \param other the other not_null to move from
    not_null(not_null&& other) = default;

    //-------------------------------------------------------------------------

    template <typename U,
              typename = typename std::enable_if<std::is_assignable<T&,const U&>::value>::type>
    NOT_NULL_CPP14_CONSTEXPR
    auto operator=(const not_null<U>& other)
      noexcept(std::is_nothrow_assignable<T&,const U&>::value) -> not_null&;

    template <typename U,
              typename = typename std::enable_if<std::is_assignable<T&,U&&>::value>::type>
    NOT_NULL_CPP14_CONSTEXPR
    auto operator=(not_null<U>&& other)
      noexcept(std::is_nothrow_assignable<T&,U&&>::value) -> not_null&;

    auto operator=(const not_null& other) -> not_null& = default;

    auto operator=(not_null&& other) -> not_null& = default;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the underlying pointer
    ///
    /// \return the underlying pointer
    constexpr auto get() const noexcept -> pointer;

    /// \brief Contextually convertible to bool
    ///
    /// This is always true
    constexpr explicit operator bool() const noexcept;

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Extracts the underlying nullable pointer from not_null
    ///
    /// \note The r-value overload allows for stealing the underlying nullable
    ///       pointer. In doing so, the invariant of this class may
    ///       intentionally be violated (subject to the underlying pointer).
    ///       Care needs to be taken to avoid potentially reusing this not_null
    ///       after moving. Realistically, using a pointer after moving is
    ///       already a bad idea anyway since it may result in use-after-move.
    ///
    /// \return the underlying nullable pointer
    constexpr auto as_nullable() const & noexcept -> const T&;
    NOT_NULL_CPP14_CONSTEXPR auto as_nullable() && noexcept -> T&&;
    /// \}

    //-------------------------------------------------------------------------

    /// \brief Dereferences the underlying pointer
    ///
    /// \return the underlying pointer
    constexpr auto operator->() const noexcept -> pointer;

    /// \brief Dereferences the underlying pointer
    ///
    /// \return reference to the underlying pointer
    constexpr auto operator*() const noexcept -> reference;

    //-------------------------------------------------------------------------
    // Deleted Operators
    //-------------------------------------------------------------------------

    // If you get an error here, it's because you're doing something you
    // shouldn't be with `not_null`. Don't do that thing.
    auto operator++() -> void = delete;
    auto operator--() -> void = delete;
    auto operator++(int) -> void = delete;
    auto operator--(int) -> void = delete;
    auto operator+=(std::ptrdiff_t) -> void = delete;
    auto operator-=(std::ptrdiff_t) -> void= delete;
    auto operator[](std::ptrdiff_t) const -> void = delete;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    T m_pointer;

    //-------------------------------------------------------------------------
    // Private Constructors
    //-------------------------------------------------------------------------
  private:

    struct ctor_tag{};

    template <typename P>
    constexpr not_null(ctor_tag, P&& ptr)
      noexcept(std::is_nothrow_constructible<typename std::decay<P>::type,P>::value);

    template <typename U>
    friend constexpr auto assume_not_null(U&&)
      noexcept(std::is_nothrow_constructible<typename std::decay<U>::type,U>::value)
      -> not_null<typename std::decay<U>::type>;

  };

  //===========================================================================
  // non-member functions : class : not_null
  //===========================================================================

  //---------------------------------------------------------------------------
  // Utilities
  //---------------------------------------------------------------------------

  template <typename T>
  constexpr auto check_not_null(T&& ptr)
    -> not_null<typename std::decay<T>::type>;

  template <typename T>
  constexpr auto assume_not_null(T&& ptr)
    noexcept(std::is_nothrow_constructible<typename std::decay<T>::type,T>::value)
    -> not_null<typename std::decay<T>::type>;

  //---------------------------------------------------------------------------
  // Comparisons
  //---------------------------------------------------------------------------

  template <typename T>
  constexpr auto operator==(const not_null<T>& lhs, std::nullptr_t) noexcept -> bool;
  template <typename T>
  constexpr auto operator==(std::nullptr_t, const not_null<T>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() == std::declval<const U&>())>
  constexpr auto operator==(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() == std::declval<const U&>())>
  constexpr auto operator==(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() == std::declval<const U&>())>
  constexpr auto operator==(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

  template <typename T>
  constexpr auto operator!=(const not_null<T>& lhs, std::nullptr_t) noexcept -> bool;
  template <typename T>
  constexpr auto operator!=(std::nullptr_t, const not_null<T>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() != std::declval<const U&>())>
  constexpr auto operator!=(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() != std::declval<const U&>())>
  constexpr auto operator!=(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() != std::declval<const U&>())>
  constexpr auto operator!=(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() < std::declval<const U&>())>
  constexpr auto operator<(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() < std::declval<const U&>())>
  constexpr auto operator<(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() < std::declval<const U&>())>
  constexpr auto operator<(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() > std::declval<const U&>())>
  constexpr auto operator>(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() > std::declval<const U&>())>
  constexpr auto operator>(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() > std::declval<const U&>())>
  constexpr auto operator>(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() <= std::declval<const U&>())>
  constexpr auto operator<=(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() <= std::declval<const U&>())>
  constexpr auto operator<=(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() <= std::declval<const U&>())>
  constexpr auto operator<=(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() >= std::declval<const U&>())>
  constexpr auto operator>=(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() >= std::declval<const U&>())>
  constexpr auto operator>=(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() >= std::declval<const U&>())>
  constexpr auto operator>=(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

} // inline namespace bitwizeshift
} // namespace cpp

inline
NOT_NULL_NS_IMPL::not_null_contract_violation::not_null_contract_violation()
  : logic_error{
      "check_not_null invoked with null pointer; "
      "not_null's contruct has been violated"
    }
{

}

//=============================================================================
// detail utilities : not_null
//=============================================================================

template <typename Ptr>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::detail::not_null_to_address(const Ptr& p)
  noexcept -> decltype(::NOT_NULL_NS_IMPL::detail::not_null_to_address(p.operator->()))
{
#if __cplusplus >= 202002L
  return std::to_address(p);
#else
  return not_null_to_address(p.operator->());
#endif
}

template <typename U>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::detail::not_null_to_address(U* p)
  noexcept -> U*
{
  return p;
}

inline NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::detail::throw_null_pointer_error()
  -> void
{
#if defined(NOT_NULL_DISABLE_EXCEPTIONS)
  std::fprintf(
    stderr,
    "check_not_null invoked with null pointer; "
    "not_null's contruct has been violated"
  );
  std::abort();
#else
  throw not_null_contract_violation{};
#endif
}

//=============================================================================
// class : not_null
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T>
template <typename U,
          typename std::enable_if<NOT_NULL_NS_IMPL::detail::not_null_is_implicit_convertible<T,const U&>::value,int>::type>
NOT_NULL_NS_IMPL::not_null<T>::not_null(const not_null<U>& other)
  noexcept(std::is_nothrow_constructible<T,const U&>::value)
  : m_pointer(other.as_nullable())
{

}

template <typename T>
template <typename U,
          typename std::enable_if<NOT_NULL_NS_IMPL::detail::not_null_is_explicit_convertible<T,const U&>::value,int>::type>
NOT_NULL_NS_IMPL::not_null<T>::not_null(const not_null<U>& other)
  noexcept(std::is_nothrow_constructible<T,const U&>::value)
  : m_pointer(other.as_nullable())
{

}

template <typename T>
template <typename U,
          typename std::enable_if<NOT_NULL_NS_IMPL::detail::not_null_is_implicit_convertible<T,U&&>::value,int>::type>
NOT_NULL_NS_IMPL::not_null<T>::not_null(not_null<U>&& other)
  noexcept(std::is_nothrow_constructible<T,U&&>::value)
  : m_pointer(static_cast<not_null<U>&&>(other).as_nullable())
{

}

template <typename T>
template <typename U,
          typename std::enable_if<NOT_NULL_NS_IMPL::detail::not_null_is_explicit_convertible<T,U&&>::value,int>::type>
NOT_NULL_NS_IMPL::not_null<T>::not_null(not_null<U>&& other)
  noexcept(std::is_nothrow_constructible<T,U&&>::value)
  : m_pointer(static_cast<not_null<U>&&>(other).as_nullable())
{

}

//-----------------------------------------------------------------------------

template <typename T>
template <typename U, typename>
inline NOT_NULL_CPP14_CONSTEXPR NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::not_null<T>::operator=(const not_null<U>& other)
  noexcept(std::is_nothrow_assignable<T&,const U&>::value) -> not_null&
{
  m_pointer = other.as_nullable();
  return (*this);
}

template <typename T>
template <typename U, typename>
inline NOT_NULL_CPP14_CONSTEXPR NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::not_null<T>::operator=(not_null<U>&& other)
  noexcept(std::is_nothrow_assignable<T&,U&&>::value) -> not_null&
{
  m_pointer = static_cast<not_null<U>&&>(other).as_nullable();
  return (*this);
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template<typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::not_null<T>::get()
  const noexcept -> pointer
{
  return detail::not_null_to_address(m_pointer);
}

template <typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
NOT_NULL_NS_IMPL::not_null<T>::operator bool()
  const noexcept
{
  return true;
}

//-----------------------------------------------------------------------------

template<typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::not_null<T>::as_nullable()
  const & noexcept -> const T&
{
  return m_pointer;
}

template<typename T>
inline NOT_NULL_CPP14_CONSTEXPR NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::not_null<T>::as_nullable()
  && noexcept -> T&&
{
  return static_cast<T&&>(m_pointer);
}

//-----------------------------------------------------------------------------

template<typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::not_null<T>::operator->()
  const noexcept -> pointer
{
  return get();
}

template<typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::not_null<T>::operator*()
  const noexcept -> reference
{
  return *get();
}

//-----------------------------------------------------------------------------
// Private Constructor
//-----------------------------------------------------------------------------

template <typename T>
template <typename P>
inline constexpr NOT_NULL_INLINE_VISIBILITY
cpp::bitwizeshift::not_null<T>::not_null(ctor_tag, P&& ptr)
  noexcept(std::is_nothrow_constructible<typename std::decay<P>::type, P>::value)
  : m_pointer(detail::not_null_forward<P>(ptr))
{

}

//=============================================================================
// non-member functions : class : not_null
//=============================================================================

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

template <typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::check_not_null(T&& t)
  -> not_null<typename std::decay<T>::type>
{
  return (t != nullptr || (detail::throw_null_pointer_error(), true)),
    assume_not_null(detail::not_null_forward<T>(t));
}

template <typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::assume_not_null(T&& t)
  noexcept(std::is_nothrow_constructible<typename std::decay<T>::type,T>::value)
  -> not_null<typename std::decay<T>::type>
{
  using value_type = typename std::decay<T>::type;

  return not_null<value_type>{
    typename not_null<value_type>::ctor_tag{},
    std::forward<T>(t)
  };
}

//-----------------------------------------------------------------------------
// Comparisons
//-----------------------------------------------------------------------------

template <typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator==(const not_null<T>&, std::nullptr_t)
  noexcept -> bool
{
  return false;
}

template <typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator==(std::nullptr_t, const not_null<T>&)
  noexcept -> bool
{
  return false;
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator==(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() == rhs.as_nullable();
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator==(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() == rhs;
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator==(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs == rhs.as_nullable();
}

template <typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator!=(const not_null<T>&, std::nullptr_t)
  noexcept -> bool
{
  return true;
}

template <typename T>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator!=(std::nullptr_t, const not_null<T>&)
  noexcept -> bool
{
  return true;
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator!=(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() != rhs.as_nullable();
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator!=(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() != rhs;
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator!=(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs != rhs.as_nullable();
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator<(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() < rhs.as_nullable();
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator<(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() < rhs;
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator<(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs < rhs.as_nullable();
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator>(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() > rhs.as_nullable();
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator>(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() > rhs;
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator>(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs > rhs.as_nullable();
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator<=(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() <= rhs.as_nullable();
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator<=(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() <= rhs;
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator<=(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs <= rhs.as_nullable();
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator>=(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() >= rhs.as_nullable();
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator>=(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() >= rhs;
}

template <typename T, typename U, typename>
inline constexpr NOT_NULL_INLINE_VISIBILITY
auto NOT_NULL_NS_IMPL::operator>=(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs >= rhs.as_nullable();
}

#endif /* CPP_BITWIZESHIFT_NOT_NULL_HPP */
