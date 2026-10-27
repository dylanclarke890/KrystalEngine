#include "Krystal.Booey/SVG/SVGViewSpec.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGTransformList.hpp"
#include "Krystal.Core/Text/ParsingUtilities.hpp"

namespace krys::boo::svg
{
  SVGViewSpec::SVGViewSpec(SVGElement &contextElement) noexcept
      : SVGFitToViewBox(&contextElement, SVGPropertyAccess::ReadOnly),
        _contextElement(WeakPtr<SVGElement>::WithRef(contextElement)),
        _transform(SVGTransformList::Create(&contextElement, SVGPropertyAccess::ReadOnly))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGViewSpec::_transform>(SVGNames::TransformAttr());
    }
  }

  RefPtr<SVGElement> SVGViewSpec::ViewTarget() const noexcept
  {
    auto *contextElement = _contextElement.get();
    if (!contextElement)
    {
      return nullptr;
    }
    auto element = dom::mixins::NonElementParentNode::GetElementById(contextElement->TreeScope().RootNode(),
                                                                     _viewTargetString);
    return element ? ShareRefPtr(DynamicDowncast<SVGElement>(*element)) : nullptr;
  }

  dom::DOMString SVGViewSpec::TransformString() const noexcept
  {
    return _transform->ValueAsString();
  }

  void SVGViewSpec::Reset() noexcept
  {
    _viewTargetString = {};
    _transform->ClearItems();
    SVGFitToViewBox::Reset();
    SVGZoomAndPan::Reset();
  }

  bool SVGViewSpec::ParseViewSpec(dom::DOMStringView string) noexcept
  {
    auto buffer = string;
    if (buffer.empty() || !_contextElement)
    {
      return false;
    }

    if (!text::SkipCharactersExactly(buffer, u8"svgView"))
    {
      return false;
    }

    if (!text::SkipExactly(buffer, u8'('))
    {
      return false;
    }

    while (!buffer.empty() && buffer.front() != u8')')
    {
      if (buffer.front() == u8'v')
      {
        if (text::SkipCharactersExactly(buffer, u8"viewBox"))
        {
          if (!text::SkipExactly(buffer, u8'('))
          {
            return false;
          }
          auto viewBox = SVGFitToViewBox::ParseViewBox(buffer, false);
          if (!viewBox)
          {
            return false;
          }
          SetViewBox(krys::move(*viewBox));
          if (!text::SkipExactly(buffer, u8')'))
          {
            return false;
          }
        }
        else if (text::SkipCharactersExactly(buffer, u8"viewTarget"))
        {
          if (!text::SkipExactly(buffer, u8'('))
          {
            return false;
          }
          auto viewTargetStart = buffer;
          text::SkipUntil(buffer, u8')');
          if (buffer.empty())
          {
            return false;
          }
          _viewTargetString =
            dom::DOMString(viewTargetStart.substr(0, viewTargetStart.size() - buffer.size()));
          buffer.remove_prefix(1);
        }
        else
        {
          return false;
        }
      }
      else if (buffer.front() == u8'z')
      {
        if (!text::SkipCharactersExactly(buffer, u8"zoomAndPan"))
        {
          return false;
        }
        if (!text::SkipExactly(buffer, u8'('))
        {
          return false;
        }
        auto zoomAndPan = SVGZoomAndPan::ParseZoomAndPan(buffer);
        if (!zoomAndPan)
        {
          return false;
        }
        SetZoomAndPan(*zoomAndPan);
        if (!text::SkipExactly(buffer, u8')'))
        {
          return false;
        }
      }
      else if (buffer.front() == u8'p')
      {
        if (!text::SkipCharactersExactly(buffer, u8"preserveAspectRatio"))
        {
          return false;
        }
        if (!text::SkipExactly(buffer, u8'('))
        {
          return false;
        }
        SVGPreserveAspectRatioValue preserveAspectRatio;
        if (!preserveAspectRatio.Parse(buffer, false))
        {
          return false;
        }
        SetPreserveAspectRatio(preserveAspectRatio);
        if (!text::SkipExactly(buffer, u8')'))
        {
          return false;
        }
      }
      else if (buffer.front() == u8't')
      {
        if (!text::SkipCharactersExactly(buffer, u8"transform"))
        {
          return false;
        }
        if (!text::SkipExactly(buffer, u8'('))
        {
          return false;
        }
        (void)_transform->ParseGeneric(buffer);
        if (!text::SkipExactly(buffer, u8')'))
        {
          return false;
        }
      }
      else
      {
        return false;
      }

      (void)text::SkipExactly(buffer, u8';');
    }

    if (buffer.empty() || buffer.front() != u8')')
    {
      return false;
    }

    return true;
  }
}
