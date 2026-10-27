#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorSVGTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextFragment.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class LegacyInlineFlowBox;
  class RenderObject;
  class SVGInlineTextBox;

  /// @brief Answers SVGTextContentElement's queries (lengths, character positions and extents) from the laid
  /// out text fragments.
  /// @note Character positions are UTF-8 code units, as the DOM's offsets are.
  class SVGTextQuery
  {
  public:
    // Public helper struct. Private classes in SVGTextQuery inherit from it.
    struct Data;

  private:
    using ProcessTextFragmentCallback = bool (SVGTextQuery::*)(Data *, const SVGTextFragment &) const;

    List<inline_iterator::SVGTextBoxIterator> _textBoxes;

  public:
    explicit SVGTextQuery(RenderObject *) noexcept;

    KRYS_NODISCARD uint32 NumberOfCharacters() const noexcept;
    KRYS_NODISCARD float TextLength() const noexcept;
    KRYS_NODISCARD float SubStringLength(uint32 startPosition, uint32 length) const noexcept;
    KRYS_NODISCARD FloatPoint StartPositionOfCharacter(uint32 position) const noexcept;
    KRYS_NODISCARD FloatPoint EndPositionOfCharacter(uint32 position) const noexcept;
    KRYS_NODISCARD float RotationOfCharacter(uint32 position) const noexcept;
    KRYS_NODISCARD FloatRect ExtentOfCharacter(uint32 position) const noexcept;
    KRYS_NODISCARD int32 CharacterNumberAtPosition(const FloatPoint &) const noexcept;

  private:
    bool ExecuteQuery(Data *, ProcessTextFragmentCallback) const noexcept;

    void CollectTextBoxesInInlineBox(inline_iterator::InlineBoxIterator) noexcept;
    bool MapStartEndPositionsIntoFragmentCoordinates(Data *, const SVGTextFragment &, uint32 &startPosition,
                                                     uint32 &endPosition) const noexcept;
    void ModifyStartEndPositionsRespectingLigatures(Data *, const SVGTextFragment &, uint32 &startPosition,
                                                    uint32 &endPosition) const noexcept;

    bool NumberOfCharactersCallback(Data *, const SVGTextFragment &) const noexcept;
    bool TextLengthCallback(Data *, const SVGTextFragment &) const noexcept;
    bool SubStringLengthCallback(Data *, const SVGTextFragment &) const noexcept;
    bool StartPositionOfCharacterCallback(Data *, const SVGTextFragment &) const noexcept;
    bool EndPositionOfCharacterCallback(Data *, const SVGTextFragment &) const noexcept;
    bool RotationOfCharacterCallback(Data *, const SVGTextFragment &) const noexcept;
    bool ExtentOfCharacterCallback(Data *, const SVGTextFragment &) const noexcept;
    bool CharacterNumberAtPositionCallback(Data *, const SVGTextFragment &) const noexcept;
  };
}
