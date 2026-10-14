#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'speak-as'> = none | normal | spell-out || digits || [ literal-punctuation | no-punctuation ]
  // FIXME: `none` is non-standard and computes to `normal`
  // https://drafts.csswg.org/css-speech-1/#propdef-speak-as

  enum class SpeakAsValue : uint8
  {
    SpellOut,
    Digits,
    LiteralPunctuation,
    NoPunctuation,
  };

  using SpeakAsValueEnumSet = css::SpaceSeparatedEnumSet<SpeakAsValue>;

  struct SpeakAs
  {
    using EnumSet = SpeakAsValueEnumSet;
    using value_type = SpeakAsValueEnumSet::value_type;

    constexpr SpeakAs(css::keywords::Normal) noexcept : _value {}
    {
    }
    constexpr SpeakAs(EnumSet &&set) noexcept : _value {krys::move(set)}
    {
    }
    constexpr SpeakAs(value_type value) noexcept : SpeakAs {EnumSet {value}}
    {
    }
    constexpr SpeakAs(std::initializer_list<value_type> initializerList) noexcept
        : SpeakAs {EnumSet {initializerList}}
    {
    }

    KRYS_NODISCARD constexpr static SpeakAs FromRaw(EnumSet::StorageType rawValue) noexcept
    {
      return EnumSet::FromRaw(rawValue);
    }
    KRYS_NODISCARD constexpr EnumSet::StorageType ToRaw() const noexcept
    {
      return _value.ToRaw();
    }

    KRYS_NODISCARD constexpr bool Contains(SpeakAsValue e) const noexcept
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

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return _value.IsEmpty();
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNormal())
      {
        return visitor(css::keywords::Normal {});
      }
      return visitor(_value);
    }

    KRYS_NODISCARD constexpr bool operator==(const SpeakAs &) const noexcept = default;

  private:
    EnumSet _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SpeakAs>
  {
    KRYS_NODISCARD SpeakAs operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SpeakAs)
