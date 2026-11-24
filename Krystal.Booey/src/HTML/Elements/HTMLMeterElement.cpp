#include "Krystal.Booey/HTML/Elements/HTMLMeterElement.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/CSS/UserAgentStyleSheets.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"
#include "Krystal.Booey/HTML/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMeter.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <numeric>

namespace krys::boo
{
  HTMLMeterElement::HTMLMeterElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Meter)
  {
  }

  HTMLMeterElement::~HTMLMeterElement() noexcept = default;

  Ref<HTMLMeterElement> HTMLMeterElement::Create(Document &document) noexcept
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

  bool HTMLMeterElement::ChildShouldCreateRenderer(const Node &child) const noexcept
  {
    return !Is<RenderMeter>(Renderer()) && HTMLElement::ChildShouldCreateRenderer(child);
  }

  void HTMLMeterElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                            Maybe<DOMStringView> value,
                                            const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
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
    KRYS_NODISCARD double ParseAttribute(const HTMLMeterElement &element, DOMStringAtom name,
                                         double fallbackValue) noexcept
    {
      auto attribute = element.GetAttribute(krys::move(name));
      return attribute ? html::ParseHTMLFloatingPointNumberValue(*attribute, fallbackValue) : fallbackValue;
    }

    void SetDoubleAttribute(HTMLMeterElement &element, DOMStringAtom name, double value) noexcept
    {
      html::Reflect<double>(element, krys::move(name), krys::move(value));
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
      // The optimum region is below low.
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
      // The optimum region is above high.
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

    // The optimum region is between low and high; the value, clamped to min and max, is never in an even less
    // good region.
    // https://html.spec.whatwg.org/#the-meter-element
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
          element.SetUserAgentPart(css::user_agent_parts::WebkitMeterOptimumValue());
          return;
        case HTMLMeterElement::GaugeRegion::Suboptimal:
          (void)element.SetAttribute(u8"class", u8"suboptimum");
          element.SetUserAgentPart(css::user_agent_parts::WebkitMeterSuboptimumValue());
          return;
        case HTMLMeterElement::GaugeRegion::EvenLessGood:
          (void)element.SetAttribute(u8"class", u8"even-less-good");
          element.SetUserAgentPart(css::user_agent_parts::WebkitMeterEvenLessGoodValue());
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

  void HTMLMeterElement::DidAddUserAgentShadowRoot(boo::ShadowRoot &root) noexcept
  {
    krys_debug_assert(!_valueElement);

    auto &document = NodeDocument();
    auto style = document.CreateElementNS(infra::namespaces::HTML, u8"style").ReleaseValue();
    (void)style->TextContent(DOMString(css::MeterElementShadowUserAgentStyleSheet));
    (void)root.AppendChild(*style);

    // The parts are named, so the page can style them.
    auto inner = CreateRef<HTMLDivElement>(document);
    inner->Id(u8"inner");
    inner->SetUserAgentPart(css::user_agent_parts::WebkitMeterInnerElement());
    (void)root.AppendChild(*inner);

    auto bar = CreateRef<HTMLDivElement>(document);
    bar->Id(u8"bar");
    bar->SetUserAgentPart(css::user_agent_parts::WebkitMeterBar());
    (void)inner->AppendChild(*bar);

    auto valueElement = CreateRef<HTMLDivElement>(document);
    valueElement->Id(u8"value");
    (void)bar->AppendChild(*valueElement);
    _valueElement = CreateWeakPtr(valueElement.get());

    DidElementStateChange();
  }
}
