/*
 * Copyright (C) 2007, 2010 Rob Buis <buis@kde.org>
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Web/Svg/SVGViewSpec.hpp"
#include "Krystal.Core/Strings/ParsingUtilities.hpp"
#include "Krystal.Web/Dom/Mixins/NonElementParentNode.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Web/Svg/SVGNames.hpp"
#include "Krystal.Web/Svg/SVGTransformList.hpp"

namespace krys::web
{
  SVGViewSpec::SVGViewSpec(SVGElement &contextElement) noexcept
      : SVGFitToViewBox(&contextElement, SVGPropertyAccess::ReadOnly),
        _contextElement(CreateWeakPtr(&contextElement)),
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
    return DynamicDowncast<SVGElement>(krys::move(element));
  }

  DOMString SVGViewSpec::TransformString() const noexcept
  {
    return _transform->ValueAsString();
  }

  void SVGViewSpec::Reset() noexcept
  {
    _viewTargetString = {};
    _transform->ClearItems();
    SVGFitToViewBox::Reset();
  }

  bool SVGViewSpec::ParseViewSpec(DOMStringView string) noexcept
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
          _viewTargetString = DOMString(viewTargetStart.substr(0, viewTargetStart.size() - buffer.size()));
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
        // `zoomAndPan` is deprecated and not supported: its value is checked and ignored.
        if (!text::SkipCharactersExactly(buffer, u8"disable")
            && !text::SkipCharactersExactly(buffer, u8"magnify"))
        {
          return false;
        }
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
