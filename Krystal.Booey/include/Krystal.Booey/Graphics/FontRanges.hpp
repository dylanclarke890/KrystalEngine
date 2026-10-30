#pragma once

#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo
{
  enum class ExternalResourceDownloadPolicy : bool
  {
    Forbid,
    Allow
  };

  enum class IsGenericFontFamily : bool
  {
    No,
    Yes
  };

  /// @brief Gives the font of a range of a family, which may still be loading.
  class FontAccessor : public RefCounted<FontAccessor>
  {
  public:
    virtual ~FontAccessor() noexcept = default;

    KRYS_NODISCARD virtual const Font *GetFont(ExternalResourceDownloadPolicy policy) const noexcept = 0;
    KRYS_NODISCARD virtual bool IsLoading() const noexcept = 0;
  };

  /// @brief The fonts of a family, each for a range of code points (one range covering everything for a local
  /// font; the `unicode-range`s of a web font family).
  class FontRanges
  {
  public:
    class Range
    {
    private:
      char32 _from;
      char32 _to;
      Ref<FontAccessor> _fontAccessor;

    public:
      Range(char32 from, char32 to, Ref<FontAccessor> &&fontAccessor) noexcept
          : _from(from), _to(to), _fontAccessor(krys::move(fontAccessor))
      {
      }

      KRYS_NODISCARD char32 From() const noexcept
      {
        return _from;
      }

      KRYS_NODISCARD char32 To() const noexcept
      {
        return _to;
      }

      KRYS_NODISCARD const Font *GetFont(ExternalResourceDownloadPolicy policy) const noexcept;

      KRYS_NODISCARD const FontAccessor &GetFontAccessor() const noexcept
      {
        return *_fontAccessor;
      }
    };

  private:
    SmallList<Range, 1> _ranges;
    boo::IsGenericFontFamily _isGenericFontFamily {boo::IsGenericFontFamily::No};

  public:
    FontRanges() noexcept = default;
    explicit FontRanges(RefPtr<Font> &&font) noexcept;
    FontRanges(FontRanges &&other, boo::IsGenericFontFamily isGenericFontFamily) noexcept;
    ~FontRanges() noexcept;

    FontRanges(const FontRanges &) noexcept = default;
    FontRanges(FontRanges &&) noexcept = default;
    FontRanges &operator=(const FontRanges &) noexcept = default;
    FontRanges &operator=(FontRanges &&) noexcept = default;

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return _ranges.empty();
    }

    void AppendRange(Range &&range) noexcept
    {
      _ranges.push_back(krys::move(range));
    }

    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return static_cast<uint32>(_ranges.size());
    }

    KRYS_NODISCARD const Range &RangeAt(uint32 i) const noexcept
    {
      return _ranges[i];
    }

    KRYS_NODISCARD GlyphData GlyphDataForCharacter(char32 character,
                                                   ExternalResourceDownloadPolicy policy) const noexcept;
    KRYS_NODISCARD const Font *FontForCharacter(char32 character) const noexcept;
    KRYS_NODISCARD const Font &FontForFirstRange() const noexcept;
    KRYS_NODISCARD bool IsLoading() const noexcept;

    KRYS_NODISCARD bool IsGenericFontFamily() const noexcept
    {
      return _isGenericFontFamily == boo::IsGenericFontFamily::Yes;
    }
  };
}
