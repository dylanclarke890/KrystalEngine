#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'contain'> = none | strict@(aliased-to=[size layout paint style]) | content@(aliased-to=[layout paint
  // style]) | [ [size | inline-size] || layout || style || paint ]
  // https://drafts.csswg.org/css-contain-2/#contain-property

  enum class ContainValue : uint8
  {
    Size,
    InlineSize,
    Layout,
    Style,
    Paint,
  };

  using ContainValueEnumSet = css::SpaceSeparatedEnumSet<ContainValue>;

  struct Contain
  {
    using EnumSet = ContainValueEnumSet;
    using value_type = ContainValueEnumSet::value_type;

    static constexpr ContainValueEnumSet strict {ContainValue::Size, ContainValue::Layout,
                                                 ContainValue::Style, ContainValue::Paint};
    static constexpr ContainValueEnumSet content {ContainValue::Layout, ContainValue::Style,
                                                  ContainValue::Paint};

    constexpr Contain(css::keywords::None) noexcept : _value {}
    {
    }
    constexpr Contain(css::keywords::Strict) noexcept : _value {strict}
    {
    }
    constexpr Contain(css::keywords::Content) noexcept : _value {content}
    {
    }
    constexpr Contain(EnumSet &&set) noexcept : _value {krys::move(set)}
    {
    }
    constexpr Contain(value_type value) noexcept : Contain {EnumSet {value}}
    {
    }
    constexpr Contain(std::initializer_list<value_type> initializerList) noexcept
        : Contain {EnumSet {initializerList}}
    {
    }

    KRYS_NODISCARD constexpr static Contain FromRaw(EnumSet::StorageType rawValue) noexcept
    {
      if (!rawValue)
      {
        return css::keywords::None {};
      }
      return EnumSet::FromRaw(rawValue);
    }
    KRYS_NODISCARD constexpr EnumSet::StorageType ToRaw() const noexcept
    {
      return _value.ToRaw();
    }

    KRYS_NODISCARD constexpr bool Contains(value_type e) const noexcept
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
    constexpr void Add(EnumSet other) noexcept
    {
      _value.Value.Add(other.Value);
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

      // Handle "strict" and "content" shorthands
      if (_value == strict)
      {
        return visitor(css::keywords::Strict {});
      }
      if (_value == content)
      {
        return visitor(css::keywords::Content {});
      }

      return visitor(_value);
    }

    KRYS_NODISCARD constexpr bool operator==(const Contain &) const noexcept = default;

  private:
    EnumSet _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Contain>
  {
    KRYS_NODISCARD Contain operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Contain)
