#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include <concepts>

namespace Krys::detail_concepts
{
  template <typename T, template <typename...> class Templ>
  struct IsSpecializationOf : std::false_type
  {
  };

  template <typename... T, template <typename...> class Templ>
  struct IsSpecializationOf<Templ<T...>, Templ> : std::true_type
  {
  };

  template <typename T>
  struct TypeIdentity
  {
    using type = T;
  };
}

namespace Krys
{
  template <typename T>
  concept Integral = std::is_integral_v<T>;

  template <typename T>
  concept Unsigned = std::is_unsigned_v<T>;

  template <typename T>
  concept Signed = std::is_signed_v<T>;

  template <typename T>
  concept SignedIntegral = Integral<T> && Signed<T>;

  template <typename T>
  concept UnsignedIntegral = Integral<T> && Unsigned<T>;

  template <typename T>
  concept FloatingPoint = std::is_floating_point_v<T>;

  template <typename T>
  concept Number = Integral<T> || FloatingPoint<T>;

  template <typename T>
  concept Scalar = std::is_scalar_v<T>;

  template <typename T>
  concept Void = std::is_void_v<T>;

  template <typename T>
  concept IsPointer = std::is_pointer_v<T>;

  template <typename T>
  concept IsEnum = std::is_enum_v<T>;

  template <typename T>
  concept IsArray = std::is_array_v<T>;

  template <typename T>
  concept IsFunction = std::is_function_v<T>;

  template <typename TFunc, typename... Args>
  concept Callable = std::invocable<TFunc, Args...>;

  template <typename TFunc, typename... Args>
  concept NoThrowInvocable = std::is_nothrow_invocable_v<TFunc, Args...>;

  template <typename TReturn, typename TFunc, typename... Args>
  concept InvocableReturns = std::is_invocable_r_v<TReturn, TFunc, Args...>;

  template <typename TReturn, typename TFunc, typename... Args>
  concept NoThrowInvocableReturns = std::is_nothrow_invocable_r_v<TReturn, TFunc, Args...>;

  /// @brief Checks type `T` is callable type that does not cause side effects.
  template <typename T, typename... Args>
  concept RegularCallable = std::regular_invocable<T, Args...>;

  template <typename T, typename... Args>
  concept Predicate = std::predicate<T, Args...>;

  template <typename T, typename... Args>
  concept Constructible = std::is_constructible_v<T, Args...>;

  template <typename T, typename... Args>
  concept TriviallyConstructible = std::is_trivially_constructible_v<T, Args...>;

  template <typename T, typename... Args>
  concept NoThrowConstructible = std::is_nothrow_constructible_v<T, Args...>;

  template <typename T>
  concept DefaultConstructible = std::is_default_constructible_v<T>;

  template <typename T>
  concept TriviallyDefaultConstructible = std::is_trivially_default_constructible_v<T>;

  template <typename T>
  concept NoThrowDefaultConstructible = std::is_nothrow_default_constructible_v<T>;

  template <typename TTo, typename TFrom>
  concept Assignable = std::is_assignable_v<TTo, TFrom>;

  template <typename TTo, typename TFrom>
  concept TriviallyAssignable = std::is_trivially_assignable_v<TTo, TFrom>;

  template <typename TTo, typename TFrom>
  concept NoThrowAssignable = std::is_nothrow_assignable_v<TTo, TFrom>;

  template <typename T>
  concept Destructible = std::is_destructible_v<T>;

  template <typename T>
  concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

  template <typename T>
  concept NoThrowDestructible = std::is_nothrow_destructible_v<T>;

  template <typename T>
  concept CopyConstructible = std::is_copy_constructible_v<T>;

  template <typename T>
  concept TriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;

  template <typename T>
  concept NoThrowCopyConstructible = std::is_nothrow_copy_constructible_v<T>;

  template <typename T>
  concept CopyAssignable = std::is_copy_assignable_v<T>;

  template <typename T>
  concept TriviallyCopyAssignable = std::is_trivially_copy_assignable_v<T>;

  template <typename T>
  concept NoThrowCopyAssignable = std::is_nothrow_copy_assignable_v<T>;

  template <typename T>
  concept MoveConstructible = std::is_move_constructible_v<T>;

  template <typename T>
  concept TriviallyMoveConstructible = std::is_trivially_move_constructible_v<T>;

  template <typename T>
  concept NoThrowMoveConstructible = std::is_nothrow_move_constructible_v<T>;

  template <typename T>
  concept MoveAssignable = std::is_move_assignable_v<T>;

  template <typename T>
  concept TriviallyMoveAssignable = std::is_trivially_move_assignable_v<T>;

  template <typename T>
  concept NoThrowMoveAssignable = std::is_nothrow_move_assignable_v<T>;

  template <typename T, typename... Args>
  concept ConstructibleFrom = std::constructible_from<T, Args...>;

  template <typename TDerived, typename TBase>
  concept DerivedFrom = std::derived_from<TDerived, TBase>;

  template <typename TBase, typename TDerived>
  concept BaseOf = std::is_base_of_v<TBase, TDerived>;

  template <typename T, typename U>
  concept SameType = std::is_same_v<T, U>;

  template <typename T, typename... Types>
  concept OneOf = std::disjunction_v<std::is_same<T, Types>...>;

  template <typename T>
  concept Reference = std::is_reference_v<T>;

  /// @brief Determines whether the given type is an l-value reference.
  template <typename T>
  concept LValueRef = std::is_lvalue_reference<T>::value;

  /// @brief Determines whether the given type is an r-value reference.
  template <typename T>
  concept RValueRef = std::is_rvalue_reference<T>::value;

  template <typename T>
  concept IsConst = std::is_const_v<remove_ref_t<T>>;

  template <typename T>
  concept Class = std::is_class_v<T>;

  template <typename T>
  concept IsFinal = std::is_final_v<T>;

  /// @brief Concept that checks whether TFrom is convertible to TTo.
  template <typename TFrom, typename TTo>
  concept ConvertibleTo = std::convertible_to<TFrom, TTo>;

  template <typename TFrom, typename TTo>
  concept NoThrowConvertibleTo = std::is_nothrow_convertible_v<TFrom, TTo>;

  template <typename T>
  concept EqualityComparable = requires(T a, T b) {
    { a == b } -> ConvertibleTo<bool>;
  };

  template <typename T>
  concept InEqualityComparable = requires(T a, T b) {
    { a == b } -> ConvertibleTo<bool>;
  };

  template <typename T>
  concept LessThanComparable = requires(T a, T b) {
    { a < b } -> ConvertibleTo<bool>;
  };

  template <typename T>
  concept GreaterThanComparable = requires(T a, T b) {
    { a > b } -> ConvertibleTo<bool>;
  };

  template <typename T>
  concept HasStandardLayout = std::is_standard_layout_v<T>;

  /// @brief Checks whether the given full, complete type from the first argument is related to the raw
  /// template name provided in the second.
  template <typename T, template <typename...> class Templ>
  concept IsSpecializationOf = ::Krys::detail_concepts::IsSpecializationOf<remove_cvref_t<T>, Templ>::value;

  /// @brief A type trait that yields the type passed to it unchanged.
  template <typename T>
  using TypeIdentity = typename ::Krys::detail_concepts::TypeIdentity<T>::type;

  template <typename TFrom, typename TTo>
  concept NonDerivedCompatiblePointer = ConvertibleTo<TFrom (*)[], TTo (*)[]>;

  template <typename T>
  concept HasUniqueObjectRepresentations = std::has_unique_object_representations_v<T>;

  template <typename T>
  concept IsEmpty = std::is_empty_v<T>;

  template <typename T>
  concept Swappable = std::is_swappable_v<T>;

  template <typename T>
  concept NoThrowSwappable = std::is_nothrow_swappable_v<T>;
}