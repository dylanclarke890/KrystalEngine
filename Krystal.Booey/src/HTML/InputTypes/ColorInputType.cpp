#include "Krystal.Booey/HTML/InputTypes/ColorInputType.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorInlines.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/ScopedEventQueue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"

namespace krys::boo::html
{
  namespace
  {
    // https://html.spec.whatwg.org/#update-a-color-well-control-color
    // 3. Let color be the result of parsing value.
    // (As a CSS <color>, with currentcolor as black, as WebKit does.)
    KRYS_NODISCARD Maybe<Color> ParseColorValue(dom::DOMStringView string,
                                                const HTMLInputElement &input) noexcept
    {
      using namespace css::PropertyParserHelpers;
      auto context = input.NodeDocument().CssParserContext();
      context.Mode = css::ParserMode::HTMLStandard;
      auto color = ParseColorRawSimple(string, context);
      if (color.IsValid())
      {
        return color;
      }

      ColorParsingOptions options;
      css::PlatformColorResolutionState state {.ResolvedCurrentColor = Color::black};
      color = ParseColorRawGeneral(string, context, options, state);
      if (color.IsValid())
      {
        return color;
      }
      return null;
    }

    // https://html.spec.whatwg.org/#serialize-a-color-well-control-color
    KRYS_NODISCARD dom::DOMString SerializeColorValue(Color color, const HTMLInputElement &element) noexcept
    {
      // 1. Assert: element is an input element whose type attribute is in the Color state.
      // 2. Let htmlCompatible be false.
      // 3. If element's alpha attribute is not specified, then set color's alpha component to be fully
      // opaque.
      bool alpha = element.Alpha();
      if (!alpha)
      {
        color = color.OpaqueColor();
      }

      // 4. If element's colorspace attribute is in the Limited sRGB state:
      if (element.ColourSpace() == u8"limited-srgb")
      {
        // 4a. Set color to color converted to the 'srgb' color space.
        // 4b. Round each of color's components so they are in the range 0 to 255, inclusive. Components are
        //     to be rounded towards +∞.
        // (Rounded to the nearest, as WebKit does.)
        auto rgba = color.ToColorTypeLossy<SRGBA<uint8>>();
        // 4c. If element's alpha attribute is not specified, then set htmlCompatible to true.
        // 4d. Otherwise, set color to color converted using the 'color()' function.
        color = alpha ? Color(rgba, ColorFlags::UseColorFunctionSerialisation) : Color(ResolvedColour(rgba));
      }
      // 5. Otherwise:
      else
      {
        // 5a. Assert: element's colorspace attribute is in the Display P3 state.
        krys_debug_assert(element.ColourSpace() == u8"display-p3");
        // 5b. Set color to color converted to the 'display-p3' color space.
        color = ResolvedColour(color.ToColorTypeLossy<ExtendedDisplayP3<float>>());
      }

      // 6. Return the result of serializing color. If htmlCompatible is true, then do so with HTML-compatible
      //    serialization requested.
      // (An opaque sRGB colour serialises as # and six lowercase hexadecimal digits.)
      return dom::DOMString(SerialisationForHTML(color));
    }
  }

  ColorInputType::~ColorInputType() noexcept
  {
    EndColorChooser();
  }

  bool ColorInputType::IsPresentingAttachedView() const noexcept
  {
    return !!_chooser;
  }

  bool ColorInputType::SupportsRequired() const noexcept
  {
    return false;
  }

  Maybe<dom::DOMString> ColorInputType::FallbackValue() const noexcept
  {
    krys_debug_assert(Element());
    return SerializeColorValue(Color::black, *Element());
  }

  dom::DOMString ColorInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    // https://html.spec.whatwg.org/#update-a-color-well-control-color
    krys_debug_assert(Element());
    auto color = ParseColorValue(proposedValue, *Element());

    // 4. If color is failure, then set color to opaque black.
    if (!color)
    {
      return *FallbackValue();
    }

    // 5. Set element's value to the result of serializing a color well control color given element and color.
    return SerializeColorValue(*color, *Element());
  }

  Color ColorInputType::ValueAsColor() const noexcept
  {
    krys_debug_assert(Element());
    auto color = ParseColorValue(Element()->Value(), *Element());
    krys_debug_assert(!!color);
    if (!color)
    {
      return Color::black;
    }
    return *color;
  }

  void ColorInputType::CreateShadowSubtree() noexcept
  {
    krys_debug_assert(NeedsShadowSubtree());
    krys_debug_assert(Element());
    krys_debug_assert(Element()->UserAgentShadowRoot());

    auto &document = Element()->NodeDocument();
    auto wrapperElement = CreateRef<HTMLDivElement>(document);
    auto colorSwatch = CreateRef<HTMLDivElement>(document);

    auto &shadowRoot = *Element()->UserAgentShadowRoot();
    (void)shadowRoot.AppendChild(*wrapperElement);

    (void)wrapperElement->AppendChild(*colorSwatch);
    wrapperElement->SetUserAgentPart(css::UserAgentParts::WebkitColorSwatchWrapper());
    colorSwatch->SetUserAgentPart(css::UserAgentParts::WebkitColorSwatch());

    RenderTheme::Singleton().CreateColorWellSwatchSubtree(*colorSwatch);

    UpdateColorSwatch();
  }

  void ColorInputType::SetValue(dom::DOMStringView value, bool valueChanged,
                                TextFieldEventBehavior eventBehavior,
                                TextControlSetValueSelection selection) noexcept
  {
    InputType::SetValue(value, valueChanged, eventBehavior, selection);

    if (!valueChanged)
    {
      return;
    }

    UpdateColorSwatch();
    if (RefPtr<ColorChooser> chooser = _chooser)
    {
      chooser->SetSelectedColor(ValueAsColor());
    }
  }

  void ColorInputType::AttributeChanged(dom::DOMStringAtom name) noexcept
  {
    if (name == u8"value")
    {
      UpdateColorSwatch();
      // FIXME: Telling accessibility the value changed (AXObjectCache::valueChanged), with accessibility.
    }

    InputType::AttributeChanged(name);
  }

  void ColorInputType::HandleActivationBehavior(dom::Event &event) noexcept
  {
    krys_debug_assert(Element());
    if (Element()->IsDisabledFormControl() || !Element()->Renderer())
    {
      return;
    }

    // Only for a user's click or key.
    if (!HasTransientActivation())
    {
      return;
    }

    ShowPicker();
    event.SetDefaultHandled();
  }

  void ColorInputType::ShowPicker() noexcept
  {
    if (auto *chromeClient = GetChromeClient())
    {
      if (RefPtr<ColorChooser> chooser = _chooser)
      {
        chooser->ReattachColorChooser(ValueAsColor());
      }
      else
      {
        Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
        if (auto *element = Element())
        {
          styleInvalidation.emplace(*element, css::SelectorPseudoClass::Open, true);
        }
        _chooser = chromeClient->CreateColorChooser(*this, ValueAsColor());
      }
    }
  }

  void ColorInputType::Detach() noexcept
  {
    EndColorChooser();
  }

  void ColorInputType::ElementDidBlur() noexcept
  {
    EndColorChooser();
  }

  void ColorInputType::DidChooseColor(const Color &color) noexcept
  {
    auto *element = Element();
    if (!element || element->IsDisabledFormControl())
    {
      return;
    }

    Ref<HTMLInputElement> input = ShareRef(*element);
    auto serializedColor = SerializeColorValue(color, *input);
    if (serializedColor == input->Value())
    {
      return;
    }

    // input and change wait until the swatch shows the new value.
    dom::EventQueueScope eventQueue;
    input->SetValueFromRenderer(serializedColor);
    UpdateColorSwatch();
    input->DispatchFormControlChangeEvent();
  }

  void ColorInputType::DidEndChooser() noexcept
  {
    {
      Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
      if (auto *element = Element())
      {
        styleInvalidation.emplace(*element, css::SelectorPseudoClass::Open, false);
      }
      _chooser = nullptr;
    }
    if (auto *element = Element())
    {
      if (auto *renderer = element->Renderer())
      {
        renderer->Repaint();
      }
    }
  }

  void ColorInputType::EndColorChooser() noexcept
  {
    if (RefPtr<ColorChooser> chooser = _chooser)
    {
      chooser->EndChooser();
    }
  }

  void ColorInputType::UpdateColorSwatch() noexcept
  {
    auto *colorSwatch = ShadowColorSwatch();
    if (!colorSwatch)
    {
      return;
    }

    RenderTheme::Singleton().SetColorWellSwatchBackground(*colorSwatch, ValueAsColor());
  }

  HTMLElement *ColorInputType::ShadowColorSwatch() const noexcept
  {
    krys_debug_assert(Element());
    auto *shadow = Element()->UserAgentShadowRoot();
    if (!shadow)
    {
      return nullptr;
    }

    auto *wrapper = shadow->FirstChild();
    return wrapper ? DynamicDowncast<HTMLElement>(wrapper->FirstChild()) : nullptr;
  }

  IntRect ColorInputType::ElementRectRelativeToRootView() const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto *renderer = element.Renderer();
    auto *view = element.NodeDocument().View();
    if (!renderer || !view)
    {
      return IntRect();
    }
    return view->ContentsToView(renderer->AbsoluteBoundingBoxRect());
  }

  bool ColorInputType::IsElementRTL() const noexcept
  {
    krys_debug_assert(Element());
    auto *renderer = Element()->Renderer();
    return renderer && renderer->Style().GetWritingMode().ComputedTextDirection() == TextDirection::RTL;
  }

  bool ColorInputType::SupportsAlpha() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->Alpha();
  }

  List<Color> ColorInputType::SuggestedColors() const noexcept
  {
    // The datalist's suggestions that are colours.
    List<Color> suggestions;
    krys_debug_assert(Element());
    if (auto dataList = Element()->List())
    {
      for (auto &option : dataList->Suggestions())
      {
        if (auto color = ParseColorValue(option->Value(), *Element()))
        {
          suggestions.push_back(*color);
        }
      }
    }
    return suggestions;
  }
}
