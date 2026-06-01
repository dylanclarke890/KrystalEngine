#pragma once

#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/HTML/ElementInternals.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/USVString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/String/FixedString.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML::Attributes
{
  template <typename T>
  concept ReflectTarget = DerivedFrom<T, HTMLElement> || SameType<T, ElementInternals>;

  template <typename T>
  concept ReflectTypeDOMString = OneOf<T, DOMString, Maybe<DOMString>>;

  template <typename T>
  concept ReflectURLType = OneOf<T, USVString>;

  // TODO(CONTENT-ATTRIBUTE-REFLECTION): FrozenArray<T>? is also one of the possible types.
  template <typename T>
  concept ReflectType =
    ReflectTypeDOMString<T> || ReflectURLType<T> || OneOf<T, bool, int32, uint32, double, DOMTokenList>;

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

  /// @brief Types that unconditionally return exceptions during reflection.
  template <typename T>
  concept ReflectAlwaysReturnsExceptionOrT = OneOf<T, USVString, DOMTokenList>;

  template <typename T>
  using reflect_get_return_t =
    conditional_t<SameType<T, DOMString>, Maybe<DOMString>,
                  conditional_t<ReflectAlwaysReturnsExceptionOrT<T>, ExceptionOr<T>, T>>;

  template <typename T, OnlyNonNegativeNumbers OnlyNonNegative, OnlyPositiveNumbers OnlyPositive>
  using reflect_set_return_t =
    conditional_t<ReflectAlwaysReturnsExceptionOrT<T> || (SameType<T, int32> && OnlyNonNegative)
                    || (SameType<T, uint32> && OnlyPositive),
                  ExceptionOr<void>, void>;

  template <typename T>
  struct MaybeReflectParameter
  {
    T Parameter;
    bool HasParameter {false};

    MaybeReflectParameter() noexcept = default;

    constexpr MaybeReflectParameter(T parameter) noexcept : Parameter(parameter), HasParameter(true)
    {
    }

    KRYS_NODISCARD constexpr operator bool() const noexcept
    {
      return HasParameter;
    }

    KRYS_NODISCARD constexpr T operator*() const noexcept
    {
      return Parameter;
    }
  };

  template <typename T>
  using MaybeReflectDefault = MaybeReflectParameter<ReflectDefault<T>>;

  template <typename T>
  using MaybeReflectRange = MaybeReflectParameter<ReflectRange<T>>;

  template <typename T>
  constexpr inline MaybeReflectDefault<T> NoDefaultValue = {};

  template <typename T>
  constexpr inline MaybeReflectRange<T> NoRange = {};

  /// @brief Implements the logic for reflecting content attributes to IDL attributes and vice versa.
  /// Enumerated attributes and attributes that are limited to a set of known values
  /// (DOMString/Maybe<DOMString>) are not handled by the generic Reflect functions and need to be handled
  /// separately by the caller. This includes 'missing', 'default' and 'invalid' value handling.
  /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#reflect
  class Reflection
  {
  public:
#pragma region Reflect

    /// @brief Represents the `[Reflect]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    template <ReflectType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> Reflect(const Target &target,
                                                               DOMStringAtom name) noexcept
    {
      if constexpr (OneOf<TValue, DOMString, Maybe<DOMString>>)
      {
        return ReflectDOMString(target, name);
      }
      else if constexpr (SameType<TValue, USVString>)
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
      else if constexpr (SameType<TValue, DOMTokenList>)
      {
        return ReflectDOMTokenList(target, name);
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
      Reflect(Target &target, DOMStringAtom name, TValue &&value) noexcept
    {
      if constexpr (OneOf<TValue, DOMString, Maybe<DOMString>>)
      {
        ReflectDOMString(target, name, Krys::Move(value));
      }
      else if constexpr (SameType<TValue, USVString>)
      {
        return ReflectUSVString<TreatedAsURL(false)>(target, name, Krys::Move(value));
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
      else if constexpr (SameType<TValue, DOMTokenList>)
      {
        return ReflectDOMTokenList(target, name, value);
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
                                                               DOMStringAtom name) noexcept
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
      Reflect(Target &target, DOMStringAtom name, TValue value) noexcept
    {
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
                                                               DOMStringAtom name) noexcept
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
                                                               DOMStringAtom name) noexcept
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
      ReflectSetter(Target &target, DOMStringAtom name, TValue &&value) noexcept
    {
      return Reflect(target, name, Krys::Move(value));
    }

#pragma endregion

#pragma region ReflectURL

    /// @brief Represents the `[ReflectURL]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflecturl
    template <ReflectURLType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> ReflectURL(const Target &target,
                                                                  DOMStringAtom name) noexcept
    {
      return ReflectUSVString<TreatedAsURL(true)>(target, name);
    }

    /// @brief Represents the `[ReflectURL]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflecturl
    template <ReflectURLType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<TValue, OnlyNonNegativeNumbers(false),
                                               OnlyPositiveNumbers(false)>
      ReflectURL(Target &target, DOMStringAtom name, TValue &&value) noexcept
    {
      return ReflectUSVString<TreatedAsURL(true)>(target, name, Krys::Move(value));
    }

#pragma endregion

#pragma region ReflectNonNegative

    /// @brief Represents the `[ReflectNonNegative]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectnonnegative
    template <ReflectNonNegativeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static reflect_get_return_t<TValue> ReflectNonNegative(const Target &target,
                                                                          DOMStringAtom name) noexcept
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
      ReflectNonNegative(Target &target, DOMStringAtom name, TValue value) noexcept
    {
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
                                                                          DOMStringAtom name) noexcept
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
                                                                       DOMStringAtom name) noexcept
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
      ReflectPositive(Target &target, DOMStringAtom name, TValue value) noexcept
    {
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
                                                                       DOMStringAtom name) noexcept
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
      ReflectPositive(Target &target, DOMStringAtom name, TValue value) noexcept
    {
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
      ReflectPositiveWithFallback(const Target &target, DOMStringAtom name) noexcept
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
      ReflectPositiveWithFallback(Target &target, DOMStringAtom name, TValue value) noexcept
    {
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
      ReflectPositiveWithFallback(const Target &target, DOMStringAtom name) noexcept
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
      ReflectPositiveWithFallback(Target &target, DOMStringAtom name, TValue value) noexcept
    {
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
    template <DerivedFrom<HTMLElement> TElement>
    KRYS_NODISCARD static RawPtr<TElement> GetElement(TElement &element) noexcept
    {
      return &element;
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-content-attribute
    KRYS_NODISCARD static Maybe<DOMString> GetContentAttribute(const HTMLElement &element,
                                                               DOMStringAtom name) noexcept
    {
      auto attribute = ElementAlgorithms::GetAttributeByNamespace(DOMStringAtom::Null(), name, element);
      if (attribute == nullptr)
      {
        return Null;
      }

      return attribute->Value();
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#set-the-content-attribute
    static void SetContentAttribute(HTMLElement &element, DOMStringAtom name, DOMString &&value) noexcept
    {
      ElementAlgorithms::SetAttributeValue(element, name, Krys::Move(value));
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#delete-the-content-attribute
    static void DeleteContentAttribute(HTMLElement &element, DOMStringAtom name) noexcept
    {
      ElementAlgorithms::RemoveAttributeByNamespace(DOMStringAtom::Null(), name, element);
    }

#pragma endregion

#pragma region ElementInternals ReflectTarget

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-element
    KRYS_NODISCARD static RawPtr<HTMLElement> GetElement(ElementInternals &internals) noexcept
    {
      return internals.TargetElement();
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-content-attribute
    KRYS_NODISCARD static Maybe<DOMString> GetContentAttribute(const ElementInternals &internals,
                                                               DOMStringAtom name) noexcept
    {
      // TODO(impl): ELEMENT-INTERNALS
      return Null;
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#set-the-content-attribute
    static void SetContentAttribute(ElementInternals &internals, DOMStringAtom name,
                                    DOMString &&value) noexcept
    {
      // TODO(impl): ELEMENT-INTERNALS
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#delete-the-content-attribute
    static void DeleteContentAttribute(ElementInternals &internals, DOMStringAtom name) noexcept
    {
      // TODO(impl): ELEMENT-INTERNALS
    }

#pragma endregion

#pragma region ReflectDOMString

    /// @brief Helper for getting reflected content attributes for 'DOMString/DOMString?' types. The handling
    /// for returning an empty string for null attributes is handled by the caller.
    template <ReflectTarget Target>
    KRYS_NODISCARD static Maybe<DOMString> ReflectDOMString(const Target &target, DOMStringAtom name) noexcept
    {
      return GetContentAttribute(target, name);
    }

    /// @brief Helper for setting reflected content attributes with 'DOMString?' type.
    template <ReflectTarget Target>
    static void ReflectDOMString(Target &target, DOMStringAtom name, Maybe<DOMString> &&value) noexcept
    {
      if (!value.has_value())
      {
        DeleteContentAttribute(target, name);
      }
      else
      {
        SetContentAttribute(target, name, Krys::Move(*value));
      }
    }

    /// @brief Helper for setting reflected content attributes with 'DOMString' type.
    template <ReflectTarget Target>
    static void ReflectDOMString(Target &target, DOMStringAtom name, DOMString &&value) noexcept
    {
      SetContentAttribute(target, name, Krys::Move(value));
    }

#pragma endregion

#pragma region ReflectUSVString

    /// @brief Helper for getting reflected content attributes with 'USVString' type.
    template <TreatedAsURL AsUrl, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<USVString> ReflectUSVString(const Target &target,
                                                                  DOMStringAtom name) noexcept
    {
      // SPEC-VIOLATION(USVString): Not supported.
      return ExceptionCode::NotSupportedError;
    }

    /// @brief Helper for setting reflected content attributes with 'USVString' type.
    template <TreatedAsURL AsUrl, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectUSVString(Target &target, DOMStringAtom name,
                                                             USVString &&value,
                                                             TreatedAsURL treatedAsURL) noexcept
    {
      // SPEC-VIOLATION(USVString): Not supported.
      return ExceptionCode::NotSupportedError;
    }

#pragma endregion

#pragma region ReflectBool

    /// @brief Helper for getting reflected content attributes with 'bool' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static bool ReflectBool(const Target &target, DOMStringAtom name) noexcept
    {
      auto contentAttributeValue = GetContentAttribute(target, name);
      return contentAttributeValue.has_value();
    }

    /// @brief Helper for setting reflected content attributes with 'bool' type.
    template <ReflectTarget Target>
    static void ReflectBool(Target &target, DOMStringAtom name, bool value) noexcept
    {
      if (!value)
      {
        DeleteContentAttribute(target, name);
      }
      else
      {
        SetContentAttribute(target, name, DOMString {});
      }
    }

#pragma endregion

#pragma region ReflectLong

    /// @brief Helper for getting reflected content attributes with 'long' type.
    template <OnlyNonNegativeNumbers OnlyNonNegative, MaybeReflectDefault<int32> DefaultValue,
              ReflectTarget Target>
    KRYS_NODISCARD static int32 ReflectLong(const Target &target, DOMStringAtom name) noexcept
    {
      auto contentAttributeValue = GetContentAttribute(target, name);
      if (contentAttributeValue.has_value())
      {
        if constexpr (OnlyNonNegative)
        {
          auto parsedValue = MicroParsers::Numbers::ParseNonNegativeInteger(*contentAttributeValue);
          if (parsedValue.Success() && parsedValue.Value <= std::numeric_limits<int32>::max())
          {
            return static_cast<int32>(parsedValue.Value);
          }
        }
        else
        {
          auto parsedValue = MicroParsers::Numbers::ParseInteger(*contentAttributeValue);
          if (parsedValue.Success() && parsedValue.Value <= std::numeric_limits<int32>::max())
          {
            return static_cast<int32>(parsedValue.Value);
          }
        }
      }

      if constexpr (DefaultValue)
      {
        return *DefaultValue;
      }
      else if constexpr (OnlyNonNegative)
      {
        return -1;
      }

      return 0;
    }

    /// @brief Helper for setting reflected content attributes with 'long' type.
    template <OnlyNonNegativeNumbers OnlyNonNegative, ReflectTarget Target>
    KRYS_NODISCARD static reflect_set_return_t<int32, OnlyNonNegative, OnlyPositiveNumbers(false)>
      ReflectLong(Target &target, DOMStringAtom name, int32 value) noexcept
    {
      if constexpr (OnlyNonNegative)
      {
        if (value < 0)
        {
          return ExceptionCode::IndexSizeError;
        }
      }

      auto newValueStr = std::to_string(value);
      DOMStringView u8view(reinterpret_cast<const char8 *>(newValueStr.data()), newValueStr.size());
      SetContentAttribute(target, name, DOMString(u8view));

      if constexpr (OnlyNonNegative)
      {
        return {};
      }
    }

#pragma endregion

#pragma region ReflectUnsignedLong

    constexpr static int32 MaxUnsignedLongValue {2'147'483'647};

    /// @brief Helper for getting reflected content attributes with 'unsigned long' type.
    template <OnlyPositiveNumbers OnlyPositive, OnlyPositiveNumbersWithFallback OnlyPositiveWithFallback,
              MaybeReflectDefault<uint32> DefaultValue, MaybeReflectRange<uint32> Range, ReflectTarget Target>
    KRYS_NODISCARD static uint32 ReflectUnsignedLong(const Target &target, DOMStringAtom name) noexcept
    {
      static_assert(!(OnlyPositive && OnlyPositiveWithFallback),
                    "Cannot be both 'OnlyPositive' and 'OnlyPositiveWithFallback'.");

      auto contentAttributeValue = GetContentAttribute(target, name);

      uint64 minimum = 0u;

      if constexpr (OnlyPositive || OnlyPositiveWithFallback)
      {
        minimum = 1u;
      }

      if (Range)
      {
        minimum = (*Range).ClampedMin;
      }

      uint64 maximum = MaxUnsignedLongValue;
      if (Range)
      {
        maximum = (*Range).ClampedMax;
      }

      if (contentAttributeValue.has_value())
      {
        auto parsedValue = MicroParsers::Numbers::ParseNonNegativeInteger(*contentAttributeValue);
        if (parsedValue.Success())
        {
          if (parsedValue >= minimum && parsedValue <= maximum)
          {
            return static_cast<uint32>(parsedValue.Value);
          }

          if (Range)
          {
            return static_cast<uint32>(std::clamp(parsedValue.Value, minimum, maximum));
          }
        }
      }

      if constexpr (DefaultValue)
      {
        return *DefaultValue;
      }

      return static_cast<uint32>(minimum);
    }

    /// @brief Helper for setting reflected content attributes with 'unsigned long' type.
    template <OnlyPositiveNumbers OnlyPositive, OnlyPositiveNumbersWithFallback OnlyPositiveWithFallback,
              MaybeReflectDefault<uint32> DefaultValue, ReflectTarget Target>
    KRYS_NODISCARD static conditional_t<OnlyPositive, ExceptionOr<void>, void>
      ReflectUnsignedLong(Target &target, DOMStringAtom name, uint32 value) noexcept
    {
      static_assert(!(OnlyPositive && OnlyPositiveWithFallback),
                    "Cannot be both 'OnlyPositive' and 'OnlyPositiveWithFallback'.");

      if constexpr (OnlyPositive)
      {
        if (value == 0)
        {
          return ExceptionCode::IndexSizeError;
        }
      }

      uint32 minimum = 0u;
      if constexpr (OnlyPositive || OnlyPositiveWithFallback)
      {
        minimum = 1u;
      }

      uint32 newValue = minimum;
      if constexpr (DefaultValue)
      {
        newValue = *DefaultValue;
      }

      if (value >= minimum && value <= MaxUnsignedLongValue)
      {
        newValue = value;
      }

      auto newValueStr = std::to_string(value);
      DOMStringView u8view(reinterpret_cast<const char8 *>(newValueStr.data()), newValueStr.size());
      SetContentAttribute(target, name, DOMString(u8view));

      if constexpr (OnlyPositive)
      {
        return {};
      }
    }

#pragma endregion

#pragma region ReflectDouble

    /// @brief Helper for getting reflected content attributes with 'double' type.
    template <OnlyPositiveNumbers OnlyPositive, MaybeReflectDefault<double> DefaultValue,
              ReflectTarget Target>
    KRYS_NODISCARD static double ReflectDouble(const Target &target, DOMStringAtom name) noexcept
    {
      auto contentAttributeValue = GetContentAttribute(target, name);
      if (contentAttributeValue.has_value())
      {
        auto parsedValue = MicroParsers::Numbers::ParseFloatingPoint(*contentAttributeValue);
        if (parsedValue.Success())
        {
          if constexpr (!OnlyPositive)
          {
            return parsedValue.Value;
          }
          else if (parsedValue > 0)
          {
            return parsedValue.Value;
          }
        }
      }

      if constexpr (DefaultValue)
      {
        return *DefaultValue;
      }

      return 0;
    }

    /// @brief Helper for setting reflected content attributes with 'double' type.
    template <OnlyPositiveNumbers OnlyPositive, ReflectTarget Target>
    static void ReflectDouble(Target &target, DOMStringAtom name, double value) noexcept
    {
      if constexpr (OnlyPositive)
      {
        if (value <= 0)
        {
          return; // Not an exception as per the spec.
        }
      }

      char buffer[64] {};
      auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
      if (ec != std::errc())
      {
        return;
      }

      DOMStringView u8view(reinterpret_cast<const char8 *>(buffer), ptr - buffer);
      SetContentAttribute(target, name, DOMString(u8view));
    }

#pragma endregion

#pragma region ReflectDOMTokenList

    // NOTE: These methods can only reflect 'HTMLElement' target as per the spec.

    /// @brief Helper for getting reflected content attributes with 'DOMTokenList' type.
    KRYS_NODISCARD static ExceptionOr<DOMTokenList &> ReflectDOMTokenList(const HTMLElement &target,
                                                                          DOMStringAtom name) noexcept
    {
      // TODO(CONTENT-ATTRIBUTE-REFLECTION)
      return ExceptionCode::NotSupportedError;
    }

    /// @brief Helper for setting reflected content attributes with 'DOMTokenList' type.
    KRYS_NODISCARD static ExceptionOr<void> ReflectDOMTokenList(HTMLElement &target,
                                                                DOMTokenList &value) noexcept
    {
      // TODO(CONTENT-ATTRIBUTE-REFLECTION)
      return ExceptionCode::NotSupportedError;
    }

#pragma endregion
  };
}