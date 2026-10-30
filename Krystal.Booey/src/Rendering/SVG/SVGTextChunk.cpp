#include "Krystal.Booey/Rendering/SVG/SVGTextChunk.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGTextContentElement.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <algorithm>
#include <ranges>

namespace krys::boo
{
  namespace
  {
    /// @brief The number of characters (code points) of @p text that @p fragment covers.
    KRYS_NODISCARD uint32 NumberOfCharacters(utf8_stringview text, const SVGTextFragment &fragment) noexcept
    {
      return static_cast<uint32>(std::ranges::count_if(text.substr(fragment.CharacterOffset, fragment.Length),
                                                       [](char8 c)
                                                       { return !krys::text::IsTrailingUTF8(c); }));
    }
  }

  SVGTextChunk::SVGTextChunk(const List<inline_iterator::SVGTextBoxIterator> &lineLayoutBoxes, uint32 first,
                             uint32 limit, SVGTextFragmentMap &fragmentMap) noexcept
  {
    krys_debug_assert(first < limit);
    krys_debug_assert(limit <= lineLayoutBoxes.size());

    auto firstBox = lineLayoutBoxes[first];
    auto &style = firstBox->Renderer().Style();

    if (style.GetWritingMode().IsBidiRTL())
    {
      _chunkStyle |= SVGTextChunk::RightToLeftText;
    }

    if (style.GetWritingMode().IsVertical())
    {
      _chunkStyle |= SVGTextChunk::VerticalText;
    }

    switch (style.GetTextAnchor())
    {
      case TextAnchor::Start:  break;
      case TextAnchor::Middle: _chunkStyle |= MiddleAnchor; break;
      case TextAnchor::End:    _chunkStyle |= EndAnchor; break;
    }

    if (RefPtr textContentElement =
          ShareRefPtr(svg::SVGTextContentElement::ElementFromRenderer(firstBox->Renderer().Parent())))
    {
      svg::SVGLengthContext lengthContext(textContentElement.get());
      _desiredTextLength = textContentElement->SpecifiedTextLength().Value(lengthContext);

      switch (textContentElement->LengthAdjust())
      {
        case svg::SVGLengthAdjustUnknown:          break;
        case svg::SVGLengthAdjustSpacing:          _chunkStyle |= LengthAdjustSpacing; break;
        case svg::SVGLengthAdjustSpacingAndGlyphs: _chunkStyle |= LengthAdjustSpacingAndGlyphs; break;
      }
    }

    for (uint32 i = first; i < limit; ++i)
    {
      auto &box = lineLayoutBoxes[i];
      auto it = fragmentMap.find(inline_iterator::MakeKey(*box));
      if (it == fragmentMap.end())
      {
        continue;
      }
      _boxes.push_back({box, &it->second});
    }
  }

  uint32 SVGTextChunk::TotalCharacters() const noexcept
  {
    uint32 characters = 0;
    for (auto &box : _boxes)
    {
      auto text = box.Box->Renderer().Text();
      for (auto &fragment : *box.Fragments)
      {
        characters += NumberOfCharacters(text, fragment);
      }
    }
    return characters;
  }

  float SVGTextChunk::TotalLength() const noexcept
  {
    const SVGTextFragment *firstFragment = nullptr;
    const SVGTextFragment *lastFragment = nullptr;

    for (auto &box : _boxes)
    {
      if (box.Fragments->size())
      {
        firstFragment = &box.Fragments->front();
        break;
      }
    }

    for (auto &box : _boxes | std::views::reverse)
    {
      if (box.Fragments->size())
      {
        lastFragment = &box.Fragments->back();
        break;
      }
    }

    krys_debug_assert(!firstFragment == !lastFragment);
    if (!firstFragment)
    {
      return 0;
    }

    if (_chunkStyle & VerticalText)
    {
      return (lastFragment->Y + lastFragment->Height) - firstFragment->Y;
    }

    return (lastFragment->X + lastFragment->Width) - firstFragment->X;
  }

  float SVGTextChunk::TotalAnchorShift() const noexcept
  {
    float length = TotalLength();
    if (_chunkStyle & MiddleAnchor)
    {
      return -length / 2;
    }
    if (_chunkStyle & EndAnchor)
    {
      return _chunkStyle & RightToLeftText ? 0 : -length;
    }
    return _chunkStyle & RightToLeftText ? -length : 0;
  }

  void SVGTextChunk::Layout(SVGChunkTransformMap &textBoxTransformations) const noexcept
  {
    if (HasDesiredTextLength())
    {
      if (HasLengthAdjustSpacing())
      {
        ProcessTextLengthSpacingCorrection();
      }
      else
      {
        krys_debug_assert(HasLengthAdjustSpacingAndGlyphs());
        BuildBoxTransformations(textBoxTransformations);
      }
    }

    if (HasTextAnchor())
    {
      ProcessTextAnchorCorrection();
    }
  }

  void SVGTextChunk::ProcessTextLengthSpacingCorrection() const noexcept
  {
    float textLengthShift = 0;
    auto totalCharacters = TotalCharacters();
    if (totalCharacters > 1)
    {
      textLengthShift = (DesiredTextLength() - TotalLength()) / (totalCharacters - 1);
    }

    bool isVerticalText = _chunkStyle & VerticalText;
    uint32 atCharacter = 0;
    for (auto &box : _boxes)
    {
      auto text = box.Box->Renderer().Text();
      for (auto &fragment : *box.Fragments)
      {
        if (isVerticalText)
        {
          fragment.Y += textLengthShift * atCharacter;
        }
        else
        {
          fragment.X += textLengthShift * atCharacter;
        }

        atCharacter += NumberOfCharacters(text, fragment);
      }
    }
  }

  void SVGTextChunk::BuildBoxTransformations(SVGChunkTransformMap &textBoxTransformations) const noexcept
  {
    AffineTransform spacingAndGlyphsTransform;
    bool foundFirstFragment = false;
    for (auto &box : _boxes)
    {
      if (!foundFirstFragment)
      {
        if (!BoxSpacingAndGlyphsTransform(*box.Fragments, spacingAndGlyphsTransform))
        {
          continue;
        }
        foundFirstFragment = true;
      }

      textBoxTransformations[inline_iterator::MakeKey(*box.Box)] = spacingAndGlyphsTransform;
    }
  }

  bool SVGTextChunk::BoxSpacingAndGlyphsTransform(const List<SVGTextFragment> &fragments,
                                                  AffineTransform &spacingAndGlyphsTransform) const noexcept
  {
    if (fragments.empty())
    {
      return false;
    }

    const SVGTextFragment &fragment = fragments.front();
    float scale = DesiredTextLength() / TotalLength();

    spacingAndGlyphsTransform.Translate(fragment.X, fragment.Y);

    if (_chunkStyle & VerticalText)
    {
      spacingAndGlyphsTransform.ScaleNonUniform(1, scale);
    }
    else
    {
      spacingAndGlyphsTransform.ScaleNonUniform(scale, 1);
    }

    spacingAndGlyphsTransform.Translate(-fragment.X, -fragment.Y);
    return true;
  }

  void SVGTextChunk::ProcessTextAnchorCorrection() const noexcept
  {
    float textAnchorShift = TotalAnchorShift();
    bool isVerticalText = _chunkStyle & VerticalText;
    for (auto &box : _boxes)
    {
      for (auto &fragment : *box.Fragments)
      {
        if (isVerticalText)
        {
          fragment.Y += textAnchorShift;
        }
        else
        {
          fragment.X += textAnchorShift;
        }
      }
    }
  }
}
