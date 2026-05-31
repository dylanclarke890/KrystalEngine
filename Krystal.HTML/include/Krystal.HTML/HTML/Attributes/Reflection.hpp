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

  // TODO(CONTENT-ATTRIBUTE-REFLECTION): FrozenArray<T>? is also one of the possible types.
  template <typename T>
  concept ReflectType =
    OneOf<T, DOMString, Maybe<DOMString>, USVString, bool, int32, uint32, double, DOMTokenList>;

  template <typename T>
  concept ReflectSetterType = ReflectType<T>;

  template <typename T>
  concept ReflectURLType = OneOf<T, USVString>;

  template <typename T>
  concept ReflectNonNegativeType = OneOf<T, int32>;

  template <typename T>
  concept ReflectPositiveType = OneOf<T, double, uint32>;

  template <typename T>
  concept ReflectPositiveWithFallbackType = ReflectPositiveType<T>;

  template <typename T>
  concept ReflectDefaultType = OneOf<T, double, int32, uint32>;

  template <typename T>
  concept ReflectRangeType = OneOf<T, uint32>;

  template <typename T>
  concept ReflectLimitedToKnownValuesType = OneOf<T, DOMString, Maybe<DOMString>>;

  template <ReflectRangeType T>
  struct ReflectRange
  {
    int32 ClampedMin {0};
    int32 ClampedMax {0};
  };

  template <ReflectDefaultType T>
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

  struct OnlyKnownValues : public StronglyTypedBool<OnlyKnownValues>
  {
    using Base::Base;
  };

  struct TreatedAsURL : public StronglyTypedBool<TreatedAsURL>
  {
    using Base::Base;
  };

  struct ReflectUnsignedLongOptions
  {
    Maybe<ReflectDefault<uint32>> DefaultValue {Null};
    Maybe<ReflectRange<uint32>> ClampedRange {Null};
    OnlyPositiveNumbers OnlyPositive {false};
    OnlyPositiveNumbersWithFallback OnlyPositiveWithFallback {false};
  };

  struct ReflectLongOptions
  {
    Maybe<ReflectDefault<int32>> DefaultValue {Null};
    OnlyNonNegativeNumbers OnlyNonNegative {false};
  };

  struct ReflectDoubleOptions
  {
    Maybe<ReflectDefault<double>> DefaultValue {Null};
    OnlyPositiveNumbers OnlyPositive {false};
  };

  /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#reflect
  class Reflection
  {
  public:
#pragma region Reflect

    /// @brief Represents the `[Reflect]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    template <ReflectType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue> Reflect(const Target &target, DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, DOMString>)
      {
        return ReflectDOMString(target, name);
      }
      else if constexpr (SameType<TValue, Maybe<DOMString>>)
      {
        return ReflectNullableDOMString(target, name);
      }
      else if constexpr (SameType<TValue, USVString>)
      {
        return ReflectUSVString(target, name, TreatedAsURL(false));
      }
      else if constexpr (SameType<TValue, bool>)
      {
        return ReflectBool(target, name);
      }
      else if constexpr (SameType<TValue, int32>)
      {
        return ReflectLong(target, name);
      }
      else if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong(target, name);
      }
      else if constexpr (SameType<TValue, double>)
      {
        return ReflectDouble(target, name);
      }
      else if constexpr (SameType<TValue, DOMTokenList>)
      {
        return ReflectDOMTokenList(target, name);
      }
    }

    /// @brief Represents the `[Reflect]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    template <ReflectType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> Reflect(Target &target, DOMStringAtom name,
                                                    TValue &&value) noexcept
    {
      if constexpr (SameType<TValue, DOMString>)
      {
        return ReflectDOMString(target, name, Krys::Move(value));
      }
      else if constexpr (SameType<TValue, Maybe<DOMString>>)
      {
        return ReflectNullableDOMString(target, name, Krys::Move(value));
      }
      else if constexpr (SameType<TValue, USVString>)
      {
        return ReflectUSVString(target, name, Krys::Move(value), TreatedAsURL(false));
      }
      else if constexpr (SameType<TValue, bool>)
      {
        return ReflectBool(target, name, value);
      }
      else if constexpr (SameType<TValue, int32>)
      {
        return ReflectLong(target, name, value);
      }
      else if constexpr (SameType<TValue, uint32>)
      {
        return ReflectUnsignedLong(target, name, value);
      }
      else if constexpr (SameType<TValue, double>)
      {
        return ReflectDouble(target, name, value);
      }
      else if constexpr (SameType<TValue, DOMTokenList>)
      {
        return ReflectDOMTokenList(target, name, value);
      }
    }

    /// @brief Represents the `[Reflect][ReflectDefault(defaultValue)]` combination of IDL attributes
    /// (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectDefaultType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue> Reflect(const Target &target, DOMStringAtom name,
                                                      ReflectDefault<TValue> defaultValue) noexcept
    {
      if constexpr (SameType<TValue, int32>)
      {
        ReflectLongOptions options {.DefaultValue = defaultValue};
        return ReflectLong(target, name, options);
      }
      else if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.DefaultValue = defaultValue};
        return ReflectUnsignedLong(target, name, options);
      }
      else if constexpr (SameType<TValue, double>)
      {
        ReflectDoubleOptions options {.DefaultValue = defaultValue};
        return ReflectDouble(target, name, options);
      }
    }

    /// @brief Represents the `[Reflect][ReflectDefault(defaultValue)]` combination of IDL attributes
    /// (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectDefaultType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> Reflect(Target &target, DOMStringAtom name, TValue value,
                                                    ReflectDefault<TValue> defaultValue) noexcept
    {
      if constexpr (SameType<TValue, int32>)
      {
        ReflectLongOptions options {.DefaultValue = defaultValue};
        return ReflectLong(target, name, value, options);
      }
      else if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.DefaultValue = defaultValue};
        return ReflectUnsignedLong(target, name, value, options);
      }
      else if constexpr (SameType<TValue, double>)
      {
        ReflectDoubleOptions options {.DefaultValue = defaultValue};
        return ReflectDouble(target, name, value, options);
      }
    }

    /// @brief Represents the `[Reflect][ReflectRange(range)]` combination of IDL attributes (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    /// @see https://html.spec.whatwg.org/#xattr-reflectrange
    template <ReflectRangeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue> Reflect(const Target &target, DOMStringAtom name,
                                                      ReflectRange<TValue> range) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.ClampedRange = range};
        return ReflectUnsignedLong(target, name, options);
      }
    }

    /// @brief Represents the `[Reflect][ReflectRange(range)]` combination of IDL attributes (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    /// @see https://html.spec.whatwg.org/#xattr-reflectrange
    template <ReflectRangeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> Reflect(Target &target, DOMStringAtom name, TValue value,
                                                    ReflectRange<TValue> range) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.ClampedRange = range};
        return ReflectUnsignedLong(target, name, value, options);
      }
    }

    /// @brief Represents the `[Reflect][ReflectDefault(defaultValue)][ReflectRange(range)]` combination of
    /// IDL attributes (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    /// @see https://html.spec.whatwg.org/#xattr-reflectrange
    template <ReflectRangeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue> Reflect(const Target &target, DOMStringAtom name,
                                                      ReflectRange<TValue> range,
                                                      ReflectDefault<TValue> defaultValue) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.DefaultValue = defaultValue, .ClampedRange = range};
        return ReflectUnsignedLong(target, name, options);
      }
    }

    /// @brief Represents the `[Reflect][ReflectDefault(defaultValue)][ReflectRange(range)]` combination of
    /// IDL attributes (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflect
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    /// @see https://html.spec.whatwg.org/#xattr-reflectrange
    template <ReflectRangeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> Reflect(Target &target, DOMStringAtom name, TValue value,
                                                    ReflectRange<TValue> range,
                                                    ReflectDefault<TValue> defaultValue) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.DefaultValue = defaultValue, .ClampedRange = range};
        return ReflectUnsignedLong(target, name, value, options);
      }
    }

#pragma endregion

#pragma region ReflectSetter

    /// @brief Represents the `[ReflectSetter]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectsetter
    template <ReflectSetterType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectSetter(Target &target, DOMStringAtom name,
                                                          TValue &&value) noexcept
    {
      return Reflect(target, name, Krys::Move(value));
    }

#pragma endregion

#pragma region ReflectURL

    /// @brief Represents the `[ReflectURL]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflecturl
    template <ReflectURLType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue> ReflectURL(const Target &target, DOMStringAtom name) noexcept
    {
      return ReflectUSVString(target, name, TreatedAsURL(true));
    }

    /// @brief Represents the `[ReflectURL]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflecturl
    template <ReflectURLType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectURL(Target &target, DOMStringAtom name,
                                                       TValue &&value) noexcept
    {
      return ReflectUSVString(target, name, Krys::Move(value), TreatedAsURL(true));
    }

#pragma endregion

#pragma region ReflectNonNegative

    /// @brief Represents the `[ReflectNonNegative]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectnonnegative
    template <ReflectNonNegativeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue> ReflectNonNegative(const Target &target,
                                                                 DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, int32>)
      {
        ReflectLongOptions options {.OnlyNonNegative = OnlyNonNegativeNumbers {true}};
        return ReflectLong(target, name, options);
      }
    }

    /// @brief Represents the `[ReflectNonNegative]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectnonnegative
    template <ReflectNonNegativeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectNonNegative(Target &target, DOMStringAtom name,
                                                               TValue value) noexcept
    {
      if constexpr (SameType<TValue, int32>)
      {
        ReflectLongOptions options {.OnlyNonNegative = OnlyNonNegativeNumbers {true}};
        return ReflectLong(target, name, value, options);
      }
    }

    /// @brief Represents the `[ReflectNonNegative][ReflectDefault(defaultValue)]` combination of IDL
    /// attributes (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectnonnegative
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectNonNegativeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue> ReflectNonNegative(const Target &target, DOMStringAtom name,
                                                                 ReflectDefault<TValue> defaultValue) noexcept
    {
      if constexpr (SameType<TValue, int32>)
      {
        ReflectLongOptions options {.DefaultValue = defaultValue,
                                    .OnlyNonNegative = OnlyNonNegativeNumbers {true}};
        return ReflectLong(target, name, options);
      }
    }

    /// @brief Represents the `[ReflectNonNegative][ReflectDefault(defaultValue)]` combination of IDL
    /// attributes (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectnonnegative
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectNonNegativeType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectNonNegative(Target &target, DOMStringAtom name,
                                                               TValue value,
                                                               ReflectDefault<TValue> defaultValue) noexcept
    {
      ReflectLongOptions options {.DefaultValue = defaultValue,
                                  .OnlyNonNegative = OnlyNonNegativeNumbers {true}};
      return ReflectLong(target, name, value, options);
    }

#pragma endregion

#pragma region ReflectPositive

    /// @brief Represents the `[ReflectPositive]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositive
    template <ReflectPositiveType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue> ReflectPositive(const Target &target,
                                                              DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectUnsignedLong(target, name, options);
      }
      else if constexpr (SameType<TValue, double>)
      {
        ReflectDoubleOptions options {.OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectDouble(target, name, options);
      }
    }

    /// @brief Represents the `[ReflectPositive]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositive
    template <ReflectPositiveType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectPositive(Target &target, DOMStringAtom name,
                                                            TValue value) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectUnsignedLong(target, name, value, options);
      }
      else if constexpr (SameType<TValue, double>)
      {
        ReflectDoubleOptions options {.OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectDouble(target, name, value, options);
      }
    }

    /// @brief Represents the `[ReflectPositive][ReflectDefault(defaultValue)]` combination of IDL
    /// attributes (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositive
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectPositiveType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue> ReflectPositive(const Target &target, DOMStringAtom name,
                                                              ReflectDefault<TValue> defaultValue) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.DefaultValue = defaultValue,
                                            .OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectUnsignedLong(target, name, options);
      }
      else if constexpr (SameType<TValue, double>)
      {
        ReflectDoubleOptions options {.DefaultValue = defaultValue,
                                      .OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectDouble(target, name, options);
      }
    }

    /// @brief Represents the `[ReflectPositive][ReflectDefault(defaultValue)]` combination of IDL
    /// attributes (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositive
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectPositiveType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectPositive(Target &target, DOMStringAtom name, TValue value,
                                                            ReflectDefault<TValue> defaultValue) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.DefaultValue = defaultValue,
                                            .OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectUnsignedLong(target, name, value, options);
      }
      else if constexpr (SameType<TValue, double>)
      {
        ReflectDoubleOptions options {.DefaultValue = defaultValue,
                                      .OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectDouble(target, name, value, options);
      }
    }

#pragma endregion

#pragma region ReflectPositiveWithFallback

    /// @brief Represents the `[ReflectPositiveWithFallback]` IDL attribute (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositivewithfallback
    template <ReflectPositiveWithFallbackType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue> ReflectPositiveWithFallback(const Target &target,
                                                                          DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.OnlyPositiveWithFallback =
                                              OnlyPositiveNumbersWithFallback {true}};
        return ReflectUnsignedLong(target, name, options);
      }
      else if constexpr (SameType<TValue, double>)
      {
        ReflectDoubleOptions options {.OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectDouble(target, name, options);
      }
    }

    /// @brief Represents the `[ReflectPositiveWithFallback]` IDL attribute (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositivewithfallback
    template <ReflectPositiveWithFallbackType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectPositiveWithFallback(Target &target, DOMStringAtom name,
                                                                        TValue value) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {.OnlyPositiveWithFallback =
                                              OnlyPositiveNumbersWithFallback {true}};
        return ReflectUnsignedLong(target, name, value, options);
      }
      else if constexpr (SameType<TValue, double>)
      {
        ReflectDoubleOptions options {.OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectDouble(target, name, value, options);
      }
    }

    /// @brief Represents the `[ReflectPositiveWithFallback][ReflectDefault(defaultValue)]` combination of IDL
    /// attributes (getter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositivewithfallback
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectPositiveWithFallbackType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<TValue>
      ReflectPositiveWithFallback(const Target &target, DOMStringAtom name,
                                  ReflectDefault<TValue> defaultValue) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {
          .DefaultValue = defaultValue, .OnlyPositiveWithFallback = OnlyPositiveNumbersWithFallback {true}};
        return ReflectUnsignedLong(target, name, options);
      }
      else if constexpr (SameType<TValue, double>)
      {
        ReflectDoubleOptions options {.DefaultValue = defaultValue,
                                      .OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectDouble(target, name, options);
      }
    }

    /// @brief Represents the `[ReflectPositiveWithFallback][ReflectDefault(defaultValue)]` combination of IDL
    /// attributes (setter).
    /// @see https://html.spec.whatwg.org/#xattr-reflectpositivewithfallback
    /// @see https://html.spec.whatwg.org/#xattr-reflectdefault
    template <ReflectPositiveWithFallbackType TValue, ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void>
      ReflectPositiveWithFallback(Target &target, DOMStringAtom name, TValue value,
                                  ReflectDefault<TValue> defaultValue) noexcept
    {
      if constexpr (SameType<TValue, uint32>)
      {
        ReflectUnsignedLongOptions options {
          .DefaultValue = defaultValue, .OnlyPositiveWithFallback = OnlyPositiveNumbersWithFallback {true}};
        return ReflectUnsignedLong(target, name, value, options);
      }
      else if constexpr (SameType<TValue, double>)
      {
        // NOTE: The spec says this attribute can be used for 'double' but doesn't actually handle the case
        // for 'OnlyPositiveWithFallback' for 'double' reflected attributes. Here we assume it has the same
        // behavior as 'OnlyPositive' for 'double'.

        ReflectDoubleOptions options {.DefaultValue = defaultValue,
                                      .OnlyPositive = OnlyPositiveNumbers {true}};
        return ReflectDouble(target, name, value, options);
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

    /// @brief Helper for getting reflected content attributes with 'DOMString' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<DOMString> ReflectDOMString(Target &target, DOMStringAtom name,
                                                                  OnlyKnownValues onlyKnownValues) noexcept
    {
      // TODO(CONTENT-ATTRIBUTE-REFLECTION)
      return ExceptionCode::NotSupportedError;
    }

    /// @brief Helper for setting reflected content attributes with 'DOMString' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectDOMString(const Target &target, DOMStringAtom name,
                                                             DOMString &&value,
                                                             OnlyKnownValues onlyKnownValues) noexcept
    {
      SetContentAttribute(target, name, Krys::Move(*value));
      return {};
    }

    /// @brief Helper for getting reflected content attributes with 'DOMString?' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<Maybe<DOMString>>
      ReflectNullableDOMString(Target &target, DOMStringAtom name, OnlyKnownValues onlyKnownValues) noexcept
    {
      // TODO(CONTENT-ATTRIBUTE-REFLECTION)
      return ExceptionCode::NotSupportedError;
    }

    /// @brief Helper for setting reflected content attributes with 'DOMString?' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectNullableDOMString(const Target &target, DOMStringAtom name,
                                                                     Maybe<DOMString> &&value,
                                                                     OnlyKnownValues onlyKnownValues) noexcept
    {
      if (!value.has_value())
      {
        DeleteContentAttribute(target, name);
      }
      else
      {
        SetContentAttribute(target, name, Krys::Move(*value));
      }

      return {};
    }

#pragma endregion

#pragma region ReflectUSVString

    /// @brief Helper for getting reflected content attributes with 'USVString' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<USVString> ReflectUSVString(Target &target, DOMStringAtom name,
                                                                  TreatedAsURL treatedAsURL) noexcept
    {
      // SPEC-VIOLATION(USVString): Not supported.
      return ExceptionCode::NotSupportedError;
    }

    /// @brief Helper for setting reflected content attributes with 'USVString' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectUSVString(const Target &target, DOMStringAtom name,
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
    KRYS_NODISCARD static ExceptionOr<bool> ReflectBool(Target &target, DOMStringAtom name) noexcept
    {
      auto contentAttributeValue = GetContentAttribute(target, name);

      if (!contentAttributeValue.has_value())
      {
        return false;
      }

      return true;
    }

    /// @brief Helper for setting reflected content attributes with 'bool' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectBool(const Target &target, DOMStringAtom name,
                                                        bool value) noexcept
    {
      if (!value)
      {
        DeleteContentAttribute(target, name);
      }
      else
      {
        SetContentAttribute(target, name, DOMString {});
      }

      return {};
    }

#pragma endregion

#pragma region ReflectLong

    /// @brief Helper for getting reflected content attributes with 'long' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<int32> ReflectLong(Target &target, DOMStringAtom name,
                                                         ReflectLongOptions options = {}) noexcept
    {
      auto contentAttributeValue = GetContentAttribute(target, name);
      if (contentAttributeValue.has_value())
      {
        if (options.OnlyNonNegative)
        {
          auto parsedValue = MicroParsers::Numbers::ParseNonNegativeInteger(*contentAttributeValue);
          if (parsedValue.Success() && parsedValue <= std::numeric_limits<int32>::max())
          {
            return static_cast<int32>(parsedValue);
          }
        }
        else
        {
          auto parsedValue = MicroParsers::Numbers::ParseInteger(*contentAttributeValue);
          if (parsedValue.Success() && parsedValue <= std::numeric_limits<int32>::max())
          {
            return static_cast<int32>(parsedValue);
          }
        }
      }

      if (options.DefaultValue.has_value())
      {
        return *options.DefaultValue;
      }

      if (options.OnlyNonNegative)
      {
        return -1;
      }

      return 0;
    }

    /// @brief Helper for setting reflected content attributes with 'long' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectLong(Target &target, DOMStringAtom name, int32 value,
                                                        ReflectLongOptions options = {}) noexcept
    {
      if (options.OnlyNonNegative && value < 0)
      {
        return ExceptionCode::IndexSizeError;
      }

      SetContentAttribute(target, name, std::to_string(value));

      return {};
    }

#pragma endregion

#pragma region ReflectUnsignedLong

    constexpr static int32 MaxUnsignedLongValue {2'147'483'647};

    /// @brief Helper for getting reflected content attributes with 'unsigned long' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<uint32>
      ReflectUnsignedLong(const Target &target, DOMStringAtom name,
                          ReflectUnsignedLongOptions options = {}) noexcept
    {
      assert(!(onlyPositiveNumbers && onlyPositiveNumbersWithFallback));

      auto contentAttributeValue = GetContentAttribute(target, name);

      uint32 minimum = 0u;

      if (options.OnlyPositive || options.OnlyPositiveWithFallback)
      {
        minimum = 1u;
      }

      if (options.ClampedRange.has_value())
      {
        minimum = options.ClampedRange->ClampedMin;
      }

      uint32 maximum = MaxUnsignedLongValue;
      if (options.ClampedRange.has_value())
      {
        maximum = options.ClampedRange->ClampedMax;
      }

      if (contentAttributeValue.has_value())
      {
        auto parsedValue = MicroParsers::Numbers::ParseNonNegativeInteger(*contentAttributeValue);
        if (parsedValue.Success())
        {
          if (parsedValue >= minimum && parsedValue <= maximum)
          {
            return parsedValue;
          }

          if (options.ClampedRange.has_value())
          {
            return std::clamp(parsedValue.Value, minimum, maximum);
          }
        }
      }

      if (options.DefaultValue.has_value())
      {
        return *options.DefaultValue;
      }

      return minimum;
    }

    /// @brief Helper for setting reflected content attributes with 'unsigned long' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void>
      ReflectUnsignedLong(Target &target, DOMStringAtom name, uint32 value,
                          ReflectUnsignedLongOptions options = {}) noexcept
    {
      assert(!(options.OnlyPositive && options.OnlyPositiveWithFallback));

      if (options.OnlyPositive && value == 0)
      {
        return ExceptionCode::IndexSizeError;
      }

      uint32 minimum = 0u;
      if (options.OnlyPositive || options.OnlyPositiveWithFallback)
      {
        minimum = 1u;
      }

      uint32 newValue = minimum;
      if (options.DefaultValue.has_value())
      {
        newValue = *options.DefaultValue;
      }

      if (value >= minimum && value <= MaxUnsignedLongValue)
      {
        newValue = value;
      }

      SetContentAttribute(target, name, std::to_string(newValue));
      return {};
    }

#pragma endregion

#pragma region ReflectDouble

    /// @brief Helper for getting reflected content attributes with 'double' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<double> ReflectDouble(Target &target, DOMStringAtom name,
                                                            ReflectDoubleOptions options = {}) noexcept
    {
      auto contentAttributeValue = GetContentAttribute(target, name);

      if (contentAttributeValue.has_value())
      {
        auto parsedValue = MicroParsers::Numbers::ParseFloatingPoint(*contentAttributeValue);
        if (parsedValue.Success())
        {
        }
      }

      if (options.DefaultValue.has_value())
      {
        return *options.DefaultValue;
      }

      return minimum;
    }

    /// @brief Helper for setting reflected content attributes with 'double' type.
    template <ReflectTarget Target>
    KRYS_NODISCARD static ExceptionOr<void> ReflectDouble(const Target &target, DOMStringAtom name,
                                                          double value,
                                                          ReflectDoubleOptions options = {}) noexcept
    {
      if (options.OnlyPositive && value <= 0)
      {
        return {}; // Not an error as per the spec.
      }

      SetContentAttribute(target, name, std::to_string(value));
      return {};
    }

#pragma endregion

#pragma region ReflectDOMTokenList
    // NOTE: Can only reflect 'HTMLElement' target as per the spec.

    /// @brief Helper for getting reflected content attributes with 'DOMTokenList' type.
    KRYS_NODISCARD static ExceptionOr<DOMTokenList &> ReflectDOMTokenList(HTMLElement &target,
                                                                          DOMStringAtom name) noexcept
    {
      // TODO(CONTENT-ATTRIBUTE-REFLECTION)
      return ExceptionCode::NotSupportedError;
    }

    /// @brief Helper for setting reflected content attributes with 'DOMTokenList' type.
    KRYS_NODISCARD static ExceptionOr<void> ReflectDOMTokenList(const HTMLElement &target,
                                                                DOMTokenList &value) noexcept
    {
      // TODO(CONTENT-ATTRIBUTE-REFLECTION)
      return ExceptionCode::NotSupportedError;
    }

#pragma endregion
  };
}