#include "Krystal.Booey/Style/Values/Content/Content.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/AttrValue.hpp"
#include "Krystal.Booey/CSS/Values/CounterValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Content CSSValueConversion<Content>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal: return css::keywords::Normal {};
        case css::ValueId::None:   return css::keywords::None {};
        default:                   break;
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Normal {};
    }

    const auto *contentListAltTextPair = DynamicDowncast<css::ValuePair>(value);
    const auto *contentList = RequiredDowncast<css::ValueList>(
      state, contentListAltTextPair ? contentListAltTextPair->First() : value);
    if (!contentList)
    {
      return css::keywords::Normal {};
    }

    // FIXME: Replace with support for CSS Values 5 attr() substitution function.
    auto processAttrContent = [&](const css::AttrValue &attrValue) -> css::CSSOMString
    {
      if (!state.Style().GetPseudoElementType())
      {
        state.Style().SetHasAttrContent();
      }
      else
      {
        const_cast<RenderStyle &>(state.ParentStyle()).SetHasAttrContent();
      }

      css::CSSOMStringAtom attributeLocalName {attrValue.AttributeName()};
      const auto *element = state.GetElement();
      auto attributeValue =
        element ? element->GetAttributeNS(dom::DOMStringAtom::Null(), attributeLocalName) : std::nullopt;

      // Register the fact that the attribute value affects the style.
      state.RegisterContentAttribute(attributeLocalName);

      if (!attributeValue)
      {
        const auto *fallback = DynamicDowncast<css::PrimitiveValue>(attrValue.Fallback());
        return fallback && fallback->IsString() ? fallback->StringValue() : css::CSSOMString {};
      }
      return *attributeValue;
    };

    auto computeContentList = [&]() -> Content::ListType
    {
      return Content::ListType::map(
        *contentList,
        [&](const css::Value &item) -> Content::ListItem
        {
          if (item.IsImage())
          {
            if (auto image = state.CreateStyleImage(item))
            {
              return Content::Image {ImageWrapper {Ref<StyleImage> {krys::move(image)}}};
            }
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return Content::Text {css::CSSOMString {}};
          }
          if (const auto *primitive = DynamicDowncast<css::PrimitiveValue>(item))
          {
            switch (primitive->ValueId())
            {
              case css::ValueId::OpenQuote:    return Content::Quote {QuoteType::OpenQuote};
              case css::ValueId::CloseQuote:   return Content::Quote {QuoteType::CloseQuote};
              case css::ValueId::NoOpenQuote:  return Content::Quote {QuoteType::NoOpenQuote};
              case css::ValueId::NoCloseQuote: return Content::Quote {QuoteType::NoCloseQuote};
              default:                         break;
            }
            if (primitive->IsString())
            {
              return Content::Text {primitive->StringValue()};
            }
            if (const auto *attr = primitive->CSSAttrValue())
            {
              return Content::Text {processAttrContent(*attr)};
            }
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return Content::Text {css::CSSOMString {}};
          }
          if (const auto *counter = DynamicDowncast<css::CounterValue>(item))
          {
            return Content::Counter {counter->Identifier(), counter->Separator(),
                                     ConvertCSSValueToStyle<CounterStyle>(state, *counter->CounterStyle())};
          }
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return Content::Text {css::CSSOMString {}};
        });
    };

    auto computeAltText = [&]() -> Maybe<css::CSSOMString>
    {
      if (!contentListAltTextPair)
      {
        return {};
      }

      auto altTextList =
        RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, contentListAltTextPair->Second());
      if (!altTextList)
      {
        return {};
      }

      css::CSSOMString altText;
      for (const auto &item : *altTextList)
      {
        if (item.IsString())
        {
          altText += item.StringValue();
        }
        else if (const auto *attr = item.CSSAttrValue())
        {
          altText += processAttrContent(*attr);
        }
      }
      return altText;
    };

    return Content::Data {computeContentList(), computeAltText()};
  }

  Ref<css::Value> CSSValueCreation<Content::Counter>::operator()(css::ValuePool &pool,
                                                                 const RenderStyle &style,
                                                                 const Content::Counter &value) noexcept
  {
    return css::CounterValue::Create(value.Identifier, value.Separator,
                                     CreateCSSValue(pool, style, value.Style));
  }
}
