#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutAttributesBuilder.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGTextPositioningElement.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <algorithm>

namespace krys::boo
{
  SVGTextLayoutAttributesBuilder::TextPosition::TextPosition(svg::SVGTextPositioningElement *newElement,
                                                             uint32 newStart, uint32 newLength) noexcept
      : Element(ShareCheckedPtr(newElement)), Start(newStart), Length(newLength)
  {
  }

  SVGTextLayoutAttributesBuilder::SVGTextLayoutAttributesBuilder() noexcept = default;

  SVGTextLayoutAttributesBuilder::~SVGTextLayoutAttributesBuilder() noexcept = default;

  void
    SVGTextLayoutAttributesBuilder::BuildLayoutAttributesForTextRenderer(RenderSVGInlineText &text) noexcept
  {
    auto *textRoot = RenderSVGText::LocateRenderSVGTextAncestor(text);
    if (!textRoot)
    {
      return;
    }

    if (_textPositions.empty())
    {
      _characterDataMap.clear();

      _textLength = 0;
      bool lastCharacterWasSpace = true;
      CollectTextPositioningElements(*textRoot, lastCharacterWasSpace);

      if (!_textLength)
      {
        return;
      }

      BuildCharacterDataMap(*textRoot);
    }

    _metricsBuilder.BuildMetricsAndLayoutAttributes(*textRoot, &text, _characterDataMap);
  }

  bool SVGTextLayoutAttributesBuilder::BuildLayoutAttributesForForSubtree(RenderSVGText &textRoot) noexcept
  {
    _characterDataMap.clear();

    if (_textPositions.empty())
    {
      _textLength = 0;
      bool lastCharacterWasSpace = true;
      CollectTextPositioningElements(textRoot, lastCharacterWasSpace);
    }

    if (!_textLength)
    {
      return false;
    }

    BuildCharacterDataMap(textRoot);
    _metricsBuilder.BuildMetricsAndLayoutAttributes(textRoot, nullptr, _characterDataMap);
    return true;
  }

  void SVGTextLayoutAttributesBuilder::RebuildMetricsForSubtree(RenderSVGText &text) noexcept
  {
    _metricsBuilder.MeasureTextRenderer(text, nullptr);
  }

  namespace
  {
    void ProcessRenderSVGInlineText(const RenderSVGInlineText &text, uint32 &atCharacter,
                                    bool &lastCharacterWasSpace) noexcept
    {
      auto string = text.Text();
      if (text.Style().GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve)
      {
        atCharacter += static_cast<uint32>(std::ranges::count_if(
          string, [](char8 character) { return !krys::text::IsTrailingUTF8(character); }));
        return;
      }

      // FIXME: This is not a complete whitespace collapsing implementation; it doesn't handle newlines or
      // tabs.
      for (auto character : string)
      {
        if (krys::text::IsTrailingUTF8(character))
        {
          continue;
        }

        if (character == u8' ' && lastCharacterWasSpace)
        {
          continue;
        }

        lastCharacterWasSpace = character == u8' ';
        ++atCharacter;
      }
    }
  }

  void SVGTextLayoutAttributesBuilder::CollectTextPositioningElements(RenderBoxModelObject &start,
                                                                      bool &lastCharacterWasSpace) noexcept
  {
    krys_debug_assert(!Is<RenderSVGText>(start) || _textPositions.empty());

    for (auto &child : ChildrenOfType<RenderObject>(start))
    {
      if (auto *inlineText = DynamicDowncast<RenderSVGInlineText>(child))
      {
        ProcessRenderSVGInlineText(*inlineText, _textLength, lastCharacterWasSpace);
        continue;
      }

      auto *inlineChild = DynamicDowncast<RenderSVGInline>(child);
      if (!inlineChild)
      {
        continue;
      }

      RefPtr element = svg::SVGTextPositioningElement::ElementFromRenderer(*inlineChild);

      size_t atPosition = _textPositions.size();
      if (element)
      {
        _textPositions.push_back(TextPosition(element.get(), _textLength));
      }

      CollectTextPositioningElements(*inlineChild, lastCharacterWasSpace);

      if (!element)
      {
        continue;
      }

      // Update text position, after we're back from recursion.
      TextPosition &position = _textPositions[atPosition];
      krys_debug_assert(!position.Length);
      position.Length = _textLength - position.Start;
    }
  }

  void SVGTextLayoutAttributesBuilder::BuildCharacterDataMap(RenderSVGText &textRoot) noexcept
  {
    RefPtr outermostTextElement = svg::SVGTextPositioningElement::ElementFromRenderer(textRoot);
    krys_debug_assert(outermostTextElement);

    // Grab outermost <text> element value lists and insert them in the character data map.
    TextPosition wholeTextPosition(outermostTextElement.get(), 0, _textLength);
    FillCharacterDataMap(wholeTextPosition);

    // Fill character data map using child text positioning elements in top-down order.
    for (auto &position : _textPositions)
    {
      FillCharacterDataMap(position);
    }

    // Handle x/y default attributes.
    auto [iterator, isNewEntry] = _characterDataMap.try_emplace(1, SVGCharacterData {.X = 0, .Y = 0});
    if (!isNewEntry)
    {
      SVGCharacterData &data = iterator->second;
      if (SVGTextLayoutAttributes::IsEmptyValue(data.X))
      {
        data.X = 0;
      }
      if (SVGTextLayoutAttributes::IsEmptyValue(data.Y))
      {
        data.Y = 0;
      }
    }
  }

  void SVGTextLayoutAttributesBuilder::FillCharacterDataMap(const TextPosition &position) noexcept
  {
    RefPtr element = ShareRefPtr(position.Element.get());
    const auto &xList = element->X();
    const auto &yList = element->Y();
    const auto &dxList = element->Dx();
    const auto &dyList = element->Dy();
    const auto &rotateList = element->Rotate();

    auto xListSize = static_cast<uint32>(xList.Items().size());
    auto yListSize = static_cast<uint32>(yList.Items().size());
    auto dxListSize = static_cast<uint32>(dxList.Items().size());
    auto dyListSize = static_cast<uint32>(dyList.Items().size());
    auto rotateListSize = static_cast<uint32>(rotateList.Items().size());
    if (!xListSize && !yListSize && !dxListSize && !dyListSize && !rotateListSize)
    {
      return;
    }

    svg::SVGLengthContext lengthContext(element.get());
    for (uint32 i = 0; i < position.Length; ++i)
    {
      bool hasX = i < xListSize;
      bool hasY = i < yListSize;
      bool hasDx = i < dxListSize;
      bool hasDy = i < dyListSize;
      bool hasRotate = rotateListSize;
      if (!hasX && !hasY && !hasDx && !hasDy && !hasRotate)
      {
        break;
      }

      auto &data = _characterDataMap[position.Start + i + 1];

      if (hasX)
      {
        data.X = xList.Items()[i]->Value().Value(lengthContext);
      }
      if (hasY)
      {
        data.Y = yList.Items()[i]->Value().Value(lengthContext);
      }
      if (hasDx)
      {
        data.Dx = dxList.Items()[i]->Value().Value(lengthContext);
      }
      if (hasDy)
      {
        data.Dy = dyList.Items()[i]->Value().Value(lengthContext);
      }

      if (hasRotate)
      {
        uint32 rotateIndex = std::min(i, rotateListSize - 1);
        data.Rotate = rotateList.Items()[rotateIndex]->Value();
      }
    }
  }
}
