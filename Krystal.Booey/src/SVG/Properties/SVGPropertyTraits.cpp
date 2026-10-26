#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorInlines.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/Text/StringToIntegerConversion.hpp"

namespace krys::boo::svg
{
  namespace
  {
    class SVGStyleColorResolutionDelegate final : public css::PlatformColorResolutionDelegate
    {
    public:
      Ref<SVGElement> Element;

      explicit SVGStyleColorResolutionDelegate(Ref<SVGElement> element) noexcept
          : Element {krys::move(element)}
      {
      }

      KRYS_NODISCARD Color CurrentColor() const noexcept override
      {
        if (auto *renderer = Element->Renderer())
        {
          return renderer->Style().VisitedDependentColor(css::PropertyId::Color);
        }
        return {};
      }
    };
  }

  Color SVGPropertyTraits<Color>::FromString(SVGElement &targetElement, dom::DOMStringView string) noexcept
  {
    using namespace css::PropertyParserHelpers;

    Ref document = ShareRef(targetElement.NodeDocument());
    auto &cssParserContext = document->CssParserContext();

    auto trimmedString = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(string);

    auto color = ParseColorRawSimple(trimmedString, cssParserContext);
    if (color.IsValid())
    {
      return color;
    }

    SVGStyleColorResolutionDelegate delegate(ShareRef(targetElement));
    ColorParsingOptions options;
    css::PlatformColorResolutionState state {.Delegate = &delegate};
    return ParseColorRawGeneral(trimmedString, cssParserContext, options, state);
  }

  int32 SVGPropertyTraits<int32>::FromString(SVGElement &, dom::DOMStringView string) noexcept
  {
    return text::ParseInteger<int32>(string).value_or(0);
  }

  dom::DOMString SVGPropertyTraits<Color>::ToString(const Color &type) noexcept
  {
    return SerialisationForHTML(type);
  }
}
