#include "Krystal.Booey/HTML/ColorInputType.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLDivElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <string_view>

namespace krys::boo::html
{
  namespace
  {
    // https://html.spec.whatwg.org/#valid-simple-colour
    KRYS_NODISCARD bool IsValidSimpleColor(dom::DOMStringView string) noexcept
    {
      if (string.size() != 7)
      {
        return false;
      }
      if (string[0] != u8'#')
      {
        return false;
      }
      for (size_t i = 1; i < 7; ++i)
      {
        if (!text::IsASCIIHexDigit(string[i]))
        {
          return false;
        }
      }
      return true;
    }

    // https://html.spec.whatwg.org/#rules-for-parsing-simple-colour-values
    KRYS_NODISCARD Maybe<SRGBA<uint8>> ParseSimpleColorValue(dom::DOMStringView string) noexcept
    {
      if (!IsValidSimpleColor(string))
      {
        return null;
      }
      auto hex = [&](size_t i)
      {
        return static_cast<uint8>(text::ToASCIIHexValue(string[i]) * 16
                                  + text::ToASCIIHexValue(string[i + 1]));
      };
      return SRGBA<uint8> {hex(1), hex(3), hex(5), 255};
    }

    /// @brief The serialisation of a simple colour: # and six lowercase hexadecimal digits.
    KRYS_NODISCARD dom::DOMString SerializeColorValue(const Color &input) noexcept
    {
      constexpr std::string_view hexDigits = "0123456789abcdef";
      dom::DOMString result(u8"#");
      auto resolved = ResolvedColour(input.ToColorTypeLossy<SRGBA<uint8>>());
      for (uint8 component : {resolved.Red, resolved.Green, resolved.Blue})
      {
        result += static_cast<char8>(hexDigits[component >> 4]);
        result += static_cast<char8>(hexDigits[component & 0xF]);
      }
      return result;
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
    return SerializeColorValue(Color::black);
  }

  dom::DOMString ColorInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    auto color = ParseSimpleColorValue(proposedValue);

    if (!color)
    {
      return *FallbackValue();
    }

    return SerializeColorValue(*color);
  }

  Color ColorInputType::ValueAsColor() const noexcept
  {
    krys_debug_assert(Element());
    auto color = ParseSimpleColorValue(Element()->Value());
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

    // FIXME: Only while processing a user gesture (UserGestureIndicator), once there are gestures.

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
    auto serializedColor = SerializeColorValue(color);
    if (serializedColor == input->Value())
    {
      return;
    }

    // FIXME: Queueing the events until the value is set (EventQueueScope), once events can be queued.
    input->SetValueFromRenderer(serializedColor);
    UpdateColorSwatch();
    input->DispatchFormControlChangeEvent();
  }

  void ColorInputType::DidEndChooser() noexcept
  {
    _chooser = nullptr;
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

  bool ColorInputType::SupportsAlpha() const noexcept
  {
    return false;
  }

  List<Color> ColorInputType::SuggestedColors() const noexcept
  {
    return {};
  }
}
