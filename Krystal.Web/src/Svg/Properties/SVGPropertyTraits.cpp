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

#include "Krystal.Web/Svg/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Core/Strings/StringToIntegerConversion.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "Krystal.Web/Css/Properties/Consumers/Color.hpp"
#include "Krystal.Web/Css/Properties/Consumers/ColorInlines.hpp"
#include "Krystal.Web/Css/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"

namespace krys::web
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

  Color SVGPropertyTraits<Color>::FromString(SVGElement &targetElement, DOMStringView string) noexcept
  {
    using namespace css::parse_property;

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

  int32 SVGPropertyTraits<int32>::FromString(SVGElement &, DOMStringView string) noexcept
  {
    return text::ParseInteger<int32>(string).value_or(0);
  }

  DOMString SVGPropertyTraits<Color>::ToString(const Color &type) noexcept
  {
    return SerialisationForHTML(type);
  }
}
