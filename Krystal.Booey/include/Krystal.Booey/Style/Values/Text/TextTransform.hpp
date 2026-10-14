#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'text-transform'> = none | [ capitalize | uppercase | lowercase ] || full-width || full-size-kana |
  // math-auto https://drafts.csswg.org/css-text/#propdef-text-transform Additional value `math-auto` added by
  // MathML https://w3c.github.io/mathml-core/#math-auto-transform

  enum class TextTransformValue : uint8
  {
    Capitalize,
    Uppercase,
    Lowercase,
    FullWidth,
    FullSizeKana,
    MathAuto,
  };
  constexpr auto maxTextTransformValue = TextTransformValue::MathAuto;

  using TextTransformValueEnumSet = css::SpaceSeparatedEnumSet<TextTransformValue>;

  // FIXME: This could be packed in to 5 bits if we didn't use an EnumSet.
  struct TextTransform
  {
    using EnumSet = TextTransformValueEnumSet;
    using value_type = TextTransformValueEnumSet::value_type;

    constexpr TextTransform(css::keywords::None) noexcept : _value {}
    {
    }
    constexpr TextTransform(EnumSet &&set) noexcept : _value {krys::move(set)}
    {
    }
    constexpr TextTransform(value_type value) noexcept : TextTransform {EnumSet {value}}
    {
    }
    constexpr TextTransform(std::initializer_list<value_type> initializerList) noexcept
        : TextTransform {EnumSet {initializerList}}
    {
    }

    KRYS_NODISCARD constexpr static TextTransform FromRaw(EnumSet::StorageType rawValue) noexcept
    {
      return EnumSet::FromRaw(rawValue);
    }
    KRYS_NODISCARD constexpr EnumSet::StorageType ToRaw() const noexcept
    {
      return _value.ToRaw();
    }

    KRYS_NODISCARD constexpr bool Contains(TextTransformValue e) const noexcept
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

    KRYS_NODISCARD constexpr bool operator==(const TextTransform &) const noexcept = default;

  private:
    EnumSet _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TextTransform>
  {
    KRYS_NODISCARD TextTransform operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextTransform)
