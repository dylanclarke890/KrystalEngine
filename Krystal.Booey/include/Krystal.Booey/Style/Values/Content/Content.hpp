#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/CounterStyles/CounterStyle.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageWrapper.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <leader()>            = leader( <leader-type> )
  // <target>              = <target-counter()> | <target-counters()> | <target-text()>
  // <counter>             = <counter()> | <counters()>
  // <quote>               = open-quote | close-quote | no-open-quote | no-close-quote
  // <content-replacement> = <image>
  // <content-list>        = [ <string> | contents | <image> | <counter> | <quote> | <target> | <leader()> ]+
  // <alt-content>         = [ <string> | <counter> ]+

  // <'content'>           = normal | none | [ <content-replacement> | <content-list> ] [/ <alt-content> ]?
  // https://www.w3.org/TR/css-content-3/#propdef-content

  // MISSING from <content-list>:
  //    contents
  //    <leader()>
  //    <target>

  // MISSING from <alt-content>:
  //   <counter>

  struct Content
  {
    struct Text
    {
      css::CSSOMString Value;

      KRYS_NODISCARD bool operator==(const Text &) const noexcept = default;
    };
    struct Image
    {
      ImageWrapper Value;

      KRYS_NODISCARD bool operator==(const Image &) const noexcept = default;
    };
    struct Counter
    {
      css::CSSOMStringAtom Identifier;
      css::CSSOMStringAtom Separator;
      CounterStyle Style;

      template <typename... F>
      decltype(auto) SwitchOn(F &&...) const noexcept;

      KRYS_NODISCARD bool operator==(const Counter &) const noexcept = default;
    };
    struct Quote
    {
      QuoteType Value;

      KRYS_NODISCARD bool operator==(const Quote &) const noexcept = default;
    };
    using ListItem = Variant<Text, Image, Counter, Quote>;
    using ListType = css::SpaceSeparatedFixedList<ListItem>;

    // FIXME: This struct could be optimized down to a pointer when unused by using TrailingArray.
    struct Data
    {
      ListType List;
      Maybe<css::CSSOMString> AltText;

      KRYS_NODISCARD bool operator==(const Data &) const noexcept = default;
    };

    Content(css::keywords::None keyword) noexcept : _value {keyword}
    {
    }

    Content(css::keywords::Normal keyword) noexcept : _value {keyword}
    {
    }

    Content(Data &&data) noexcept : _value {krys::move(data)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::None>(_value);
    }
    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Normal>(_value);
    }
    KRYS_NODISCARD bool IsData() const noexcept
    {
      return krys::HoldsAlternative<Data>(_value);
    }

    KRYS_NODISCARD const Data *TryData() const noexcept KRYS_LIFETIME_BOUND
    {
      return std::get_if<Data>(&_value);
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const Content &) const noexcept = default;

  private:
    // FIXME: If Data is converted to be pointer sized when unused using TrailingArray (see above), this can
    // be converted to use a CompactVariant.
    Variant<css::keywords::None, css::keywords::Normal, Data> _value;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Content::Data &value) noexcept
  {
    if constexpr (!I)
    {
      return value.List;
    }
    else if constexpr (I == 1)
    {
      return value.AltText;
    }
  }

  DEFINE_TYPE_WRAPPER_GET(Content::Text, Value);
  DEFINE_TYPE_WRAPPER_GET(Content::Image, Value);
  DEFINE_TYPE_WRAPPER_GET(Content::Quote, Value);

  template <typename... F>
  decltype(auto) Content::Counter::SwitchOn(F &&...f) const noexcept
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    using CounterFunction =
      css::FunctionNotation<css::ValueId::Counter,
                            css::CommaSeparatedTuple<css::CustomIdentifier, Maybe<CounterStyle>>>;
    using CountersFunction = css::FunctionNotation<
      css::ValueId::Counters,
      css::CommaSeparatedTuple<css::CustomIdentifier, css::CSSOMStringAtom, Maybe<CounterStyle>>>;

    if (Separator.View().empty())
    {
      return visitor(CounterFunction {
        .Parameters = {
          css::CustomIdentifier {Identifier},
          Style != css::CustomIdentifier {css::CSSOMStringAtom {css::ToString(css::ValueId::Decimal)}}
            ? Maybe<CounterStyle> {Style}
            : std::nullopt}});
    }
    else
    {
      return visitor(CountersFunction {
        .Parameters = {
          css::CustomIdentifier {Identifier}, Separator,
          Style != css::CustomIdentifier {css::CSSOMStringAtom {css::ToString(css::ValueId::Decimal)}}
            ? Maybe<CounterStyle> {Style}
            : std::nullopt}});
    }
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Content>
  {
    KRYS_NODISCARD Content operator()(BuilderState &, const css::Value &) noexcept;
  };

  // `Content::Counter` is special-cased to return a `css::CounterValue`.
  template <>
  struct CSSValueCreation<Content::Counter>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const Content::Counter &) noexcept;
  };
}

DEFINE_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Content::Data, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::Content::Text)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::Content::Image)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::Content::Quote)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Content::Counter)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Content)
