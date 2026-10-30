#include "Krystal.Booey/HTML/HTMLAreaElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Algorithms/HyperlinkAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLMapElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  HTMLAreaElement::HTMLAreaElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Area)
  {
    SetEventTargetFlag(dom::EventTargetFlags::HasActivationBehavior);
  }

  void HTMLAreaElement::ActivationBehavior(dom::Event &event) noexcept
  {
    // If element has no href attribute, then return.
    if (!css::IsLink(*this))
    {
      return;
    }
    HyperlinkAlgorithms::FollowTheHyperlink(*this, event);
  }

  void HTMLAreaElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (css::IsLink(*this))
    {
      if (Focused() && HyperlinkAlgorithms::IsEnterKeyKeydownEvent(event))
      {
        event.SetDefaultHandled();
        (void)DispatchSimulatedClick(&event);
        return;
      }

      if (event.Type() == EventNames::AuxClick && dom::MouseEvent::CanTriggerActivationBehavior(event))
      {
        HyperlinkAlgorithms::FollowTheHyperlink(*this, event);
        return;
      }
    }

    HTMLElement::DefaultEventHandler(event);
  }

  void HTMLAreaElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value,
                                           dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"shape")
    {
      auto shape = value.value_or(dom::DOMStringView());
      if (text::IsASCIICaselessEqual(shape, utf8_stringview(u8"default")))
      {
        _shape = AreaShape::Default;
      }
      else if (text::IsASCIICaselessEqual(shape, utf8_stringview(u8"circle"))
               || text::IsASCIICaselessEqual(shape, utf8_stringview(u8"circ")))
      {
        _shape = AreaShape::Circle;
      }
      else if (text::IsASCIICaselessEqual(shape, utf8_stringview(u8"poly"))
               || text::IsASCIICaselessEqual(shape, utf8_stringview(u8"polygon")))
      {
        _shape = AreaShape::Poly;
      }
      else
      {
        _shape = AreaShape::Rect;
      }
      InvalidateCachedRegion();
    }
    else if (localName == u8"coords")
    {
      _coords = Numbers::ParseListOfFloatingPointNumbers(value.value_or(dom::DOMStringView()));
      InvalidateCachedRegion();
    }
  }

  void HTMLAreaElement::InvalidateCachedRegion() noexcept
  {
    _lastSize = LayoutSize(-1, -1);
  }

  bool HTMLAreaElement::MapMouseEvent(LayoutPoint location, const LayoutSize &size,
                                      HitTestResult &result) noexcept
  {
    if (_lastSize != size)
    {
      _region = GetRegion(size);
      _lastSize = size;
    }

    if (!_region->Contains(location, WindRule::EvenOdd))
    {
      return false;
    }

    result.SetInnerNode(this);
    result.SetURLElement(this);
    return true;
  }

  Path HTMLAreaElement::ComputePathForFocusRing(const LayoutSize &elementSize) const noexcept
  {
    return GetRegion(IsDefault() ? elementSize : _lastSize);
  }

  Path HTMLAreaElement::ComputePath(const RenderElement &renderer) const noexcept
  {
    // FIXME: This doesn't work correctly with transforms.
    FloatPoint absPos = renderer.LocalToAbsolute();

    // Default should default to the size of the containing object.
    LayoutSize size = _lastSize;
    if (IsDefault())
    {
      size = renderer.OutlineBoundsForRepaint(nullptr).Size();
    }

    auto path = GetRegion(size);
    float zoomFactor = renderer.Style().UsedZoom();
    if (zoomFactor != 1.0f)
    {
      AffineTransform zoomTransform;
      zoomTransform.Scale(zoomFactor);
      path.Transform(zoomTransform);
    }

    path.Translate(ToFloatSize(absPos));
    return path;
  }

  LayoutRect HTMLAreaElement::ComputeRect(const RenderObject *renderer) const noexcept
  {
    if (!renderer)
    {
      return {};
    }

    if (Is<RenderText>(*renderer))
    {
      krys_debug_assert(false);
      return {};
    }
    return EnclosingLayoutRect(ComputePath(Downcast<RenderElement>(*renderer)).FastBoundingRect());
  }

  Path HTMLAreaElement::GetRegion(const LayoutSize &size) const noexcept
  {
    if (_coords.empty() && !IsDefault())
    {
      return Path();
    }

    Path path;
    switch (_shape)
    {
      case AreaShape::Poly:
        if (_coords.size() >= 6)
        {
          auto numPoints = _coords.size() / 2;
          path.MoveTo(FloatPoint(static_cast<float>(_coords[0]), static_cast<float>(_coords[1])));
          for (size_t i = 1; i < numPoints; ++i)
          {
            path.AddLineTo(
              FloatPoint(static_cast<float>(_coords[i * 2]), static_cast<float>(_coords[i * 2 + 1])));
          }
          path.CloseSubpath();
        }
        break;
      case AreaShape::Circle:
        if (_coords.size() >= 3)
        {
          auto radius = static_cast<float>(_coords[2]);
          if (radius > 0)
          {
            path.AddEllipseInRect(FloatRect(static_cast<float>(_coords[0]) - radius,
                                            static_cast<float>(_coords[1]) - radius, 2 * radius, 2 * radius));
          }
        }
        break;
      case AreaShape::Rect:
        if (_coords.size() >= 4)
        {
          auto x0 = static_cast<float>(_coords[0]);
          auto y0 = static_cast<float>(_coords[1]);
          auto x1 = static_cast<float>(_coords[2]);
          auto y1 = static_cast<float>(_coords[3]);
          path.AddRect(FloatRect(x0, y0, x1 - x0, y1 - y0));
        }
        break;
      case AreaShape::Default: path.AddRect(FloatRect(FloatPoint(), FloatSize(size))); break;
    }
    return path;
  }

  RefPtr<HTMLImageElement> HTMLAreaElement::ImageElement() const noexcept
  {
    if (auto *mapElement = DynamicDowncast<HTMLMapElement>(ParentNode()))
    {
      return mapElement->ImageElement();
    }
    return nullptr;
  }

  void HTMLAreaElement::SetFocus(bool value, dom::FocusVisibility visibility) noexcept
  {
    if (Focused() == value)
    {
      return;
    }

    HTMLElement::SetFocus(value, visibility);

    auto imageElement = ImageElement();
    if (!imageElement)
    {
      return;
    }

    if (auto *renderer = DynamicDowncast<RenderImage>(imageElement->Renderer()))
    {
      renderer->AreaElementFocusChanged(*this);
    }
  }

  RefPtr<dom::Element> HTMLAreaElement::FocusAppearanceUpdateTarget() noexcept
  {
    if (!FocusAlgorithms::IsFocusableArea(*this))
    {
      return nullptr;
    }
    return ImageElement();
  }

#pragma region HTMLAreaElement

  dom::DOMString HTMLAreaElement::Alt() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLAreaElement::Alt(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"alt", krys::move(value));
  }

  dom::DOMString HTMLAreaElement::Coords() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"coords").value_or(u8"");
  }

  void HTMLAreaElement::Coords(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"coords", krys::move(value));
  }

  dom::DOMString HTMLAreaElement::Shape() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"shape").value_or(u8"");
  }

  void HTMLAreaElement::Shape(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"shape", krys::move(value));
  }

  dom::DOMString HTMLAreaElement::Target() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLAreaElement::Target(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"target", krys::move(value));
  }

  dom::DOMString HTMLAreaElement::Download() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"download").value_or(u8"");
  }

  void HTMLAreaElement::Download(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"download", krys::move(value));
  }

  dom::USVString HTMLAreaElement::Ping() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"ping");
  }

  void HTMLAreaElement::Ping(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"ping", krys::move(value));
  }

  dom::DOMString HTMLAreaElement::Rel() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLAreaElement::Rel(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"rel", krys::move(value));
  }

  dom::DOMTokenList &HTMLAreaElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"rel");
    }

    return *_relList;
  }

  dom::DOMString HTMLAreaElement::ReferrerPolicy() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLAreaElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

#pragma endregion

#pragma region HTMLHyperlinkElementUtils

  dom::USVString HTMLAreaElement::Href() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"href");
  }

  void HTMLAreaElement::Href(dom::USVString &&value) noexcept
  {
    Reflection::ReflectURL<dom::USVString>(*this, u8"href", krys::move(value));
  }

  dom::USVString HTMLAreaElement::ToString() const noexcept
  {
    return Href();
  }

  void HTMLAreaElement::Protocol(dom::DOMStringView value) noexcept
  {
    if (!FullURL().IsValid())
    {
      return;
    }
    URLDecomposition::Protocol(value);
  }

  url::URL HTMLAreaElement::FullURL() const noexcept
  {
    return HyperlinkAlgorithms::Href(*this);
  }

  void HTMLAreaElement::SetFullURL(const url::URL &url) noexcept
  {
    CEReactionsScope ceReactions;
    HyperlinkAlgorithms::SetHref(*this, url);
  }

#pragma endregion

#pragma region HTMLAreaElement Obsolete members

  bool HTMLAreaElement::NoHref() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"nohref");
  }

  void HTMLAreaElement::NoHref(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"nohref", krys::move(value));
  }

#pragma endregion
}
