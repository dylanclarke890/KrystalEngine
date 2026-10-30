#include "Krystal.Booey/Rendering/SVG/SVGTextQuery.hpp"
#include "Krystal.Booey/Rendering/LegacyInlineFlowBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextBoxPainter.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys::boo
{
  // Base structure for callback user data
  struct SVGTextQuery::Data
  {
    bool IsVerticalText {false};
    uint32 ProcessedCharacters {0};
    const RenderSVGInlineText *TextRenderer {nullptr};
    inline_iterator::SVGTextBoxIterator TextBox;
  };

  namespace
  {
    KRYS_NODISCARD inline_iterator::InlineBoxIterator InlineBoxForRenderer(RenderObject *renderer) noexcept
    {
      if (!renderer)
      {
        return {};
      }

      if (auto *renderBlock = DynamicDowncast<RenderBlockFlow>(*renderer))
      {
        // If we're given a block element, it has to be a RenderSVGText.
        krys_debug_assert(Is<RenderSVGText>(*renderBlock));
        return inline_iterator::FirstRootInlineBoxFor(*renderBlock);
      }

      if (auto *renderInline = DynamicDowncast<RenderInline>(*renderer))
      {
        return inline_iterator::LineLeftmostInlineBoxFor(*renderInline);
      }

      krys_debug_assert(false);
      return {};
    }

    /// @brief The length in bytes of the character at byte @p offset of @p renderer's text.
    KRYS_NODISCARD uint32 CharacterLengthAt(const RenderSVGInlineText &renderer, uint32 offset) noexcept
    {
      auto text = renderer.Text();
      return krys::text::DecodeUTF8(Span<const char8>(text.data() + offset, text.size() - offset)).Length;
    }
  }

  SVGTextQuery::SVGTextQuery(RenderObject *renderer) noexcept
  {
    CollectTextBoxesInInlineBox(InlineBoxForRenderer(renderer));
  }

  void SVGTextQuery::CollectTextBoxesInInlineBox(inline_iterator::InlineBoxIterator parent) noexcept
  {
    if (!parent)
    {
      return;
    }

    auto [begin, end] = parent->Descendants();
    for (auto box = begin; box != end;)
    {
      if (auto *flowBox = DynamicDowncast<inline_iterator::InlineBox>(*box))
      {
        // Skip generated content.
        if (!flowBox->Renderer().GetElement())
        {
          box.TraverseLineRightwardOnLineSkippingChildren();
        }
        else
        {
          box.TraverseLineRightwardOnLine();
        }
        continue;
      }

      if (auto *inlineTextBox = DynamicDowncast<inline_iterator::SVGTextBox>(*box))
      {
        _textBoxes.push_back(inline_iterator::SVGTextBoxIterator {*inlineTextBox});
      }

      box.TraverseLineRightwardOnLine();
    }
  }

  bool SVGTextQuery::ExecuteQuery(Data *queryData,
                                  ProcessTextFragmentCallback fragmentCallback) const noexcept
  {
    uint32 processedCharacters = 0;

    // Loop over all text boxes
    for (auto &textBox : _textBoxes)
    {
      queryData->TextBox = textBox;
      queryData->TextRenderer = &queryData->TextBox->Renderer();

      queryData->IsVerticalText = queryData->TextRenderer->GetWritingMode().IsVertical();
      const List<SVGTextFragment> &fragments = queryData->TextBox->TextFragments();

      // Loop over all text fragments in this text box, firing a callback for each.
      for (auto &fragment : fragments)
      {
        if ((this->*fragmentCallback)(queryData, fragment))
        {
          return true;
        }

        processedCharacters += fragment.Length;
      }

      queryData->ProcessedCharacters = processedCharacters;
    }

    return false;
  }

  bool SVGTextQuery::MapStartEndPositionsIntoFragmentCoordinates(Data *queryData,
                                                                 const SVGTextFragment &fragment,
                                                                 uint32 &startPosition,
                                                                 uint32 &endPosition) const noexcept
  {
    // Reuse the same logic used for text selection & painting, to map our query start/length into
    // start/endPositions of the current text fragment.
    if (startPosition < queryData->ProcessedCharacters || endPosition < queryData->ProcessedCharacters)
    {
      return false;
    }
    startPosition -= queryData->ProcessedCharacters;
    endPosition -= queryData->ProcessedCharacters;

    // <startPosition, endPosition> is now a tuple of offsets relative to the current text box.
    // Compute the offsets of the fragment in the same offset space.
    uint32 fragmentStartInBox = fragment.CharacterOffset - queryData->TextBox->Start();
    uint32 fragmentEndInBox = fragmentStartInBox + fragment.Length;

    // Check if the ranges intersect.
    startPosition = std::max<uint32>(startPosition, fragmentStartInBox);
    endPosition = std::min<uint32>(endPosition, fragmentEndInBox);

    if (startPosition >= endPosition)
    {
      return false;
    }

    ModifyStartEndPositionsRespectingLigatures(queryData, fragment, startPosition, endPosition);
    if (!boo::MapStartEndPositionsIntoFragmentCoordinates(queryData->TextBox->Start(), fragment,
                                                          startPosition, endPosition))
    {
      return false;
    }

    krys_assert(startPosition < endPosition);
    return true;
  }

  void SVGTextQuery::ModifyStartEndPositionsRespectingLigatures(Data *queryData,
                                                                const SVGTextFragment &fragment,
                                                                uint32 &startPosition,
                                                                uint32 &endPosition) const noexcept
  {
    const SVGTextLayoutAttributes *layoutAttributes = queryData->TextRenderer->LayoutAttributes();
    const List<SVGTextMetrics> &textMetricsValues = layoutAttributes->TextMetricsValues();

    uint32 textMetricsOffset = fragment.MetricsListOffset;

    // Compute the offset of the fragment within the box, since that's the
    // space <startPosition, endPosition> is in (and that's what we need).
    uint32 fragmentOffsetInBox = fragment.CharacterOffset - queryData->TextBox->Start();
    uint32 fragmentEndInBox = fragmentOffsetInBox + fragment.Length;

    // Find the text metrics cell that start at or contain the character startPosition.
    while (fragmentOffsetInBox < fragmentEndInBox)
    {
      auto &metrics = textMetricsValues[textMetricsOffset];
      uint32 glyphEnd = fragmentOffsetInBox + metrics.Length();
      if (startPosition < glyphEnd)
      {
        break;
      }
      fragmentOffsetInBox = glyphEnd;
      ++textMetricsOffset;
    }

    startPosition = fragmentOffsetInBox;

    // Find the text metrics cell that contain or ends at the character endPosition.
    while (fragmentOffsetInBox < fragmentEndInBox)
    {
      auto &metrics = textMetricsValues[textMetricsOffset];
      fragmentOffsetInBox += metrics.Length();
      if (fragmentOffsetInBox >= endPosition)
      {
        break;
      }
      ++textMetricsOffset;
    }

    endPosition = fragmentOffsetInBox;
  }

  // NumberOfCharacters() implementation
  bool SVGTextQuery::NumberOfCharactersCallback(Data *, const SVGTextFragment &) const noexcept
  {
    // no-op
    return false;
  }

  uint32 SVGTextQuery::NumberOfCharacters() const noexcept
  {
    Data data;
    ExecuteQuery(&data, &SVGTextQuery::NumberOfCharactersCallback);
    return data.ProcessedCharacters;
  }

  // TextLength() implementation
  namespace
  {
    struct TextLengthData : SVGTextQuery::Data
    {
      float TextLength {0};
    };
  }

  bool SVGTextQuery::TextLengthCallback(Data *queryData, const SVGTextFragment &fragment) const noexcept
  {
    auto *data = static_cast<TextLengthData *>(queryData);
    data->TextLength += queryData->IsVerticalText ? fragment.Height : fragment.Width;
    return false;
  }

  float SVGTextQuery::TextLength() const noexcept
  {
    if (_textBoxes.empty())
    {
      return 0;
    }

    TextLengthData data;
    ExecuteQuery(&data, &SVGTextQuery::TextLengthCallback);
    return data.TextLength;
  }

  // SubStringLength() implementation
  namespace
  {
    struct SubStringLengthData : SVGTextQuery::Data
    {
      SubStringLengthData(uint32 queryStartPosition, uint32 queryLength) noexcept
          : StartPosition(queryStartPosition), Length(queryLength)
      {
      }

      uint32 StartPosition;
      uint32 Length;

      float SubStringLength {0};
    };
  }

  bool SVGTextQuery::SubStringLengthCallback(Data *queryData, const SVGTextFragment &fragment) const noexcept
  {
    auto *data = static_cast<SubStringLengthData *>(queryData);

    uint32 startPosition = data->StartPosition;
    uint32 endPosition = startPosition + data->Length;
    if (!MapStartEndPositionsIntoFragmentCoordinates(queryData, fragment, startPosition, endPosition))
    {
      return false;
    }

    SVGTextMetrics metrics = SVGTextMetrics::MeasureCharacterRange(
      *queryData->TextRenderer, fragment.CharacterOffset + startPosition, endPosition - startPosition);
    data->SubStringLength += queryData->IsVerticalText ? metrics.Height() : metrics.Width();
    return false;
  }

  float SVGTextQuery::SubStringLength(uint32 startPosition, uint32 length) const noexcept
  {
    SubStringLengthData data(startPosition, length);
    ExecuteQuery(&data, &SVGTextQuery::SubStringLengthCallback);
    return data.SubStringLength;
  }

  // StartPositionOfCharacter() implementation
  namespace
  {
    struct StartPositionOfCharacterData : SVGTextQuery::Data
    {
      explicit StartPositionOfCharacterData(uint32 queryPosition) noexcept : Position(queryPosition)
      {
      }

      uint32 Position;
      FloatPoint StartPosition;
    };
  }

  bool SVGTextQuery::StartPositionOfCharacterCallback(Data *queryData,
                                                      const SVGTextFragment &fragment) const noexcept
  {
    auto *data = static_cast<StartPositionOfCharacterData *>(queryData);

    uint32 startPosition = data->Position;
    uint32 endPosition = startPosition + 1;
    if (!MapStartEndPositionsIntoFragmentCoordinates(queryData, fragment, startPosition, endPosition))
    {
      return false;
    }

    data->StartPosition = FloatPoint(fragment.X, fragment.Y);

    if (startPosition)
    {
      SVGTextMetrics metrics = SVGTextMetrics::MeasureCharacterRange(*queryData->TextRenderer,
                                                                     fragment.CharacterOffset, startPosition);
      if (queryData->IsVerticalText)
      {
        data->StartPosition.Move(0, metrics.Height());
      }
      else
      {
        data->StartPosition.Move(metrics.Width(), 0);
      }
    }

    AffineTransform fragmentTransform;
    fragment.BuildFragmentTransform(fragmentTransform,
                                    SVGTextFragment::TransformType::TransformIgnoringTextLength);
    if (fragmentTransform.IsIdentity())
    {
      return true;
    }

    data->StartPosition = fragmentTransform.MapPoint(data->StartPosition);
    return true;
  }

  FloatPoint SVGTextQuery::StartPositionOfCharacter(uint32 position) const noexcept
  {
    StartPositionOfCharacterData data(position);
    ExecuteQuery(&data, &SVGTextQuery::StartPositionOfCharacterCallback);
    return data.StartPosition;
  }

  // EndPositionOfCharacter() implementation
  namespace
  {
    struct EndPositionOfCharacterData : SVGTextQuery::Data
    {
      explicit EndPositionOfCharacterData(uint32 queryPosition) noexcept : Position(queryPosition)
      {
      }

      uint32 Position;
      FloatPoint EndPosition;
    };
  }

  bool SVGTextQuery::EndPositionOfCharacterCallback(Data *queryData,
                                                    const SVGTextFragment &fragment) const noexcept
  {
    auto *data = static_cast<EndPositionOfCharacterData *>(queryData);

    uint32 startPosition = data->Position;
    uint32 endPosition = startPosition + 1;
    if (!MapStartEndPositionsIntoFragmentCoordinates(queryData, fragment, startPosition, endPosition))
    {
      return false;
    }

    data->EndPosition = FloatPoint(fragment.X, fragment.Y);

    uint32 characterLength =
      CharacterLengthAt(*queryData->TextRenderer, fragment.CharacterOffset + startPosition);
    SVGTextMetrics metrics = SVGTextMetrics::MeasureCharacterRange(
      *queryData->TextRenderer, fragment.CharacterOffset, startPosition + characterLength);
    if (queryData->IsVerticalText)
    {
      data->EndPosition.Move(0, metrics.Height());
    }
    else
    {
      data->EndPosition.Move(metrics.Width(), 0);
    }

    AffineTransform fragmentTransform;
    fragment.BuildFragmentTransform(fragmentTransform,
                                    SVGTextFragment::TransformType::TransformIgnoringTextLength);
    if (fragmentTransform.IsIdentity())
    {
      return true;
    }

    data->EndPosition = fragmentTransform.MapPoint(data->EndPosition);
    return true;
  }

  FloatPoint SVGTextQuery::EndPositionOfCharacter(uint32 position) const noexcept
  {
    EndPositionOfCharacterData data(position);
    ExecuteQuery(&data, &SVGTextQuery::EndPositionOfCharacterCallback);
    return data.EndPosition;
  }

  // RotationOfCharacter() implementation
  namespace
  {
    struct RotationOfCharacterData : SVGTextQuery::Data
    {
      explicit RotationOfCharacterData(uint32 queryPosition) noexcept : Position(queryPosition)
      {
      }

      uint32 Position;
      float Rotation {0};
    };
  }

  bool SVGTextQuery::RotationOfCharacterCallback(Data *queryData,
                                                 const SVGTextFragment &fragment) const noexcept
  {
    auto *data = static_cast<RotationOfCharacterData *>(queryData);

    uint32 startPosition = data->Position;
    uint32 endPosition = startPosition + 1;
    if (!MapStartEndPositionsIntoFragmentCoordinates(queryData, fragment, startPosition, endPosition))
    {
      return false;
    }

    AffineTransform fragmentTransform;
    fragment.BuildFragmentTransform(fragmentTransform,
                                    SVGTextFragment::TransformType::TransformIgnoringTextLength);
    if (fragmentTransform.IsIdentity())
    {
      data->Rotation = 0;
    }
    else
    {
      fragmentTransform.Scale(1 / fragmentTransform.XScale(), 1 / fragmentTransform.YScale());
      data->Rotation =
        static_cast<float>(std::atan2(fragmentTransform.B(), fragmentTransform.A()) * 180 / std::numbers::pi);
    }

    return true;
  }

  float SVGTextQuery::RotationOfCharacter(uint32 position) const noexcept
  {
    RotationOfCharacterData data(position);
    ExecuteQuery(&data, &SVGTextQuery::RotationOfCharacterCallback);
    return data.Rotation;
  }

  // ExtentOfCharacter() implementation
  namespace
  {
    struct ExtentOfCharacterData : SVGTextQuery::Data
    {
      explicit ExtentOfCharacterData(uint32 queryPosition) noexcept : Position(queryPosition)
      {
      }

      uint32 Position;
      FloatRect Extent;
    };

    void CalculateGlyphBoundaries(SVGTextQuery::Data *queryData, const SVGTextFragment &fragment,
                                  uint32 startPosition, FloatRect &extent) noexcept
    {
      float scalingFactor = queryData->TextRenderer->ScalingFactor();
      krys_debug_assert(scalingFactor);

      extent.SetLocation(FloatPoint(
        fragment.X,
        fragment.Y - queryData->TextRenderer->ScaledFont().MetricsOfPrimaryFont().Ascent() / scalingFactor));

      if (startPosition)
      {
        SVGTextMetrics metrics = SVGTextMetrics::MeasureCharacterRange(
          *queryData->TextRenderer, fragment.CharacterOffset, startPosition);
        if (queryData->IsVerticalText)
        {
          extent.Move(0, metrics.Height());
        }
        else
        {
          extent.Move(metrics.Width(), 0);
        }
      }

      uint32 characterOffset = fragment.CharacterOffset + startPosition;
      SVGTextMetrics metrics =
        SVGTextMetrics::MeasureCharacterRange(*queryData->TextRenderer, characterOffset,
                                              CharacterLengthAt(*queryData->TextRenderer, characterOffset));
      extent.SetSize(FloatSize(metrics.Width(), metrics.Height()));

      AffineTransform fragmentTransform;
      fragment.BuildFragmentTransform(fragmentTransform,
                                      SVGTextFragment::TransformType::TransformIgnoringTextLength);
      if (fragmentTransform.IsIdentity())
      {
        return;
      }

      extent = fragmentTransform.MapRect(extent);
    }

    KRYS_NODISCARD FloatRect CalculateFragmentBoundaries(const RenderSVGInlineText &textRenderer,
                                                         const SVGTextFragment &fragment) noexcept
    {
      float scalingFactor = textRenderer.ScalingFactor();
      krys_debug_assert(scalingFactor);

      float baseline = textRenderer.ScaledFont().MetricsOfPrimaryFont().Ascent() / scalingFactor;

      AffineTransform fragmentTransform;
      FloatRect fragmentRect(fragment.X, fragment.Y - baseline, fragment.Width, fragment.Height);
      fragment.BuildFragmentTransform(fragmentTransform);
      return fragmentTransform.MapRect(fragmentRect);
    }
  }

  bool SVGTextQuery::ExtentOfCharacterCallback(Data *queryData,
                                               const SVGTextFragment &fragment) const noexcept
  {
    auto *data = static_cast<ExtentOfCharacterData *>(queryData);

    uint32 startPosition = data->Position;
    uint32 endPosition = startPosition + 1;
    if (!MapStartEndPositionsIntoFragmentCoordinates(queryData, fragment, startPosition, endPosition))
    {
      return false;
    }

    CalculateGlyphBoundaries(queryData, fragment, startPosition, data->Extent);
    return true;
  }

  FloatRect SVGTextQuery::ExtentOfCharacter(uint32 position) const noexcept
  {
    ExtentOfCharacterData data(position);
    ExecuteQuery(&data, &SVGTextQuery::ExtentOfCharacterCallback);
    return data.Extent;
  }

  // CharacterNumberAtPosition() implementation
  namespace
  {
    struct CharacterNumberAtPositionData : SVGTextQuery::Data
    {
      explicit CharacterNumberAtPositionData(const FloatPoint &queryPosition) noexcept
          : Position(queryPosition)
      {
      }

      FloatPoint Position;
    };
  }

  bool SVGTextQuery::CharacterNumberAtPositionCallback(Data *queryData,
                                                       const SVGTextFragment &fragment) const noexcept
  {
    auto *data = static_cast<CharacterNumberAtPositionData *>(queryData);

    // Test the query point against the bounds of the entire fragment first.
    FloatRect fragmentExtents = CalculateFragmentBoundaries(*queryData->TextRenderer, fragment);
    if (!fragmentExtents.Contains(data->Position))
    {
      return false;
    }

    // Iterate through the glyphs in this fragment, and check if their extents
    // contain the query point.
    FloatRect extent;
    auto &textMetrics = queryData->TextRenderer->LayoutAttributes()->TextMetricsValues();
    uint32 textMetricsOffset = fragment.MetricsListOffset;
    uint32 fragmentOffset = 0;
    while (fragmentOffset < fragment.Length)
    {
      CalculateGlyphBoundaries(queryData, fragment, fragmentOffset, extent);
      if (extent.Contains(data->Position))
      {
        // Compute the character offset of the glyph within the text box
        // and add to ProcessedCharacters.
        uint32 characterOffset = fragment.CharacterOffset + fragmentOffset;
        data->ProcessedCharacters += characterOffset - data->TextBox->Start();
        return true;
      }
      fragmentOffset += textMetrics[textMetricsOffset].Length();
      ++textMetricsOffset;
    }
    return false;
  }

  int32 SVGTextQuery::CharacterNumberAtPosition(const FloatPoint &position) const noexcept
  {
    CharacterNumberAtPositionData data(position);
    if (!ExecuteQuery(&data, &SVGTextQuery::CharacterNumberAtPositionCallback))
    {
      return -1;
    }

    return static_cast<int32>(data.ProcessedCharacters);
  }
}
