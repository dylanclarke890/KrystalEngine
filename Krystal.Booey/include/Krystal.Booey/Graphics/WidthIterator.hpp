#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/GlyphBuffer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <limits>

namespace krys::boo
{
  class FontCascade;
  class FontCascadeDescription;
  class Font;
  class TextRun;
  struct GlyphData;
  struct GlyphIndexRange;
  struct OriginalAdvancesForCharacterTreatedAsSpace;
  struct AdvanceInternalState;
  struct SmallCapsState;
  class Utf8TextIterator;

  using CharactersTreatedAsSpace = SmallList<OriginalAdvancesForCharacterTreatedAsSpace, 64>;

  /// @brief Lays out a run of text one font range at a time without shaping, adding each character's glyph
  /// and advance to a glyph buffer.
  struct WidthIterator
  {
  public:
    WidthIterator(const FontCascade &fontCascade, const TextRun &run,
                  HashSet<RefPtr<const Font>> *fallbackFonts = nullptr, bool accountForGlyphBounds = false,
                  bool forTextEmphasis = false) noexcept;

    /// @brief Lays out the run up to byte offset @p to.
    void Advance(uint32 to, GlyphBuffer &glyphBuffer) noexcept;
    KRYS_NODISCARD bool AdvanceOneCharacter(float &width, GlyphBuffer &glyphBuffer) noexcept;
    void Finalize(GlyphBuffer &buffer) noexcept;

    KRYS_NODISCARD float MaxGlyphBoundingBoxY() const noexcept
    {
      krys_debug_assert(_accountForGlyphBounds);
      return _maxGlyphBoundingBoxY;
    }
    KRYS_NODISCARD float MinGlyphBoundingBoxY() const noexcept
    {
      krys_debug_assert(_accountForGlyphBounds);
      return _minGlyphBoundingBoxY;
    }
    KRYS_NODISCARD float FirstGlyphOverflow() const noexcept
    {
      krys_debug_assert(_accountForGlyphBounds);
      return _firstGlyphOverflow;
    }
    KRYS_NODISCARD float LastGlyphOverflow() const noexcept
    {
      krys_debug_assert(_accountForGlyphBounds);
      return _lastGlyphOverflow;
    }

    KRYS_NODISCARD const TextRun &Run() const noexcept
    {
      return _run;
    }
    KRYS_NODISCARD float RunWidthSoFar() const noexcept
    {
      return _runWidthSoFar;
    }
    KRYS_NODISCARD uint32 CurrentCharacterIndex() const noexcept
    {
      return _currentCharacterIndex;
    }

    KRYS_NODISCARD static bool CharacterCanUseSimplifiedTextMeasuring(char32 codePoint,
                                                                      bool whitespaceIsCollapsed) noexcept;

  private:
    struct ApplyFontTransformsResult
    {
      float AdditionalAdvance;
      GlyphBufferAdvance InitialAdvance;
    };

    struct AdditionalWidth
    {
      float Left;
      float Right;
      float LeftExpansion;
      float RightExpansion;
    };

    const FontCascade &_fontCascade;
    const TextRun &_run;
    HashSet<RefPtr<const Font>> *_fallbackFonts {nullptr};
    Maybe<uint32> _lastCharacterIndex;
    GlyphBufferAdvance _leftoverInitialAdvance {MakeGlyphBufferAdvance()};
    uint32 _currentCharacterIndex {0};
    float _leftoverJustificationWidth {0};
    float _runWidthSoFar {0};
    float _expansion {0};
    float _expansionPerOpportunity {0};
    float _maxGlyphBoundingBoxY {std::numeric_limits<float>::lowest()};
    float _minGlyphBoundingBoxY {std::numeric_limits<float>::max()};
    float _firstGlyphOverflow {0};
    float _lastGlyphOverflow {0};
    TextDirection _direction {TextDirection::LTR};
    bool _containsTabs {false};
    bool _isAfterExpansion {false};
    bool _accountForGlyphBounds {false};
    bool _enableKerning {false};
    bool _requiresShaping {false};
    bool _forTextEmphasis {false};

    void AdvanceInternal(Utf8TextIterator &textIterator, GlyphBuffer &glyphBuffer) noexcept;

    KRYS_NODISCARD ApplyFontTransformsResult
      ApplyFontTransforms(GlyphBuffer &glyphBuffer, uint32 lastGlyphCount, const Font &font,
                          CharactersTreatedAsSpace &charactersTreatedAsSpace) noexcept;
    void CommitCurrentFontRange(AdvanceInternalState &advanceInternalState) noexcept;
    void StartNewFontRangeIfNeeded(AdvanceInternalState &advanceInternalState, SmallCapsState &smallCapsState,
                                   const FontCascadeDescription &fontDescription) noexcept;
    void ApplyInitialAdvance(GlyphBuffer &glyphBuffer, GlyphBufferAdvance initialAdvance,
                             uint32 lastGlyphCount) noexcept;

    KRYS_NODISCARD bool HasExtraSpacing() const noexcept;
    void ApplyExtraSpacingAfterShaping(GlyphBuffer &glyphBuffer, uint32 characterStartIndex,
                                       uint32 glyphBufferStartIndex, uint32 characterDestinationIndex,
                                       float startingRunWidth) noexcept;
    void ApplyCSSVisibilityRules(GlyphBuffer &glyphBuffer, uint32 glyphBufferStartIndex) noexcept;

    KRYS_NODISCARD AdditionalWidth CalculateAdditionalWidth(GlyphBuffer &glyphBuffer,
                                                            GlyphBufferStringOffset currentCharacterIndex,
                                                            uint32 leadingGlyphIndex,
                                                            uint32 trailingGlyphIndex,
                                                            float position) const noexcept;
    void ApplyAdditionalWidth(GlyphBuffer &glyphBuffer, GlyphIndexRange glyphIndexRange,
                              float leftAdditionalWidth, float rightAdditionalWidth,
                              float leftExpansionAdditionalWidth,
                              float rightExpansionAdditionalWidth) noexcept;

    KRYS_NODISCARD TextDirection Direction() const noexcept
    {
      return _direction;
    }
    KRYS_NODISCARD bool Rtl() const noexcept
    {
      return _direction == TextDirection::RTL;
    }
    KRYS_NODISCARD bool Ltr() const noexcept
    {
      return _direction == TextDirection::LTR;
    }
  };
}
