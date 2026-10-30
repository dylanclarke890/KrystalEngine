#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'hanging-punctuation'> = none | [ first || [ force-end | allow-end ] || last ]
  // https://drafts.csswg.org/css-text/#propdef-hanging-punctuation

  enum class HangingPunctuationValue : uint8
  {
    First,
    ForceEnd,
    AllowEnd,
    Last,
  };

  using HangingPunctuationValueEnumSet = css::SpaceSeparatedEnumSet<HangingPunctuationValue>;

  struct HangingPunctuation
  {
    using EnumSet = HangingPunctuationValueEnumSet;
    using value_type = HangingPunctuationValueEnumSet::value_type;

    constexpr HangingPunctuation(css::keywords::None) noexcept : _value {}
    {
    }
    constexpr HangingPunctuation(EnumSet &&set) noexcept : _value {krys::move(set)}
    {
    }
    constexpr HangingPunctuation(value_type value) noexcept : HangingPunctuation {EnumSet {value}}
    {
    }
    constexpr HangingPunctuation(std::initializer_list<value_type> initializerList) noexcept
        : HangingPunctuation {EnumSet {initializerList}}
    {
    }

    KRYS_NODISCARD constexpr static HangingPunctuation FromRaw(EnumSet::StorageType rawValue) noexcept
    {
      return EnumSet::FromRaw(rawValue);
    }
    KRYS_NODISCARD constexpr EnumSet::StorageType ToRaw() const noexcept
    {
      return _value.ToRaw();
    }

    KRYS_NODISCARD constexpr bool Contains(HangingPunctuationValue e) const noexcept
    {
      return _value.Contains(e);
    }
    KRYS_NODISCARD constexpr bool ContainsAny(EnumSet other) const noexcept
    {
      return _value.ContainsAny(other.Value);
    }
    KRYS_NODISCARD constexpr bool ContainsAll(EnumSet other) const noexcept
    {
      return _value.ContainsAll(other.Value);
    }
    KRYS_NODISCARD constexpr bool ContainsOnly(EnumSet other) const noexcept
    {
      return _value.ContainsOnly(other.Value);
    }

    KRYS_NODISCARD constexpr bool IsNone() const noexcept
    {
      return _value.IsEmpty();
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }
      return visitor(_value);
    }

    KRYS_NODISCARD constexpr bool operator==(const HangingPunctuation &) const noexcept = default;

  private:
    EnumSet _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<HangingPunctuation>
  {
    KRYS_NODISCARD HangingPunctuation operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::HangingPunctuation)
