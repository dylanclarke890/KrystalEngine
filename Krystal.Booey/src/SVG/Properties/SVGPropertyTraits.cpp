/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorInlines.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
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
          return renderer->Style().ColorResolvingCurrentColor(css::PropertyId::Color);
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
