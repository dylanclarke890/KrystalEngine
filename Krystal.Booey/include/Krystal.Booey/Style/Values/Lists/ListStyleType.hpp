#pragma once

#include "Krystal.Booey/Style/Values/CounterStyles/CounterStyle.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'list-style-type'> = <counter-style> | <string> | none
  // https://drafts.csswg.org/css-lists/#propdef-list-style-type
  struct ListStyleType
  {
  private:
    enum class Type : uint8
    {
      CounterStyle,
      String,
      None
    };

    Type _type {Type::None};
    // The identifier is the string when the type is String and is the @counter-style name when the type is
    // CounterStyle.
    css::CSSOMStringAtom _identifier {css::CSSOMStringAtom::Null()};

  public:
    ListStyleType(css::keywords::None) noexcept : _type {Type::None}
    {
    }

    ListStyleType(css::CSSOMStringAtom &&string) noexcept
        : _type {Type::String}, _identifier {krys::move(string)}
    {
    }

    ListStyleType(CounterStyle &&counterStyle) noexcept
        : _type {Type::CounterStyle}, _identifier {krys::move(counterStyle.Identifier.Value)}
    {
    }

    // <counter-style> specific constructors.

    ListStyleType(css::keywords::Circle keyword) noexcept
        : ListStyleType {CounterStyle {{css::CSSOMStringAtom {css::ToString(keyword.Value)}}}}
    {
    }

    ListStyleType(css::keywords::Decimal keyword) noexcept
        : ListStyleType {CounterStyle {{css::CSSOMStringAtom {css::ToString(keyword.Value)}}}}
    {
    }

    ListStyleType(css::keywords::Disc keyword) noexcept
        : ListStyleType {CounterStyle {{css::CSSOMStringAtom {css::ToString(keyword.Value)}}}}
    {
    }

    ListStyleType(css::keywords::Square keyword) noexcept
        : ListStyleType {CounterStyle {{css::CSSOMStringAtom {css::ToString(keyword.Value)}}}}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return _type == Type::None;
    }

    KRYS_NODISCARD bool IsCounterStyle() const noexcept
    {
      return _type == Type::CounterStyle;
    }

    KRYS_NODISCARD bool IsString() const noexcept
    {
      return _type == Type::String;
    }

    // <counter-style> specific predicates.

    KRYS_NODISCARD bool IsCircle() const noexcept;
    KRYS_NODISCARD bool IsDisc() const noexcept;
    KRYS_NODISCARD bool IsDecimal() const noexcept;
    KRYS_NODISCARD bool IsSquare() const noexcept;

    KRYS_NODISCARD Maybe<CounterStyle> TryCounterStyle() const noexcept
    {
      return IsCounterStyle() ? Maybe<CounterStyle> {CounterStyle {{_identifier}}} : null;
    }

    KRYS_NODISCARD Maybe<css::CSSOMStringAtom> TryString() const noexcept
    {
      return IsString() ? Maybe<css::CSSOMStringAtom> {_identifier} : null;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_type)
      {
        case Type::None:         return visitor(css::keywords::None {});
        case Type::String:       return visitor(_identifier);
        case Type::CounterStyle: return visitor(CounterStyle {{_identifier}});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD bool operator==(const ListStyleType &) const noexcept = default;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ListStyleType>
  {
    KRYS_NODISCARD ListStyleType operator()(BuilderState &, const css::Value &) noexcept;
  };

  template <>
  struct CSSValueCreation<ListStyleType>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const ListStyleType &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ListStyleType)
