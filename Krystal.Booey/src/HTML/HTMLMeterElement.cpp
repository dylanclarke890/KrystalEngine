#include "Krystal.Booey/HTML/HTMLMeterElement.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/CSS/UserAgentStyleSheets.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLDivElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/RenderMeter.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <numeric>

namespace krys::boo::html
{
  HTMLMeterElement::HTMLMeterElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Meter)
  {
  }

  HTMLMeterElement::~HTMLMeterElement() noexcept = default;

  Ref<HTMLMeterElement> HTMLMeterElement::Create(dom::Document &document) noexcept
  {
    auto meter = CreateRef<HTMLMeterElement>(document);
    meter->EnsureUserAgentShadowRoot();
    return meter;
  }

  RenderPtr<RenderElement> HTMLMeterElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    if (!RenderTheme::Singleton().SupportsMeter(style.UsedAppearance()))
    {
      return RenderElement::CreateFor(*this, krys::move(style));
    }

    return CreateRenderer<RenderMeter>(*this, krys::move(style));
  }

  bool HTMLMeterElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    return !Is<RenderMeter>(Renderer()) && HTMLElement::ChildShouldCreateRenderer(child);
  }

  void HTMLMeterElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value,
                                            dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"value" || localName == u8"min" || localName == u8"max" || localName == u8"low"
        || localName == u8"high" || localName == u8"optimum")
    {
      DidElementStateChange();
    }
  }

#pragma region HTMLMeterElement

  namespace
  {
    KRYS_NODISCARD double ParseAttribute(const HTMLMeterElement &element, dom::DOMStringAtom name,
                                         double fallbackValue) noexcept
    {
      auto attribute = element.GetAttribute(name);
      return attribute ? ParseHTMLFloatingPointNumberValue(*attribute, fallbackValue) : fallbackValue;
    }

    void SetDoubleAttribute(HTMLMeterElement &element, dom::DOMStringAtom name, double value) noexcept
    {
      Reflection::Reflect<double>(element, name, krys::move(value));
    }
  }

  double HTMLMeterElement::Min() const noexcept
  {
    return ParseAttribute(*this, u8"min", 0);
  }

  void HTMLMeterElement::Min(double value) noexcept
  {
    SetDoubleAttribute(*this, u8"min", value);
  }

  double HTMLMeterElement::Max() const noexcept
  {
    return std::max(ParseAttribute(*this, u8"max", std::max(1.0, Min())), Min());
  }

  void HTMLMeterElement::Max(double value) noexcept
  {
    SetDoubleAttribute(*this, u8"max", value);
  }

  double HTMLMeterElement::Value() const noexcept
  {
    double value = ParseAttribute(*this, u8"value", 0);
    return std::min(std::max(value, Min()), Max());
  }

  void HTMLMeterElement::Value(double value) noexcept
  {
    SetDoubleAttribute(*this, u8"value", value);
  }

  double HTMLMeterElement::Low() const noexcept
  {
    double low = ParseAttribute(*this, u8"low", Min());
    return std::min(std::max(low, Min()), Max());
  }

  void HTMLMeterElement::Low(double value) noexcept
  {
    SetDoubleAttribute(*this, u8"low", value);
  }

  double HTMLMeterElement::High() const noexcept
  {
    double high = ParseAttribute(*this, u8"high", Max());
    return std::min(std::max(high, Low()), Max());
  }

  void HTMLMeterElement::High(double value) noexcept
  {
    SetDoubleAttribute(*this, u8"high", value);
  }

  double HTMLMeterElement::Optimum() const noexcept
  {
    double optimum = ParseAttribute(*this, u8"optimum", std::midpoint(Min(), Max()));
    return std::clamp(optimum, Min(), Max());
  }

  void HTMLMeterElement::Optimum(double value) noexcept
  {
    SetDoubleAttribute(*this, u8"optimum", value);
  }

#pragma endregion

  HTMLMeterElement::GaugeRegion HTMLMeterElement::GetGaugeRegion() const noexcept
  {
    double lowValue = Low();
    double highValue = High();
    double theValue = Value();
    double optimumValue = Optimum();

    if (optimumValue < lowValue)
    {
      // The optimum range stays under low
      if (theValue <= lowValue)
      {
        return GaugeRegion::Optimum;
      }
      if (theValue <= highValue)
      {
        return GaugeRegion::Suboptimal;
      }
      return GaugeRegion::EvenLessGood;
    }

    if (highValue < optimumValue)
    {
      // The optimum range stays over high
      if (highValue <= theValue)
      {
        return GaugeRegion::Optimum;
      }
      if (lowValue <= theValue)
      {
        return GaugeRegion::Suboptimal;
      }
      return GaugeRegion::EvenLessGood;
    }

    // The optimum range stays between high and low.
    // According to the standard, <meter> never show GaugeRegionEvenLessGood in this case
    // because the value is never less or greater than min or max.
    if (lowValue <= theValue && theValue <= highValue)
    {
      return GaugeRegion::Optimum;
    }
    return GaugeRegion::Suboptimal;
  }

  double HTMLMeterElement::ValueRatio() const noexcept
  {
    double min = Min();
    double max = Max();
    double value = Value();

    if (max <= min)
    {
      return 0;
    }
    return (value - min) / (max - min);
  }

  namespace
  {
    void SetValueClass(HTMLElement &element, HTMLMeterElement::GaugeRegion gaugeRegion) noexcept
    {
      switch (gaugeRegion)
      {
        case HTMLMeterElement::GaugeRegion::Optimum:
          (void)element.SetAttribute(u8"class", u8"optimum");
          element.SetUserAgentPart(css::UserAgentParts::WebkitMeterOptimumValue());
          return;
        case HTMLMeterElement::GaugeRegion::Suboptimal:
          (void)element.SetAttribute(u8"class", u8"suboptimum");
          element.SetUserAgentPart(css::UserAgentParts::WebkitMeterSuboptimumValue());
          return;
        case HTMLMeterElement::GaugeRegion::EvenLessGood:
          (void)element.SetAttribute(u8"class", u8"even-less-good");
          element.SetUserAgentPart(css::UserAgentParts::WebkitMeterEvenLessGoodValue());
          return;
      }
    }
  }

  void HTMLMeterElement::DidElementStateChange() noexcept
  {
    if (auto *valueElement = _valueElement.get())
    {
      (void)valueElement->SetInlineStyleProperty(css::PropertyId::InlineSize, ValueRatio() * 100,
                                                 css::UnitType::Percentage);
      SetValueClass(*valueElement, GetGaugeRegion());
    }

    if (auto *renderer = GetRenderMeter())
    {
      renderer->UpdateFromElement();
    }
  }

  RenderMeter *HTMLMeterElement::GetRenderMeter() const noexcept
  {
    return DynamicDowncast<RenderMeter>(Renderer());
  }

  void HTMLMeterElement::DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept
  {
    krys_debug_assert(!_valueElement);

    auto &document = NodeDocument();
    auto style =
      document.CreateElementNS(dom::DOMStringAtom(infra::Namespaces::HTML), dom::DOMStringAtom(u8"style"))
        .ReleaseValue();
    (void)style->TextContent(dom::DOMString(css::MeterElementShadowUserAgentStyleSheet));
    (void)root.AppendChild(*style);

    // Pseudos are set to allow author styling.
    auto inner = CreateRef<HTMLDivElement>(document);
    inner->Id(u8"inner");
    inner->SetUserAgentPart(css::UserAgentParts::WebkitMeterInnerElement());
    (void)root.AppendChild(*inner);

    auto bar = CreateRef<HTMLDivElement>(document);
    bar->Id(u8"bar");
    bar->SetUserAgentPart(css::UserAgentParts::WebkitMeterBar());
    (void)inner->AppendChild(*bar);

    auto valueElement = CreateRef<HTMLDivElement>(document);
    valueElement->Id(u8"value");
    (void)bar->AppendChild(*valueElement);
    _valueElement = WeakPtr<HTMLElement>::WithRef(*valueElement);

    DidElementStateChange();
  }
}
