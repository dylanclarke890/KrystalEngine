#pragma once

#include "Krystal.Booey/Graphics/GlyphBufferMembers.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <limits>
#include <utility>

namespace krys::boo
{
  constexpr GlyphBufferGlyph DeletedGlyph = 0xFFFF;

  class Font;

  /// @brief The glyphs of a run of text, with the font, advance, origin and string offset of each.
  class GlyphBuffer
  {
  private:
    SmallList<const Font *, 1'024> _fonts;
    SmallList<GlyphBufferGlyph, 1'024> _glyphs;
    SmallList<GlyphBufferAdvance, 1'024> _advances;
    SmallList<GlyphBufferOrigin, 1'024> _origins;
    SmallList<GlyphBufferStringOffset, 1'024> _offsetsInString;
    GlyphBufferAdvance _initialAdvance {MakeGlyphBufferAdvance()};

  public:
    static constexpr GlyphBufferStringOffset NoOffset = std::numeric_limits<GlyphBufferStringOffset>::max();

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _fonts.empty();
    }

    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return static_cast<uint32>(_fonts.size());
    }

    void Clear() noexcept
    {
      _fonts.clear();
      _glyphs.clear();
      _advances.clear();
      _origins.clear();
      _offsetsInString.clear();
    }

    KRYS_NODISCARD Span<const Font *> Fonts(size_t from = 0, size_t count = std::dynamic_extent) noexcept
    {
      return Span<const Font *> {_fonts}.subspan(from, count);
    }
    KRYS_NODISCARD Span<GlyphBufferGlyph> Glyphs(size_t from = 0, size_t count = std::dynamic_extent) noexcept
    {
      return Span<GlyphBufferGlyph> {_glyphs}.subspan(from, count);
    }
    KRYS_NODISCARD Span<GlyphBufferAdvance> Advances(size_t from = 0,
                                                     size_t count = std::dynamic_extent) noexcept
    {
      return Span<GlyphBufferAdvance> {_advances}.subspan(from, count);
    }
    KRYS_NODISCARD Span<GlyphBufferOrigin> Origins(size_t from = 0,
                                                   size_t count = std::dynamic_extent) noexcept
    {
      return Span<GlyphBufferOrigin> {_origins}.subspan(from, count);
    }
    KRYS_NODISCARD Span<GlyphBufferStringOffset> OffsetsInString(size_t from = 0,
                                                                 size_t count = std::dynamic_extent) noexcept
    {
      return Span<GlyphBufferStringOffset> {_offsetsInString}.subspan(from, count);
    }
    KRYS_NODISCARD Span<const Font *const> Fonts(size_t from = 0,
                                                 size_t count = std::dynamic_extent) const noexcept
    {
      return Span<const Font *const> {_fonts}.subspan(from, count);
    }
    KRYS_NODISCARD Span<const GlyphBufferGlyph> Glyphs(size_t from = 0,
                                                       size_t count = std::dynamic_extent) const noexcept
    {
      return Span<const GlyphBufferGlyph> {_glyphs}.subspan(from, count);
    }
    KRYS_NODISCARD Span<const GlyphBufferAdvance> Advances(size_t from = 0,
                                                           size_t count = std::dynamic_extent) const noexcept
    {
      return Span<const GlyphBufferAdvance> {_advances}.subspan(from, count);
    }
    KRYS_NODISCARD Span<const GlyphBufferOrigin> Origins(size_t from = 0,
                                                         size_t count = std::dynamic_extent) const noexcept
    {
      return Span<const GlyphBufferOrigin> {_origins}.subspan(from, count);
    }
    KRYS_NODISCARD Span<const GlyphBufferStringOffset>
      OffsetsInString(size_t from = 0, size_t count = std::dynamic_extent) const noexcept
    {
      return Span<const GlyphBufferStringOffset> {_offsetsInString}.subspan(from, count);
    }

    KRYS_NODISCARD const Font &FontAt(size_t index) const noexcept
    {
      krys_debug_assert(_fonts[index]);
      return *_fonts[index];
    }
    KRYS_NODISCARD GlyphBufferGlyph GlyphAt(size_t index) const noexcept
    {
      return _glyphs[index];
    }
    KRYS_NODISCARD GlyphBufferAdvance &AdvanceAt(size_t index) noexcept
    {
      return _advances[index];
    }
    KRYS_NODISCARD GlyphBufferAdvance AdvanceAt(size_t index) const noexcept
    {
      return _advances[index];
    }
    KRYS_NODISCARD GlyphBufferOrigin OriginAt(size_t index) const noexcept
    {
      return _origins[index];
    }
    KRYS_NODISCARD GlyphBufferStringOffset UncheckedStringOffsetAt(size_t index) const noexcept
    {
      return _offsetsInString[index];
    }
    KRYS_NODISCARD Maybe<GlyphBufferStringOffset> CheckedStringOffsetAt(size_t index,
                                                                        uint32 stringLength) const noexcept
    {
      auto result = UncheckedStringOffsetAt(index);
      if (result >= stringLength)
      {
        return null;
      }
      return result;
    }

    void SetInitialAdvance(GlyphBufferAdvance initialAdvance) noexcept
    {
      _initialAdvance = initialAdvance;
    }
    KRYS_NODISCARD const GlyphBufferAdvance &InitialAdvance() const noexcept
    {
      return _initialAdvance;
    }
    void ExpandInitialAdvance(float width) noexcept
    {
      SetWidth(_initialAdvance, Width(_initialAdvance) + width);
    }
    void ExpandInitialAdvance(GlyphBufferAdvance additionalAdvance) noexcept
    {
      SetWidth(_initialAdvance, Width(_initialAdvance) + Width(additionalAdvance));
      SetHeight(_initialAdvance, Height(_initialAdvance) + Height(additionalAdvance));
    }

    void Add(Glyph glyph, const Font &font, float width,
             GlyphBufferStringOffset offsetInString = NoOffset) noexcept
    {
      GlyphBufferAdvance advance = MakeGlyphBufferAdvance(width, 0);
      Add(glyph, font, advance, offsetInString);
    }

    void Add(Glyph glyph, const Font &font, GlyphBufferAdvance advance,
             GlyphBufferStringOffset offsetInString, FloatPoint origin = {}) noexcept
    {
      _fonts.push_back(&font);
      _glyphs.push_back(glyph);
      _advances.push_back(advance);
      _origins.push_back(MakeGlyphBufferOrigin(origin));
      _offsetsInString.push_back(offsetInString);
    }

    void Remove(uint32 location, uint32 length) noexcept
    {
      _fonts.erase(_fonts.begin() + location, _fonts.begin() + location + length);
      _glyphs.erase(_glyphs.begin() + location, _glyphs.begin() + location + length);
      _advances.erase(_advances.begin() + location, _advances.begin() + location + length);
      _origins.erase(_origins.begin() + location, _origins.begin() + location + length);
      _offsetsInString.erase(_offsetsInString.begin() + location,
                             _offsetsInString.begin() + location + length);
    }

    void DeleteGlyphWithoutAffectingSize(uint32 index) noexcept
    {
      MakeGlyphInvisible(index);
      _advances[index] = MakeGlyphBufferAdvance();
    }

    void MakeGlyphInvisible(uint32 index) noexcept
    {
      // GlyphID 0xFFFF is the "deleted glyph" and is supposed to be invisible when rendered.
      _glyphs[index] = DeletedGlyph;
    }

    void MakeHole(uint32 location, uint32 length, const Font *font) noexcept
    {
      krys_debug_assert(location <= Size());
      _fonts.insert(_fonts.begin() + location, length, font);
      _glyphs.insert(_glyphs.begin() + location, length, std::numeric_limits<GlyphBufferGlyph>::max());
      _advances.insert(_advances.begin() + location, length, MakeGlyphBufferAdvance());
      _origins.insert(_origins.begin() + location, length, MakeGlyphBufferOrigin());
      _offsetsInString.insert(_offsetsInString.begin() + location, length, 0);
    }

    void Reverse(uint32 from, uint32 length) noexcept
    {
      if (!length)
      {
        return;
      }
      for (uint32 i = from, end = from + length - 1; i < end; ++i, --end)
      {
        Swap(i, end);
      }
    }

    void ExpandLastAdvance(float width) noexcept
    {
      krys_debug_assert(!IsEmpty());
      GlyphBufferAdvance &lastAdvance = _advances.back();
      SetWidth(lastAdvance, Width(lastAdvance) + width);
    }

    void ExpandAdvance(uint32 index, float width) noexcept
    {
      krys_debug_assert(index < Size());
      auto &lastAdvance = _advances[index];
      SetWidth(lastAdvance, Width(lastAdvance) + width);
    }

    void ExpandAdvanceToLogicalRight(uint32 index, float width) noexcept
    {
      if (index >= Size())
      {
        krys_debug_assert(false);
        return;
      }
      SetWidth(_advances[index], Width(_advances[index]) + width);
      SetX(_origins[index], X(_origins[index]) + width);
    }

    void ExpandLastAdvance(GlyphBufferAdvance expansion) noexcept
    {
      krys_debug_assert(!IsEmpty());
      GlyphBufferAdvance &lastAdvance = _advances.back();
      SetWidth(lastAdvance, Width(lastAdvance) + Width(expansion));
      SetHeight(lastAdvance, Height(lastAdvance) + Height(expansion));
    }

    void Shrink(uint32 truncationPoint) noexcept
    {
      _fonts.truncate(truncationPoint);
      _glyphs.truncate(truncationPoint);
      _advances.truncate(truncationPoint);
      _origins.truncate(truncationPoint);
      _offsetsInString.truncate(truncationPoint);
    }

    /*
     * This is the unflattened format:
     *
     *                                              X (Paint glyph position)   X (Paint glyph position)   X
     * (Paint glyph position) 7                          7                          7 / / / / (Origin) /
     * (Origin)                 / (Origin) /                          /                          / / / /
     *                X---------------------->X------------------------->X------------------------->X------------------------->X
     * (text position ^)  (Initial advance)            (Advance)                  (Advance) (Advance)
     *
     *
     *
     *
     *
     * And this is what we transform it into:
     *
     *                                        ----->X------------------------->X------------------------->X
     *                                       /               (Advance)                   (Advance)         \
     *                                      /                                                               \
     *                   (Initial advance) / \   (Advance)
     *                  ------------------- ----------------
     *                 / \ X X (text position ^)
     *
     * This is an operation that discards all layout information, and preserves only paint information.
     */
    void Flatten() noexcept
    {
      krys_debug_assert(Size() || (!Width(_initialAdvance) && !Height(_initialAdvance)));
      if (Size())
      {
        _initialAdvance = MakeGlyphBufferAdvance(Width(_initialAdvance) + X(_origins[0]),
                                                 Height(_initialAdvance) + Y(_origins[0]));
      }
      for (uint32 i = 0; i < Size(); ++i)
      {
        _advances[i] = MakeGlyphBufferAdvance(
          -X(_origins[i]) + Width(_advances[i]) + (i + 1 < Size() ? X(_origins[i + 1]) : 0),
          -Y(_origins[i]) + Height(_advances[i]) + (i + 1 < Size() ? Y(_origins[i + 1]) : 0));
        _origins[i] = MakeGlyphBufferOrigin();
      }
    }

    KRYS_NODISCARD bool IsFlattened() const noexcept
    {
      for (uint32 i = 0; i < Size(); ++i)
      {
        if (X(_origins[i]) || Y(_origins[i]))
        {
          return false;
        }
      }
      return true;
    }

  private:
    void Swap(uint32 index1, uint32 index2) noexcept
    {
      std::swap(_fonts[index1], _fonts[index2]);
      std::swap(_glyphs[index1], _glyphs[index2]);
      std::swap(_advances[index1], _advances[index2]);
      std::swap(_origins[index1], _origins[index2]);
      std::swap(_offsetsInString[index1], _offsetsInString[index2]);
    }
  };
}
