#include "Krystal.Booey/HTML/Canvas/CanvasStyle.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorInlines.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"

namespace krys::boo::html
{
  namespace
  {
    class CanvasStyleColorResolutionDelegate final : public css::PlatformColorResolutionDelegate
    {
    public:
      const Ref<HTMLCanvasElement> CanvasElement;

      explicit CanvasStyleColorResolutionDelegate(Ref<HTMLCanvasElement> canvasElement) noexcept
          : CanvasElement {krys::move(canvasElement)}
      {
      }

      KRYS_NODISCARD Color CurrentColor() const noexcept final;
    };

    Color CanvasStyleColorResolutionDelegate::CurrentColor() const noexcept
    {
      if (!CanvasElement->IsConnected() || !CanvasElement->InlineStyle())
      {
        return Color::black;
      }

      auto colorString = CanvasElement->InlineStyle()->GetPropertyValue(css::PropertyId::Color);
      auto color = css::PropertyParserHelpers::ParseColorRaw(colorString, CanvasElement->CssParserContext());
      if (color.IsValid())
      {
        return color;
      }
      return Color::black;
    }
  }

  Color ParseColor(dom::DOMStringView colorString, CanvasBase &canvasBase) noexcept
  {
    using namespace css::PropertyParserHelpers;

    const auto &cssParserContext = canvasBase.CssParserContext();
    auto color = ParseColorRawSimple(colorString, cssParserContext);
    if (color.IsValid())
    {
      return color;
    }

    if (auto *canvasElement = DynamicDowncast<HTMLCanvasElement>(canvasBase))
    {
      CanvasStyleColorResolutionDelegate delegate(ShareRef(*canvasElement));
      ColorParsingOptions options;
      css::PlatformColorResolutionState state {.Delegate = &delegate};
      return ParseColorRawGeneral(colorString, cssParserContext, options, state);
    }

    ColorParsingOptions options {.AllowedColorTypes = css::ColorType::Absolute | css::ColorType::Current};
    css::PlatformColorResolutionState state {.ResolvedCurrentColor = Color::black};
    return ParseColorRawGeneral(colorString, cssParserContext, options, state);
  }

  Color ParseColor(dom::DOMStringView colorString) noexcept
  {
    using namespace css::PropertyParserHelpers;

    const css::ParserContext cssParserContext(css::ParserMode::HTMLStandard);
    auto color = ParseColorRawSimple(colorString, cssParserContext);
    if (color.IsValid())
    {
      return color;
    }

    ColorParsingOptions options;
    css::PlatformColorResolutionState state {.ResolvedCurrentColor = Color::black};
    return ParseColorRawGeneral(colorString, cssParserContext, options, state);
  }
}
