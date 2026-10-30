#pragma once

#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/GlyphBuffer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <limits>
#include <utility>

struct hb_buffer_t;

namespace krys::boo
{
  class CachedTextBreakIterator;
  class FontCascade;
  class TextRun;

  enum class GlyphIterationStyle : bool
  {
    IncludePartialGlyphs,
    ByWholeGlyphs
  };

  /// @brief Lays out a run of text by shaping it with HarfBuzz, one font and script at a time, and walks the
  /// shaped glyphs in string order.
  class ComplexTextController
  {
  public:
    class ComplexTextRun;

  private:
    SmallList<FloatSize, 256> _adjustedBaseAdvances;
    SmallList<FloatPoint, 256> _glyphOrigins;
    SmallList<Glyph, 256> _adjustedGlyphs;
    SmallList<float, 256> _textAutoSpaceSpacings;
    utf8_string _smallCapsBuffer;

    // There is a 3-level hierarchy here. At the top, we are interested in the run's text. We partition that
    // text into items, each of which is a sequence of characters which should use the same Font. Shaping then
    // partitions the item into ComplexTextRuns, one per script.
    // ComplexTextRun::StringLocation() and ComplexTextRun::StringLength() refer to the offset and length of
    // the item relative to the run's text. ComplexTextRun::IndexAt() returns the offset of a codepoint
    // relative to its item. ComplexTextRun::Glyphs() and ComplexTextRun::BaseAdvances() refer to glyphs
    // relative to the ComplexTextRun. The length of the entire TextRun is _run.Length().
    SmallList<RefPtr<ComplexTextRun>, 16> _complexTextRuns;

    // The initial capacity of these vectors was selected as being the smallest power of two greater than
    // the average (3.5) plus one standard deviation (7.5) of nonzero sizes used on Arabic Wikipedia.
    SmallList<uint32, 16> _runIndices;
    SmallList<uint32, 16> _glyphCountFromStartToIndex;

    HashSet<RefPtr<const Font>> *_fallbackFonts {nullptr};

    const FontCascade &_fontCascade;
    const TextRun &_run;

    uint32 _currentCharacter {0};
    uint32 _end {0};

    FloatSize _totalAdvance;
    float _runWidthSoFar {0};
    uint32 _numGlyphsSoFar {0};
    uint32 _currentRun {0};
    uint32 _glyphInCurrentRun {0};
    uint32 _characterInCurrentGlyph {0};
    float _expansion {0};
    float _expansionPerOpportunity {0};

    float _minGlyphBoundingBoxX {std::numeric_limits<float>::max()};
    float _maxGlyphBoundingBoxX {std::numeric_limits<float>::lowest()};
    float _minGlyphBoundingBoxY {std::numeric_limits<float>::max()};
    float _maxGlyphBoundingBoxY {std::numeric_limits<float>::lowest()};

    bool _isLTROnly {true};
    bool _mayUseNaturalWritingDirection {false};
    bool _forTextEmphasis {false};

    text_spacing::SpacingState _textSpacingState;

  public:
    ComplexTextController(const FontCascade &fontCascade, const TextRun &run,
                          bool mayUseNaturalWritingDirection = false,
                          HashSet<RefPtr<const Font>> *fallbackFonts = nullptr,
                          bool forTextEmphasis = false) noexcept;

    /// @brief A controller over already shaped runs.
    ComplexTextController(const FontCascade &fontCascade, const TextRun &run,
                          List<Ref<ComplexTextRun>> &runs) noexcept;

    ~ComplexTextController() noexcept;

    /// @brief The top (negative, from the baseline) and bottom of the union of @p textRun's glyph bounds.
    KRYS_NODISCARD static std::pair<float, float>
      EnclosingGlyphBoundsForTextRun(const FontCascade &fontCascade, const TextRun &textRun) noexcept;
    /// @brief The advance of each byte of the RTL run @p textRun (a glyph's advance goes to the first byte
    /// of its character).
    KRYS_NODISCARD static List<float> GlyphAdvancesForTextRun(const FontCascade &fontCascade,
                                                              const TextRun &textRun) noexcept;

    /// @brief Advance and emit glyphs up to the specified byte offset.
    void Advance(uint32 to, GlyphBuffer *glyphBuffer = nullptr,
                 GlyphIterationStyle iterationStyle = GlyphIterationStyle::IncludePartialGlyphs,
                 HashSet<RefPtr<const Font>> *fallbackFonts = nullptr) noexcept;

    /// @brief Compute the byte offset for a given x coordinate.
    KRYS_NODISCARD uint32 OffsetForPosition(float x, bool includePartialGlyphs) noexcept;

    /// @brief Returns the width of everything we've consumed so far.
    KRYS_NODISCARD float RunWidthSoFar() const noexcept
    {
      return _runWidthSoFar;
    }

    KRYS_NODISCARD FloatSize TotalAdvance() const noexcept
    {
      return _totalAdvance;
    }

    KRYS_NODISCARD float MinGlyphBoundingBoxX() const noexcept
    {
      return _minGlyphBoundingBoxX;
    }
    KRYS_NODISCARD float MaxGlyphBoundingBoxX() const noexcept
    {
      return _maxGlyphBoundingBoxX;
    }
    KRYS_NODISCARD float MinGlyphBoundingBoxY() const noexcept
    {
      return _minGlyphBoundingBoxY;
    }
    KRYS_NODISCARD float MaxGlyphBoundingBoxY() const noexcept
    {
      return _maxGlyphBoundingBoxY;
    }

    /// @brief The glyphs one font (and script) shaped for part of a run.
    class ComplexTextRun : public RefCounted<ComplexTextRun>
    {
    private:
      SmallList<FloatSize, 64> _baseAdvances;
      SmallList<FloatPoint, 64> _glyphOrigins;
      SmallList<Glyph, 64> _glyphs;
      SmallList<uint32, 64> _glyphEndOffsets;
      SmallList<uint32, 64> _coreTextIndices;
      FloatSize _initialAdvance;
      Ref<const Font> _font;
      utf8_stringview _characters;
      uint32 _indexBegin;
      uint32 _indexEnd;
      uint32 _glyphCount {0};
      uint32 _stringLocation;
      bool _isLTR;
      bool _isMonotonic {true};
      float _textAutospaceSize {0};

      ComplexTextRun(hb_buffer_t *buffer, const Font &font, utf8_stringview characters, uint32 stringLocation,
                     uint32 indexBegin, uint32 indexEnd) noexcept;
      ComplexTextRun(const Font &font, utf8_stringview characters, uint32 stringLocation, uint32 indexBegin,
                     uint32 indexEnd, bool ltr) noexcept;
      ComplexTextRun(Span<const FloatSize> advances, Span<const FloatPoint> origins, Span<const Glyph> glyphs,
                     Span<const uint32> stringIndices, FloatSize initialAdvance, const Font &font,
                     utf8_stringview characters, uint32 stringLocation, uint32 indexBegin, uint32 indexEnd,
                     bool ltr) noexcept;

    public:
      /// @brief The glyphs HarfBuzz shaped into @p buffer.
      KRYS_NODISCARD static Ref<ComplexTextRun> Create(hb_buffer_t *buffer, const Font &font,
                                                       utf8_stringview characters, uint32 stringLocation,
                                                       uint32 indexBegin, uint32 indexEnd) noexcept
      {
        return AdoptRef(*new ComplexTextRun(buffer, font, characters, stringLocation, indexBegin, indexEnd));
      }

      /// @brief A run of @p font's missing glyph, one per character.
      KRYS_NODISCARD static Ref<ComplexTextRun> Create(const Font &font, utf8_stringview characters,
                                                       uint32 stringLocation, uint32 indexBegin,
                                                       uint32 indexEnd, bool ltr) noexcept
      {
        return AdoptRef(*new ComplexTextRun(font, characters, stringLocation, indexBegin, indexEnd, ltr));
      }

      /// @brief A run of the given glyphs.
      KRYS_NODISCARD static Ref<ComplexTextRun>
        Create(Span<const FloatSize> advances, Span<const FloatPoint> origins, Span<const Glyph> glyphs,
               Span<const uint32> stringIndices, FloatSize initialAdvance, const Font &font,
               utf8_stringview characters, uint32 stringLocation, uint32 indexBegin, uint32 indexEnd,
               bool ltr) noexcept
      {
        return AdoptRef(*new ComplexTextRun(advances, origins, glyphs, stringIndices, initialAdvance, font,
                                            characters, stringLocation, indexBegin, indexEnd, ltr));
      }

      KRYS_NODISCARD uint32 GlyphCount() const noexcept
      {
        return _glyphCount;
      }
      KRYS_NODISCARD const Font &GetFont() const noexcept
      {
        return *_font;
      }
      KRYS_NODISCARD utf8_stringview Characters() const noexcept
      {
        return _characters;
      }
      KRYS_NODISCARD uint32 StringLocation() const noexcept
      {
        return _stringLocation;
      }
      KRYS_NODISCARD uint32 StringLength() const noexcept
      {
        return static_cast<uint32>(_characters.length());
      }
      KRYS_NODISCARD uint32 IndexAt(uint32 i) const noexcept
      {
        krys_debug_assert(i < _glyphCount);
        return _coreTextIndices[i];
      }
      KRYS_NODISCARD uint32 IndexBegin() const noexcept
      {
        return _indexBegin;
      }
      KRYS_NODISCARD uint32 IndexEnd() const noexcept
      {
        return _indexEnd;
      }
      KRYS_NODISCARD uint32 EndOffsetAt(uint32 i) const noexcept
      {
        krys_debug_assert(!_isMonotonic);
        return _glyphEndOffsets[i];
      }
      KRYS_NODISCARD Span<const Glyph> Glyphs() const noexcept
      {
        return _glyphs;
      }

      void GrowInitialAdvanceHorizontally(float delta) noexcept
      {
        _initialAdvance.Expand(delta, 0);
      }
      KRYS_NODISCARD FloatSize InitialAdvance() const noexcept
      {
        return _initialAdvance;
      }
      KRYS_NODISCARD Span<const FloatSize> BaseAdvances() const noexcept
      {
        return _baseAdvances;
      }
      KRYS_NODISCARD Span<const FloatPoint> GlyphOrigins() const noexcept
      {
        return _glyphOrigins.size() == GlyphCount() ? Span<const FloatPoint> {_glyphOrigins}
                                                    : Span<const FloatPoint> {};
      }
      KRYS_NODISCARD bool IsLTR() const noexcept
      {
        return _isLTR;
      }
      KRYS_NODISCARD bool IsMonotonic() const noexcept
      {
        return _isMonotonic;
      }
      void SetIsNonMonotonic() noexcept;

      KRYS_NODISCARD float TextAutospaceSize() const noexcept
      {
        return _textAutospaceSize;
      }
    };

  private:
    ComplexTextController(const TextRun &run, const FontCascade &fontCascade) noexcept;

    void ComputeExpansionOpportunity() noexcept;
    void FinishConstruction() noexcept;

    KRYS_NODISCARD static uint32 StringBegin(const ComplexTextRun &run) noexcept
    {
      return run.StringLocation() + run.IndexBegin();
    }
    KRYS_NODISCARD static uint32 StringEnd(const ComplexTextRun &run) noexcept
    {
      return run.StringLocation() + run.IndexEnd();
    }

    void CollectComplexTextRuns() noexcept;
    void CollectComplexTextRunsForCharacters(utf8_stringview characters, uint32 stringLocation,
                                             const Font *font) noexcept;
    void AdjustGlyphsAndAdvances() noexcept;

    KRYS_NODISCARD uint32 IndexOfCurrentRun(uint32 &leftmostGlyph) noexcept;
    KRYS_NODISCARD uint32 IncrementCurrentRun(uint32 &leftmostGlyph) noexcept;

    KRYS_NODISCARD float RunWidthSoFarFraction(const ComplexTextRun &complexTextRun, uint32 glyphStartOffset,
                                               uint32 glyphEndOffset, uint32 oldCharacterInCurrentGlyph,
                                               GlyphIterationStyle iterationStyle) const noexcept;

    KRYS_NODISCARD FloatPoint GlyphOrigin(uint32 index) const noexcept
    {
      return index < _glyphOrigins.size() ? _glyphOrigins[index] : FloatPoint();
    }

    void AdvanceByCombiningCharacterSequence(const CachedTextBreakIterator &graphemeClusterIterator,
                                             uint32 &location, char32 &baseCharacter) noexcept;
  };
}
