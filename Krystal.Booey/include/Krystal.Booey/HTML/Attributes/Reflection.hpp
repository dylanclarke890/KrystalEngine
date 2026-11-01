#pragma once

#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/ElementInternals.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/NTTPMaybe.hpp"
#include "Krystal.Core/Types/StronglyTypedValue.hpp"
#include <charconv>
#include <limits>

namespace krys::boo::html
{
  template <typename T>
  concept ReflectTarget = DerivedFrom<T, HTMLElement> || SameType<T, ElementInternals>;

  template <typename T>
  concept ReflectTypeDOMString = OneOf<T, dom::DOMString, Maybe<dom::DOMString>>;

  template <typename T>
  concept ReflectURLType = OneOf<T, dom::USVString>;

  // FIXME: FrozenArray<T>? is also one of the possible types.
  // NOTE: DOMTokenList is also a possible type for reflection but it just consists of returning a
  // DOMTokenList, would be harder to handle here.
  template <typename T>
  concept ReflectType = ReflectTypeDOMString<T> || ReflectURLType<T> || OneOf<T, bool, int32, uint32, double>;

  template <typename T>
  concept ReflectNonNegativeType = OneOf<T, int32>;

  template <typename T>
  concept ReflectPositiveType = OneOf<T, double, uint32>;

  template <typename T>
  concept ReflectPositiveWithFallbackType = ReflectPositiveType<T>;

  template <typename T>
  concept ReflectDefaultGetterType = OneOf<T, double, int32, uint32>;

  template <typename T>
  concept ReflectDefaultSetterType = OneOf<T, uint32>;

  template <typename T>
  concept ReflectRangeType = OneOf<T, uint32>;

  template <ReflectRangeType T>
  struct ReflectRange
  {
    int32 ClampedMin {0};
    int32 ClampedMax {0};
  };

  template <Number T>
  struct ReflectDefault
      : public StronglyTypedNumber<ReflectDefault<T>, conditional_t<SameType<T, double>, double, uint32>>
  {
    using StronglyTypedNumber<ReflectDefault<T>,
                              conditional_t<SameType<T, double>, double, uint32>>::StronglyTypedNumber;
  };

  /// @brief Implements the logic for reflecting content attributes to IDL attributes and vice versa.
  /// Enumerated attributes and attributes that are limited to a set of known values
  /// (dom::DOMString/Maybe<dom::DOMString>) are not handled by the generic Reflect functions and need to be
  /// handled separately by the caller. This includes 'missing', 'default' and 'invalid' value handling.
  /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#reflect
  class Reflection
  {
#pragma region Types and Constants

    template <typename T>
    using MaybeReflectDefault = NTTPMaybe<ReflectDefault<T>>;

    template <typename T>
    using MaybeReflectRange = NTTPMaybe<ReflectRange<T>>;

    template <typename T>
    constexpr static MaybeReflectDefault<T> NoDefaultValue = {};

    template <typename T>
    constexpr static MaybeReflectRange<T> NoRange = {};

    struct OnlyNonNegativeNumbers : public StronglyTypedBool<OnlyNonNegativeNumbers>
    {
      using Base::Base;
    };

    struct OnlyPositiveNumbers : public StronglyTypedBool<OnlyPositiveNumbers>
    {
      using Base::Base;
    };

    struct OnlyPositiveNumbersWithFallback : public StronglyTypedBool<OnlyPositiveNumbersWithFallback>
    {
      using Base::Base;
    };

    struct TreatedAsURL : public StronglyTypedBool<TreatedAsURL>
    {
      using Base::Base;
    };

    template <typename T>
    using reflect_get_return_t = conditional_t<SameType<T, dom::DOMString>, Maybe<dom::DOMString>, T>;

    template <typename T, OnlyNonNegativeNumbers OnlyNonNegative, OnlyPositiveNumbers OnlyPositive>
    using reflect_set_return_t =
      conditional_t<(SameType<T, int32> && OnlyNonNegative) || (SameType<T, uint32> && OnlyPositive),
                    dom::ExceptionOr<void>, void>;

#pragma endregion

  public:
#pragma region Reflect

    /// @brief Represents the `[Reflect]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    template <ReflectType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> Reflect(const Target &target,
                                                               dom::DOMStringAtom name) noexcept
    {
      if constexpr (OneOf<TValue, dom::DOMString, Maybe<dom::DOMString>>)
      {
        return ReflectDOMString(target, name);
      }
      else if constexpr (SameType<TValue, dom::USVString>)
      {
        return ReflectUSVString<TreatedAsURL(false)>(target, name);
      }
      else if constexpr (SameType<TValue, bool>)
      {
        return ReflectBool(target, name);
      }
      else if constexpr (SameType<TValue, int32>)
      {
        return ReflectLong<OnlyNonNegativeNumbers(false), NoDefaultValue<int32>>(target, name);
      }
      else if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(false), OnlyPositiveNumbersWithFallback(false),
                                   NoDefaultValue<uint32>, NoRange<uint32>>(target, name);
      }
      else if constexpr (SameType<TValue, double>)
      {
        return ReflectDouble<OnlyPositiveNumbers(false), NoDefaultValue<double>>(target, name);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [Reflect] (getter)");
      }
    }

    /// @brief Represents the `[Reflect]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    template <ReflectType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<TValue, OnlyNonNegativeNumbers(false),
                                               OnlyPositiveNumbers(false)>
      Reflect(Target &target, dom::DOMStringAtom name, TValue &&value) noexcept
    {
      static_assert(!IsConst<Target>, "Target cannot be const for [Reflect] setter");

      if constexpr (OneOf<TValue, dom::DOMString, Maybe<dom::DOMString>>)
      {
        ReflectDOMString(target, name, krys::move(value));
      }
      else if constexpr (SameType<TValue, dom::USVString>)
      {
        return ReflectUSVString<TreatedAsURL(false)>(target, name, krys::move(value));
      }
      else if constexpr (SameType<TValue, bool>)
      {
        ReflectBool(target, name, value);
      }
      else if constexpr (SameType<TValue, int32>)
      {
        ReflectLong<OnlyNonNegativeNumbers(false)>(target, name, value);
      }
      else if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLong<OnlyPositiveNumbers(false), OnlyPositiveNumbersWithFallback(false),
                            NoDefaultValue<uint32>>(target, name, value);
      }
      else if constexpr (SameType<TValue, double>)
      {
        ReflectDouble<OnlyPositiveNumbers(false)>(target, name, value);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [Reflect] (setter)");
      }
    }

    /// @brief Represents the `[Reflect][ReflectDefault]` combination of IDL attributes (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectDefaultGetterType TValue, ReflectDefault<TValue> DefaultValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> Reflect(const Target &target,
                                                               dom::DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, int32>)
      {
        return ReflectLong<OnlyNonNegativeNumbers(false), MaybeReflectDefault<int32>(DefaultValue)>(target,
                                                                                                    name);
      }
      else if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(false), OnlyPositiveNumbersWithFallback(false),
                                   MaybeReflectDefault<uint32>(DefaultValue), NoRange<uint32>>(target, name);
      }
      else if constexpr (SameType<TValue, double>)
      {
        return ReflectDouble<OnlyPositiveNumbers(false), MaybeReflectDefault<double>(DefaultValue)>(target,
                                                                                                    name);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [Reflect][ReflectDefault] (getter)");
      }
    }

    /// @brief Represents the `[Reflect][ReflectDefault]` combination of IDL attributes (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectDefaultSetterType TValue, ReflectDefault<TValue> DefaultValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<TValue, OnlyNonNegativeNumbers(false),
                                               OnlyPositiveNumbers(false)>
      Reflect(Target &target, dom::DOMStringAtom name, TValue value) noexcept
    {
      static_assert(!IsConst<Target>, "Target cannot be const for [Reflect][ReflectDefault] setter");

      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(false), OnlyPositiveNumbersWithFallback(false),
                                   MaybeReflectDefault<uint32>(DefaultValue)>(target, name, value);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [Reflect][ReflectDefault] (setter)");
      }
    }

    /// @brief Represents the `[Reflect][ReflectRange]` combination of IDL attributes (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    /// @see https://html.spec.whatwg.org/#xattr-reflectrange
    template <ReflectRangeType TValue, ReflectRange<TValue> Range, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> Reflect(const Target &target,
                                                               dom::DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(false), OnlyPositiveNumbersWithFallback(false),
                                   NoDefaultValue<uint32>, MaybeReflectRange<uint32>(Range)>(target, name);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [Reflect][ReflectRange] (getter)");
      }
    }

    /// @brief Represents the `[Reflect][ReflectDefault][ReflectRange]` combination of IDL attributes
    /// (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    /// @see https://html.spec.whatwg.org/#xattr-reflectrange
    template <ReflectRangeType TValue, ReflectDefault<TValue> DefaultValue, ReflectRange<TValue> Range,
              ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> Reflect(const Target &target,
                                                               dom::DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(false), OnlyPositiveNumbersWithFallback(false),
                                   MaybeReflectDefault<uint32>(DefaultValue),
                                   MaybeReflectRange<uint32>(Range)>(target, name);
      }
      else
      {
        static_assert(DependentFalse<TValue>,
                      "Unknown type for [Reflect][ReflectDefault][ReflectRange] (getter)");
      }
    }

#pragma endregion

#pragma region ReflectSetter

    /// @brief Represents the `[ReflectSetter]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectsetter
    template <ReflectType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<TValue, OnlyNonNegativeNumbers(false),
                                               OnlyPositiveNumbers(false)>
      ReflectSetter(Target &target, dom::DOMStringAtom name, TValue &&value) noexcept
    {
      static_assert(!IsConst<Target>, "Target cannot be const for [ReflectSetter] setter");

      return Reflect(target, name, krys::move(value));
    }

#pragma endregion

#pragma region ReflectURL

    /// @brief Represents the `[ReflectURL]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflecturl
    template <ReflectURLType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> ReflectURL(const Target &target,
                                                                  dom::DOMStringAtom name) noexcept
    {
      return ReflectUSVString<TreatedAsURL(true)>(target, name);
    }

    /// @brief Represents the `[ReflectURL]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflecturl
    template <ReflectURLType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<TValue, OnlyNonNegativeNumbers(false),
                                               OnlyPositiveNumbers(false)>
      ReflectURL(Target &target, dom::DOMStringAtom name, TValue &&value) noexcept
    {
      static_assert(!IsConst<Target>, "Target cannot be const for [ReflectURL] setter");

      return ReflectUSVString<TreatedAsURL(true)>(target, name, krys::move(value));
    }

#pragma endregion

#pragma region ReflectNonNegative

    /// @brief Represents the `[ReflectNonNegative]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectnonnegative
    template <ReflectNonNegativeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> ReflectNonNegative(const Target &target,
                                                                          dom::DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, int32>)
      {
        return ReflectLong<OnlyNonNegativeNumbers(true), NoDefaultValue<int32>>(target, name);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [ReflectNonNegative] (getter)");
      }
    }

    /// @brief Represents the `[ReflectNonNegative]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectnonnegative
    template <ReflectNonNegativeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<TValue, OnlyNonNegativeNumbers(true),
                                               OnlyPositiveNumbers(false)>
      ReflectNonNegative(Target &target, dom::DOMStringAtom name, TValue value) noexcept
    {
      static_assert(!IsConst<Target>, "Target cannot be const for [ReflectNonNegative] setter");

      if constexpr (SameType<TValue, int32>)
      {
        return ReflectLong<OnlyNonNegativeNumbers(true)>(target, name, value);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [ReflectNonNegative] (setter)");
      }
    }

    /// @brief Represents the `[ReflectNonNegative][ReflectDefault]` combination of IDL attributes (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectnonnegative
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectNonNegativeType TValue, ReflectDefault<TValue> DefaultValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> ReflectNonNegative(const Target &target,
                                                                          dom::DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, int32>)
      {
        return ReflectLong<OnlyNonNegativeNumbers(true), MaybeReflectDefault<int32>(DefaultValue)>(target,
                                                                                                   name);
      }
      else
      {
        static_assert(DependentFalse<TValue>,
                      "Unknown type for [ReflectNonNegative][ReflectDefault] (getter)");
      }
    }

#pragma endregion

#pragma region ReflectPositive

    /// @brief Represents the `[ReflectPositive]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositive
    template <ReflectPositiveType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> ReflectPositive(const Target &target,
                                                                       dom::DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(true), OnlyPositiveNumbersWithFallback(false),
                                   NoDefaultValue<uint32>, NoRange<uint32>>(target, name);
      }
      else if constexpr (SameType<TValue, double>)
      {
        return ReflectDouble<OnlyPositiveNumbers(true), NoDefaultValue<double>>(target, name);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [ReflectPositive] (getter)");
      }
    }

    /// @brief Represents the `[ReflectPositive]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositive
    template <ReflectPositiveType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<TValue, OnlyNonNegativeNumbers(false),
                                               OnlyPositiveNumbers(true)>
      ReflectPositive(Target &target, dom::DOMStringAtom name, TValue value) noexcept
    {
      static_assert(!IsConst<Target>, "Target cannot be const for [ReflectPositive] setter");

      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(true), OnlyPositiveNumbersWithFallback(false),
                                   NoDefaultValue<uint32>>(target, name, value);
      }
      else if constexpr (SameType<TValue, double>)
      {
        return ReflectDouble<OnlyPositiveNumbers(true)>(target, name, value);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [ReflectPositive] (setter)");
      }
    }

    /// @brief Represents the `[ReflectPositive][ReflectDefault]` combination of IDL
    /// attributes (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositive
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectPositiveType TValue, ReflectDefault<TValue> DefaultValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> ReflectPositive(const Target &target,
                                                                       dom::DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(true), OnlyPositiveNumbersWithFallback(false),
                                   MaybeReflectDefault<uint32>(DefaultValue), NoRange<uint32>>(target, name);
      }
      else if constexpr (SameType<TValue, double>)
      {
        return ReflectDouble<OnlyPositiveNumbers(true), MaybeReflectDefault<double>(DefaultValue)>(target,
                                                                                                   name);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [ReflectPositive][ReflectDefault] (getter)");
      }
    }

    /// @brief Represents the `[ReflectPositive][ReflectDefault]` combination of IDL
    /// attributes (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositive
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectPositiveType TValue, ReflectDefault<TValue> DefaultValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<TValue, OnlyNonNegativeNumbers(false),
                                               OnlyPositiveNumbers(true)>
      ReflectPositive(Target &target, dom::DOMStringAtom name, TValue value) noexcept
    {
      static_assert(!IsConst<Target>, "Target cannot be const for [ReflectPositive][ReflectDefault] setter");

      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(true), OnlyPositiveNumbersWithFallback(false),
                                   MaybeReflectDefault<uint32>(DefaultValue)>(target, name, value);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [ReflectPositive][ReflectDefault] (setter)");
      }
    }

#pragma endregion

#pragma region ReflectPositiveWithFallback

    /// @brief Represents the `[ReflectPositiveWithFallback]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositivewithfallback
    template <ReflectPositiveWithFallbackType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue>
      ReflectPositiveWithFallback(const Target &target, dom::DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(false), OnlyPositiveNumbersWithFallback(true),
                                   NoDefaultValue<uint32>, NoRange<uint32>>(target, name);
      }
      else if constexpr (SameType<TValue, double>)
      {
        return ReflectDouble<OnlyPositiveNumbers(true), NoDefaultValue<double>>(target, name);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [ReflectPositiveWithFallback] (getter)");
      }
    }

    /// @brief Represents the `[ReflectPositiveWithFallback]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositivewithfallback
    template <ReflectPositiveWithFallbackType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<TValue, OnlyNonNegativeNumbers(false),
                                               OnlyPositiveNumbers(false)>
      ReflectPositiveWithFallback(Target &target, dom::DOMStringAtom name, TValue value) noexcept
    {
      static_assert(!IsConst<Target>, "Target cannot be const for [ReflectPositiveWithFallback] setter");

      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(false), OnlyPositiveNumbersWithFallback(true),
                                   NoDefaultValue<uint32>>(target, name, value);
      }
      else if constexpr (SameType<TValue, double>)
      {
        return ReflectDouble<OnlyPositiveNumbers(true)>(target, name, value);
      }
      else
      {
        static_assert(DependentFalse<TValue>, "Unknown type for [ReflectPositiveWithFallback] (setter)");
      }
    }

    /// @brief Represents the `[ReflectPositiveWithFallback][ReflectDefault]` combination of IDL
    /// attributes (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositivewithfallback
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectPositiveWithFallbackType TValue, ReflectDefault<TValue> DefaultValue,
              ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue>
      ReflectPositiveWithFallback(const Target &target, dom::DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(false), OnlyPositiveNumbersWithFallback(true),
                                   MaybeReflectDefault<uint32>(DefaultValue), NoRange<uint32>>(target, name);
      }
      else if constexpr (SameType<TValue, double>)
      {
        return ReflectDouble<OnlyPositiveNumbers(true), MaybeReflectDefault<double>(DefaultValue)>(target,
                                                                                                   name);
      }
      else
      {
        static_assert(DependentFalse<TValue>,
                      "Unknown type for [ReflectPositiveWithFallback][ReflectDefault] (getter)");
      }
    }

    /// @brief Represents the `[ReflectPositiveWithFallback][ReflectDefault]` combination of IDL
    /// attributes (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositivewithfallback
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectPositiveWithFallbackType TValue, ReflectDefault<TValue> DefaultValue,
              ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<TValue, OnlyNonNegativeNumbers(false),
                                               OnlyPositiveNumbers(SameType<TValue, double>)>
      ReflectPositiveWithFallback(Target &target, dom::DOMStringAtom name, TValue value) noexcept
    {
      static_assert(!IsConst<Target>,
                    "Target cannot be const for [ReflectPositiveWithFallback][ReflectDefault] setter");

      if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong<OnlyPositiveNumbers(false), OnlyPositiveNumbersWithFallback(true),
                                   MaybeReflectDefault<uint32>(DefaultValue)>(target, name, value);
      }
      else if constexpr (SameType<TValue, double>)
      {
        // NOTE: The spec says this attribute can be used for 'double' but doesn't actually handle the case
        // for 'OnlyPositiveWithFallback' for 'double' reflected attributes. Here we assume it has the same
        // behavior as 'OnlyPositive' for 'double'.

        return ReflectDouble<OnlyPositiveNumbers(true), MaybeReflectDefault<double>(DefaultValue)>(
          target, name, value);
      }
      else
      {
        static_assert(DependentFalse<TValue>,
                      "Unknown type for [ReflectPositiveWithFallback][ReflectDefault] (setter)");
      }
    }

#pragma endregion

  private:
#pragma region HTMLElement ReflectTarget

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-element
    KRYS_NODISCARD static HTMLElement *GetElement(HTMLElement &element) noexcept
    {
      return &element;
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-content-attribute
    KRYS_NODISCARD static Maybe<dom::DOMString> GetContentAttribute(const HTMLElement &element,
                                                                    dom::DOMStringAtom name) noexcept
    {
      auto attribute =
        dom::ElementAlgorithms::GetAttributeByNamespace(dom::DOMStringAtom::Null(), name, element);
      if (attribute == nullptr)
      {
        return null;
      }

      return attribute->Value();
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#set-the-content-attribute
    static void SetContentAttribute(HTMLElement &element, dom::DOMStringAtom name,
                                    dom::DOMString &&value) noexcept
    {
      dom::ElementAlgorithms::SetAttributeValue(element, name, krys::move(value));
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#delete-the-content-attribute
    static void DeleteContentAttribute(HTMLElement &element, dom::DOMStringAtom name) noexcept
    {
      dom::ElementAlgorithms::RemoveAttributeByNamespace(dom::DOMStringAtom::Null(), name, element);
    }

#pragma endregion

#pragma region ElementInternals ReflectTarget

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-element
    KRYS_NODISCARD static HTMLElement *GetElement(ElementInternals &internals) noexcept
    {
      return internals.TargetElement();
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-content-attribute
    KRYS_NODISCARD static Maybe<dom::DOMString> GetContentAttribute(const ElementInternals &internals,
                                                                    dom::DOMStringAtom name) noexcept
    {
      // FIXME: The internals' ARIA attributes, with accessibility.
      (void)internals;
      (void)name;
      return null;
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#set-the-content-attribute
    static void SetContentAttribute(ElementInternals &internals, dom::DOMStringAtom name,
                                    dom::DOMString &&value) noexcept
    {
      // FIXME: The internals' ARIA attributes, with accessibility.
      (void)internals;
      (void)name;
      (void)value;
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#delete-the-content-attribute
    static void DeleteContentAttribute(ElementInternals &internals, dom::DOMStringAtom name) noexcept
    {
      // FIXME: The internals' ARIA attributes, with accessibility.
      (void)internals;
      (void)name;
    }

#pragma endregion

#pragma region ReflectDOMString

    /// @brief Helper for getting reflected content attributes for 'DOMString/DOMString?' types: steps 1, 2,
    /// 5 and 6 of the `DOMString` getter, and 1, 2 and 5 of the `DOMString?` one, with null for the empty
    /// string (the caller's `value_or`). Enumerated attributes limited to only known values (steps 3 and 4)
    /// are `EnumeratedAttribute::ResolveCanonicalKeyword`.
    /// @see https://html.spec.whatwg.org/#limited-to-only-known-values
    template <ReflectTarget Target>
    KRYS_NODISCARD static Maybe<dom::DOMString> ReflectDOMString(const Target &target,
                                                                 dom::DOMStringAtom name) noexcept
    {
      // 1. Let element be the result of running this's get the element.
      // 2. Let contentAttributeValue be the result of running this's get the content attribute.
      // 5. If contentAttributeValue is null, then return the empty string. (DOMString only.)
      // 6. Return contentAttributeValue.
      return GetContentAttribute(target, name);
    }

    /// @brief Helper for setting reflected content attributes with 'DOMString?' type.
    template <ReflectTarget Target>
    static void ReflectDOMString(Target &target, dom::DOMStringAtom name,
                                 Maybe<dom::DOMString> &&value) noexcept
    {
      // 1. If the given value is null, then run this's delete the content attribute.
      if (!value.has_value())
      {
        DeleteContentAttribute(target, name);
      }
      // 2. Otherwise, run this's set the content attribute with the given value.
      else
      {
        SetContentAttribute(target, name, krys::move(*value));
      }
    }

    /// @brief Helper for setting reflected content attributes with 'DOMString' type.
    template <ReflectTarget Target>
    static void ReflectDOMString(Target &target, dom::DOMStringAtom name, dom::DOMString &&value) noexcept
    {
      // The setter steps are to run this's set the content attribute with the given value.
      SetContentAttribute(target, name, krys::move(value));
    }

#pragma endregion

#pragma region ReflectUSVString

    /// @brief Helper for getting reflected content attributes with 'dom::USVString' type: the content
    /// attribute's value, or the empty string if it's absent. A URL is completed against the node document's
    /// base URL, unless it doesn't parse.
    /// @see https://html.spec.whatwg.org/#treated-as-a-url
    template <TreatedAsURL AsUrl, ReflectTarget Target>
    KRYS_NODISCARD static dom::USVString ReflectUSVString(const Target &target,
                                                          dom::DOMStringAtom name) noexcept
    {
      // 1. Let element be the result of running this's get the element.
      // 2. Let contentAttributeValue be the result of running this's get the content attribute.
      auto contentAttributeValue = GetContentAttribute(target, name);

      // 3. If the reflected IDL attribute is treated as a URL:
      // 3a. If contentAttributeValue is null, then return the empty string.
      // (Without a URL, a null value is the empty string too.)
      if (!contentAttributeValue)
      {
        return {};
      }
      if constexpr (AsUrl)
      {
        // 3b. Let urlString be the result of encoding-parsing-and-serializing a URL given
        //     contentAttributeValue, relative to element's node document.
        // 3c. If urlString is not failure, then return urlString.
        if (auto url = target.NodeDocument().CompleteURL(*contentAttributeValue); url.IsValid())
        {
          return dom::USVString(url.String());
        }
      }

      // 4. Return contentAttributeValue, converted to a scalar value string.
      // (UTF-8 strings are scalar value strings.)
      return dom::USVString(krys::move(*contentAttributeValue));
    }

    /// @brief Helper for setting reflected content attributes with 'dom::USVString' type.
    template <TreatedAsURL AsUrl, ReflectTarget Target>
    static void ReflectUSVString(Target &target, dom::DOMStringAtom name, dom::USVString &&value) noexcept
    {
      // The setter steps are to run this's set the content attribute with the given value.
      SetContentAttribute(target, name, static_cast<dom::DOMString &&>(value));
    }

#pragma endregion

#pragma region ReflectBool

    /// @brief Helper for getting reflected content attributes with 'bool' type: a boolean attribute is true
    /// when it's present, whatever its value.
    /// @see https://html.spec.whatwg.org/#boolean-attribute
    template <ReflectTarget Target>
    KRYS_NODISCARD static bool ReflectBool(const Target &target, dom::DOMStringAtom name) noexcept
    {
      // 1. Let contentAttributeValue be the result of running this's get the content attribute.
      // 2. If contentAttributeValue is null, then return false.
      // 3. Return true.
      return GetContentAttribute(target, name).has_value();
    }

    /// @brief Helper for setting reflected content attributes with 'bool' type.
    template <ReflectTarget Target>
    static void ReflectBool(Target &target, dom::DOMStringAtom name, bool value) noexcept
    {
      // 1. If the given value is false, then run this's delete the content attribute.
      if (!value)
      {
        DeleteContentAttribute(target, name);
      }
      // 2. If the given value is true, then run this's set the content attribute with the empty string.
      else
      {
        SetContentAttribute(target, name, dom::DOMString {});
      }
    }

#pragma endregion

#pragma region ReflectLong

    /// @brief Helper for getting reflected content attributes with 'long' type.
    /// @see https://html.spec.whatwg.org/#default-value
    template <OnlyNonNegativeNumbers OnlyNonNegative, MaybeReflectDefault<int32> DefaultValue,
              ReflectTarget Target>
    KRYS_NODISCARD static int32 ReflectLong(const Target &target, dom::DOMStringAtom name) noexcept
    {
      // 1. Let contentAttributeValue be the result of running this's get the content attribute.
      auto contentAttributeValue = GetContentAttribute(target, name);

      // 2. If contentAttributeValue is not null:
      if (contentAttributeValue.has_value())
      {
        // 2a. Let parsedValue be the result of integer parsing contentAttributeValue if the reflected IDL
        //     attribute is not limited to only non-negative numbers; otherwise the result of non-negative
        //     integer parsing contentAttributeValue.
        auto parsedValue = [&]() -> MicroParserResult<int64>
        {
          if constexpr (OnlyNonNegative)
          {
            auto nonNegative = Numbers::ParseNonNegativeInteger(*contentAttributeValue);
            return {.Value = static_cast<int64>(nonNegative.Value), .Error = nonNegative.Error};
          }
          else
          {
            return Numbers::ParseInteger(*contentAttributeValue);
          }
        }();

        // 2b. If parsedValue is not an error and is within the long range, then return parsedValue.
        if (parsedValue.Success() && parsedValue.Value >= std::numeric_limits<int32>::min()
            && parsedValue.Value <= std::numeric_limits<int32>::max())
        {
          return static_cast<int32>(parsedValue.Value);
        }
      }

      // 3. If the reflected IDL attribute has a default value, then return defaultValue.
      if constexpr (DefaultValue)
      {
        return *DefaultValue;
      }
      // 4. If the reflected IDL attribute is limited to only non-negative numbers, then return −1.
      else if constexpr (OnlyNonNegative)
      {
        return -1;
      }

      // 5. Return 0.
      return 0;
    }

    /// @brief Helper for setting reflected content attributes with 'long' type.
    template <OnlyNonNegativeNumbers OnlyNonNegative, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<int32, OnlyNonNegative, OnlyPositiveNumbers(false)>
      ReflectLong(Target &target, dom::DOMStringAtom name, int32 value) noexcept
    {
      // 1. If the reflected IDL attribute is limited to only non-negative numbers and the given value is
      //    negative, then throw an "IndexSizeError" DOMException.
      if constexpr (OnlyNonNegative)
      {
        if (value < 0)
        {
          return dom::ExceptionCode::IndexSizeError;
        }
      }

      // 2. Run this's set the content attribute with the given value converted to the shortest possible
      // string
      //    representing the number as a valid integer.
      SetContentAttribute(target, name, IntegerToString(value));

      if constexpr (OnlyNonNegative)
      {
        return {};
      }
    }

#pragma endregion

#pragma region ReflectUnsignedLong

    constexpr static int32 MaxUnsignedLongValue {2'147'483'647};

    /// @brief Helper for getting reflected content attributes with 'unsigned long' type.
    /// @see https://html.spec.whatwg.org/#clamped-to-the-range
    template <OnlyPositiveNumbers OnlyPositive, OnlyPositiveNumbersWithFallback OnlyPositiveWithFallback,
              MaybeReflectDefault<uint32> DefaultValue, MaybeReflectRange<uint32> Range, ReflectTarget Target>
    KRYS_NODISCARD static uint32 ReflectUnsignedLong(const Target &target, dom::DOMStringAtom name) noexcept
    {
      static_assert(!(OnlyPositive && OnlyPositiveWithFallback),
                    "Cannot be both 'OnlyPositive' and 'OnlyPositiveWithFallback'.");

      // 1. Let contentAttributeValue be the result of running this's get the content attribute.
      auto contentAttributeValue = GetContentAttribute(target, name);

      // 2. Let minimum be 0.
      uint64 minimum = 0u;

      // 3. If the reflected IDL attribute is limited to only positive numbers or limited to only positive
      //    numbers with fallback, then set minimum to 1.
      if constexpr (OnlyPositive || OnlyPositiveWithFallback)
      {
        minimum = 1u;
      }

      // 4. If the reflected IDL attribute is clamped to the range, then set minimum to clampedMin.
      if constexpr (Range)
      {
        minimum = (*Range).ClampedMin;
      }

      // 5. Let maximum be 2147483647 if the reflected IDL attribute is not clamped to the range; otherwise
      //    clampedMax.
      uint64 maximum = MaxUnsignedLongValue;
      if constexpr (Range)
      {
        maximum = (*Range).ClampedMax;
      }

      // 6. If contentAttributeValue is not null:
      if (contentAttributeValue.has_value())
      {
        // 6a. Let parsedValue be the result of non-negative integer parsing contentAttributeValue.
        auto parsedValue = Numbers::ParseNonNegativeInteger(*contentAttributeValue);

        // 6b. If parsedValue is not an error and is in the range minimum to maximum, inclusive, then return
        //     parsedValue.
        if (parsedValue.Success() && parsedValue >= minimum && parsedValue <= maximum)
        {
          return static_cast<uint32>(parsedValue.Value);
        }

        // 6c. If parsedValue is not an error and the reflected IDL attribute is clamped to the range:
        // An integer too large for int64 is past the maximum too.
        if constexpr (Range)
        {
          if (parsedValue.Success() || parsedValue.Error == MicroParserError::OutOfRange)
          {
            // 6c.i. If parsedValue is less than minimum, then return minimum.
            // 6c.ii. Return maximum.
            return static_cast<uint32>(parsedValue < minimum ? minimum : maximum);
          }
        }
      }

      // 7. If the reflected IDL attribute has a default value, then return defaultValue.
      if constexpr (DefaultValue)
      {
        return *DefaultValue;
      }
      // 8. Return minimum.
      else
      {
        return static_cast<uint32>(minimum);
      }
    }

    /// @brief Helper for setting reflected content attributes with 'unsigned long' type.
    template <OnlyPositiveNumbers OnlyPositive, OnlyPositiveNumbersWithFallback OnlyPositiveWithFallback,
              MaybeReflectDefault<uint32> DefaultValue, ReflectTarget Target>
    KRYS_NODISCARD static conditional_t<OnlyPositive, dom::ExceptionOr<void>, void>
      ReflectUnsignedLong(Target &target, dom::DOMStringAtom name, uint32 value) noexcept
    {
      static_assert(!(OnlyPositive && OnlyPositiveWithFallback),
                    "Cannot be both 'OnlyPositive' and 'OnlyPositiveWithFallback'.");

      // 1. If the reflected IDL attribute is limited to only positive numbers and the given value is 0, then
      //    throw an "IndexSizeError" DOMException.
      if constexpr (OnlyPositive)
      {
        if (value == 0)
        {
          return dom::ExceptionCode::IndexSizeError;
        }
      }

      // 2. Let minimum be 0.
      uint32 minimum = 0u;

      // 3. If the reflected IDL attribute is limited to only positive numbers or limited to only positive
      //    numbers with fallback, then set minimum to 1.
      if constexpr (OnlyPositive || OnlyPositiveWithFallback)
      {
        minimum = 1u;
      }

      // 4. Let newValue be minimum.
      uint32 newValue = minimum;

      // 5. If the reflected IDL attribute has a default value, then set newValue to defaultValue.
      if constexpr (DefaultValue)
      {
        newValue = *DefaultValue;
      }

      // 6. If the given value is in the range minimum to 2147483647, inclusive, then set newValue to it.
      if (value >= minimum && value <= MaxUnsignedLongValue)
      {
        newValue = value;
      }

      // 7. Run this's set the content attribute with newValue converted to the shortest possible string
      //    representing the number as a valid non-negative integer.
      SetContentAttribute(target, name, IntegerToString(newValue));

      if constexpr (OnlyPositive)
      {
        return {};
      }
    }

#pragma endregion

#pragma region ReflectDouble

    /// @brief Helper for getting reflected content attributes with 'double' type.
    /// @see https://html.spec.whatwg.org/#clamped-to-the-range
    template <OnlyPositiveNumbers OnlyPositive, MaybeReflectDefault<double> DefaultValue,
              ReflectTarget Target>
    KRYS_NODISCARD static double ReflectDouble(const Target &target, dom::DOMStringAtom name) noexcept
    {
      // 1. Let contentAttributeValue be the result of running this's get the content attribute.
      auto contentAttributeValue = GetContentAttribute(target, name);

      // 2. If contentAttributeValue is not null:
      if (contentAttributeValue.has_value())
      {
        // 2a. Let parsedValue be the result of floating-point number parsing contentAttributeValue.
        auto parsedValue = Numbers::ParseFloatingPoint(*contentAttributeValue);
        if (parsedValue.Success())
        {
          // 2b. If parsedValue is not an error and is greater than 0, then return parsedValue.
          // 2c. If parsedValue is not an error and the reflected IDL attribute is not limited to only
          // positive
          //     numbers, then return parsedValue.
          if (!OnlyPositive || parsedValue > 0)
          {
            return parsedValue.Value;
          }
        }
      }

      // 3. If the reflected IDL attribute has a default value, then return defaultValue.
      if constexpr (DefaultValue)
      {
        return *DefaultValue;
      }

      // 4. Return 0.
      return 0;
    }

    /// @brief Helper for setting reflected content attributes with 'double' type.
    template <OnlyPositiveNumbers OnlyPositive, ReflectTarget Target>
    static void ReflectDouble(Target &target, dom::DOMStringAtom name, double value) noexcept
    {
      // 1. If the reflected IDL attribute is limited to only positive numbers and the given value is not
      //    greater than 0, then return.
      if constexpr (OnlyPositive)
      {
        if (!(value > 0))
        {
          return;
        }
      }

      // 2. Run this's set the content attribute with the given value, converted to the best representation of
      //    the number as a floating-point number.
      SetContentAttribute(target, name, SerializeForNumberType(value));
    }

#pragma endregion

    /// @brief @p value as the shortest valid integer.
    KRYS_NODISCARD static dom::DOMString IntegerToString(int64 value) noexcept
    {
      char buffer[24];
      auto end = std::to_chars(buffer, buffer + sizeof(buffer), value).ptr;
      return dom::DOMString(dom::DOMStringView(reinterpret_cast<const char8 *>(buffer), end - buffer));
    }
  };
}
