#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  /// @brief A range of a text box's text painted in its own way (unmarked text, selection, document markers,
  /// highlights), by byte offsets into the box's text.
  /// FIXME: Collecting document markers, highlights and dragged or transparent content
  /// (`collectForDocumentMarkers`, `collectForHighlights`, `collectForDraggedAndTransparentContent`) comes
  /// with editing, the highlight registry and selection; until then text is only ever unmarked, and
  /// `RenderedDocumentMarker`s aren't kept.
  struct MarkedText
  {
    // Sorted by paint order
    enum class Type : uint8
    {
      Unmarked,
      GrammarError,
      Correction,
      SpellingError,
      TextMatch,
      DictationAlternatives,
      Highlight,
      FragmentHighlight,
      TextExtractionHighlight,
      Selection,
      DraggedContent,
      TransparentContent,
    };

    enum class PaintPhase
    {
      Background,
      Foreground,
      Decoration
    };

    enum class OverlapStrategy
    {
      None,
      Frontmost
    };

    uint32 StartOffset {0};
    uint32 EndOffset {0};
    Type MarkedType {Type::Unmarked};
    StringAtom HighlightName {StringAtom::Null()};
    int32 Priority {0};

    MarkedText() noexcept = default;
    MarkedText(uint32 startOffset, uint32 endOffset, Type type,
               const StringAtom &highlightName = StringAtom::Null(), int32 priority = 0) noexcept
        : StartOffset(startOffset), EndOffset(endOffset), MarkedType(type), HighlightName(highlightName),
          Priority(priority)
    {
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return EndOffset <= StartOffset;
    }

    KRYS_NODISCARD bool operator==(const MarkedText &) const noexcept = default;

    /// @brief Splits @p markedTexts into ranges that don't partly overlap, in paint order: with
    /// `OverlapStrategy::None` every marked text covering a range gets a copy of it, with
    /// `OverlapStrategy::Frontmost` only the one painted last.
    KRYS_NODISCARD static List<MarkedText> Subdivide(const List<MarkedText> &markedTexts,
                                                     OverlapStrategy overlapStrategy = OverlapStrategy::None);
  };
}
