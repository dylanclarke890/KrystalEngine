#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'margin-trim'> = none | [ block || inline ] | [ block-start || inline-start || block-end || inline-end ]
  // https://drafts.csswg.org/css-box/#margin-trim

  enum class MarginTrimSide : uint8
  {
    BlockStart,
    InlineStart,
    BlockEnd,
    InlineEnd,
  };

  using MarginTrimSideEnumSet = css::SpaceSeparatedEnumSet<MarginTrimSide>;

  struct MarginTrim
  {
    using EnumSet = MarginTrimSideEnumSet;
    using value_type = MarginTrimSideEnumSet::value_type;

    constexpr MarginTrim(EnumSet &&set) noexcept : _value {krys::move(set)}
    {
    }
    constexpr MarginTrim(css::keywords::None) noexcept : _value {}
    {
    }
    constexpr MarginTrim(value_type value) noexcept : MarginTrim {EnumSet {value}}
    {
    }
    constexpr MarginTrim(std::initializer_list<value_type> initializerList) noexcept
        : MarginTrim {EnumSet {initializerList}}
    {
    }

    KRYS_NODISCARD constexpr static MarginTrim FromRaw(EnumSet::StorageType rawValue) noexcept
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

    KRYS_NODISCARD constexpr bool Contains(MarginTrimSide e) const noexcept
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

      // Handle "block", "inline" and "block inline" shorthands
      if (_value.ContainsAll({MarginTrimSide::BlockStart, MarginTrimSide::BlockEnd})
          && !_value.ContainsAny({MarginTrimSide::InlineStart, MarginTrimSide::InlineEnd}))
      {
        return visitor(css::keywords::Block {});
      }
      if (_value.ContainsAll({MarginTrimSide::InlineStart, MarginTrimSide::InlineEnd})
          && !_value.ContainsAny({MarginTrimSide::BlockStart, MarginTrimSide::BlockEnd}))
      {
        return visitor(css::keywords::Inline {});
      }
      if (_value.ContainsAll({MarginTrimSide::BlockStart, MarginTrimSide::BlockEnd,
                              MarginTrimSide::InlineStart, MarginTrimSide::InlineEnd}))
      {
        return visitor(css::SpaceSeparatedTuple {css::keywords::Block {}, css::keywords::Inline {}});
      }

      return visitor(_value);
    }

    KRYS_NODISCARD constexpr bool operator==(const MarginTrim &) const noexcept = default;

  private:
    EnumSet _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<MarginTrim>
  {
    KRYS_NODISCARD MarginTrim operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MarginTrim)
