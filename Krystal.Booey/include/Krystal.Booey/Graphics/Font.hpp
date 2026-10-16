#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Graphics/FontPlatformData.hpp"
#include "Krystal.Booey/Graphics/GlyphMetricsMap.hpp"
#include "Krystal.Booey/Graphics/GlyphPage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FontDescription;

  enum FontVariant : uint8
  {
    AutoVariant,
    NormalVariant,
    SmallCapsVariant,
    EmphasisMarkVariant,
    BrokenIdeographVariant
  };

  enum Pitch : uint8
  {
    UnknownPitch,
    FixedPitch,
    VariablePitch
  };

  enum class IsForPlatformFont : bool
  {
    No,
    Yes
  };

  // Used to create platform fonts.
  enum class FontOrigin : bool
  {
    Remote,
    Local
  };

  enum class FontIsInterstitial : bool
  {
    No,
    Yes
  };

  enum class FontVisibility : bool
  {
    Visible,
    Invisible
  };

  enum class FontIsOrientationFallback : bool
  {
    No,
    Yes
  };

  struct FontInternalAttributes
  {
    FontOrigin Origin : 1;
    FontIsInterstitial IsInterstitial : 1;
    FontVisibility Visibility : 1;
    FontIsOrientationFallback IsTextOrientationFallback : 1;
  };

  /// @brief A font: a face at a size, with its metrics, its glyphs and their widths and bounds, and the fonts
  /// derived from it (small caps, emphasis marks, orientations).
  class Font : public RefCounted<Font>, public CanMakeWeakPtr<Font>
  {
    KRYS_NON_COPY_MOVABLE(Font);

  public:
    using Origin = FontOrigin;
    using Visibility = FontVisibility;
    using IsOrientationFallback = FontIsOrientationFallback;
    using Attributes = FontInternalAttributes;

    static constexpr size_t InlineGlyphRunCapacity = 128;

    // Should the result of this function include the results of synthetic bold?
    enum class SyntheticBoldInclusion
    {
      Incorporate,
      Exclude
    };

    enum class IsSystemFallbackFontPlaceholder : bool
    {
      No,
      Yes
    };

  private:
    struct DerivedFonts
    {
      RefPtr<Font> SmallCapsFont;
      RefPtr<Font> NoSynthesizableFeaturesFont;
      RefPtr<Font> EmphasisMarkFont;
      RefPtr<Font> BrokenIdeographFont;
      RefPtr<Font> VerticalRightOrientationFont;
      RefPtr<Font> UprightOrientationFont;
      RefPtr<Font> InvisibleFont;
      RefPtr<Font> HalfWidthFont;
    };

    FontMetrics _fontMetrics;
    float _maxCharWidth {-1};
    float _avgCharWidth {-1};

    const FontPlatformData _platformData;

    mutable HashMap<uint32, RefPtr<GlyphPage>> _glyphPages;
    mutable GlyphMetricsMap<float> _glyphToWidthMap;
    mutable UniquePtr<GlyphMetricsMap<FloatRect>> _glyphToBoundsMap;
    // Two bits per code point of `CodePointSupportIndex`: whether it's been checked, and whether it's
    // supported.
    mutable List<bool> _codePointSupport;

    Attributes _attributes;

    mutable UniquePtr<DerivedFonts> _derivedFontData;

    Glyph _spaceGlyph {0};
    Glyph _zeroWidthSpaceGlyph {0};

    float _spaceWidth {0};
    float _syntheticBoldOffset {0};

    bool _treatAsFixedPitch {false};
    bool _canTakeFixedPitchFastContentMeasuring {false};
    bool _isBrokenIdeographFallback {false};
    bool _hasVerticalGlyphs {false};
    bool _isUsedInSystemFallbackFontCache {false};
    bool _allowsAntialiasing {true};
    bool _isSystemFontFallbackPlaceholder {false};

    Font(const FontPlatformData &platformData, Origin origin, FontIsInterstitial interstitial,
         Visibility visibility, IsOrientationFallback orientationFallback) noexcept;
    explicit Font(IsSystemFallbackFontPlaceholder) noexcept;

  public:
    ~Font() noexcept;

    KRYS_NODISCARD static Ref<Font>
      Create(const FontPlatformData &platformData, Origin origin = Origin::Local,
             FontIsInterstitial interstitial = FontIsInterstitial::No,
             Visibility visibility = Visibility::Visible,
             IsOrientationFallback orientationFallback = IsOrientationFallback::No) noexcept;

    KRYS_NODISCARD static Ref<Font> CreateSystemFallbackFontPlaceholder() noexcept
    {
      return AdoptRef(*new Font(IsSystemFallbackFontPlaceholder::Yes));
    }

    KRYS_NODISCARD bool IsSystemFontFallbackPlaceholder() const noexcept
    {
      return _isSystemFontFallbackPlaceholder;
    }

    KRYS_NODISCARD const FontPlatformData &PlatformData() const noexcept
    {
      return _platformData;
    }

    KRYS_NODISCARD const Font *SmallCapsFont(const FontDescription &fontDescription) const noexcept;
    KRYS_NODISCARD const Font &NoSynthesizableFeaturesFont() const noexcept;
    KRYS_NODISCARD const Font *EmphasisMarkFont(const FontDescription &fontDescription) const noexcept;
    KRYS_NODISCARD const Font &BrokenIdeographFont() const noexcept;
    KRYS_NODISCARD RefPtr<Font> HalfWidthFont() const noexcept;

    KRYS_NODISCARD bool IsProbablyOnlyUsedToRenderIcons() const noexcept;

    KRYS_NODISCARD const Font *VariantFont(const FontDescription &description,
                                           FontVariant variant) const noexcept
    {
      switch (variant)
      {
        case SmallCapsVariant:       return SmallCapsFont(description);
        case EmphasisMarkVariant:    return EmphasisMarkFont(description);
        case BrokenIdeographVariant: return &BrokenIdeographFont();
        case AutoVariant:
        case NormalVariant:          break;
      }
      krys_debug_assert_msg(false, "a variant font is only asked for a variant");
      return this;
    }

    KRYS_NODISCARD bool VariantCapsSupportedForSynthesis(FontVariantCaps fontVariantCaps) const noexcept;

    KRYS_NODISCARD const Font &VerticalRightOrientationFont() const noexcept;
    KRYS_NODISCARD const Font &UprightOrientationFont() const noexcept;
    KRYS_NODISCARD const Font &InvisibleFont() const noexcept;

    KRYS_NODISCARD bool HasVerticalGlyphs() const noexcept
    {
      return _hasVerticalGlyphs;
    }

    KRYS_NODISCARD bool IsTextOrientationFallback() const noexcept
    {
      return _attributes.IsTextOrientationFallback == IsOrientationFallback::Yes;
    }

    KRYS_NODISCARD const FontMetrics &GetFontMetrics() const noexcept
    {
      return _fontMetrics;
    }

    KRYS_NODISCARD float SizePerUnit() const noexcept
    {
      return PlatformData().Size() / (GetFontMetrics().UnitsPerEm() ? GetFontMetrics().UnitsPerEm() : 1);
    }

    KRYS_NODISCARD float MaxCharWidth() const noexcept
    {
      return _maxCharWidth;
    }

    void SetMaxCharWidth(float maxCharWidth) noexcept
    {
      _maxCharWidth = maxCharWidth;
    }

    KRYS_NODISCARD float AvgCharWidth() const noexcept
    {
      return _avgCharWidth;
    }

    void SetAvgCharWidth(float avgCharWidth) noexcept
    {
      _avgCharWidth = avgCharWidth;
    }

    KRYS_NODISCARD FloatRect BoundsForGlyph(Glyph glyph) const noexcept;
    KRYS_NODISCARD SmallList<FloatRect, InlineGlyphRunCapacity>
      BoundsForGlyphs(Span<const Glyph> glyphs) const noexcept;

    KRYS_NODISCARD float WidthForGlyph(Glyph glyph, SyntheticBoldInclusion syntheticBoldInclusion =
                                                      SyntheticBoldInclusion::Incorporate) const noexcept;

    KRYS_NODISCARD float SpaceWidth(
      SyntheticBoldInclusion syntheticBoldInclusion = SyntheticBoldInclusion::Incorporate) const noexcept
    {
      return _spaceWidth
             + (syntheticBoldInclusion == SyntheticBoldInclusion::Incorporate ? SyntheticBoldOffset() : 0);
    }

    KRYS_NODISCARD float SyntheticBoldOffset() const noexcept
    {
      return _syntheticBoldOffset;
    }

    KRYS_NODISCARD Glyph SpaceGlyph() const noexcept
    {
      return _spaceGlyph;
    }

    KRYS_NODISCARD Glyph ZeroWidthSpaceGlyph() const noexcept
    {
      return _zeroWidthSpaceGlyph;
    }

    KRYS_NODISCARD bool IsZeroWidthSpaceGlyph(Glyph glyph) const noexcept
    {
      return glyph == _zeroWidthSpaceGlyph && glyph;
    }

    KRYS_NODISCARD GlyphData GlyphDataForCharacter(char32 character) const noexcept;
    KRYS_NODISCARD Glyph GlyphForCharacter(char32 character) const noexcept;
    KRYS_NODISCARD bool SupportsCodePoint(char32 character) const noexcept;
    KRYS_NODISCARD bool PlatformSupportsCodePoint(char32 character,
                                                  Maybe<char32> variation = null) const noexcept;

    KRYS_NODISCARD const GlyphPage *GetGlyphPage(uint32 pageNumber) const noexcept;

    void DeterminePitch() noexcept;

    KRYS_NODISCARD Pitch GetPitch() const noexcept
    {
      return _treatAsFixedPitch ? FixedPitch : VariablePitch;
    }

    KRYS_NODISCARD bool CanTakeFixedPitchFastContentMeasuring() const noexcept
    {
      return _canTakeFixedPitchFastContentMeasuring;
    }

    KRYS_NODISCARD Origin GetOrigin() const noexcept
    {
      return _attributes.Origin;
    }

    KRYS_NODISCARD bool IsInterstitial() const noexcept
    {
      return _attributes.IsInterstitial == FontIsInterstitial::Yes;
    }

    KRYS_NODISCARD Visibility GetVisibility() const noexcept
    {
      return _attributes.Visibility;
    }

    KRYS_NODISCARD bool AllowsAntialiasing() const noexcept
    {
      return _allowsAntialiasing;
    }

    KRYS_NODISCARD utf8_string Description() const noexcept;

    /// @brief Whether every code point of @p text (UTF-8) can be rendered with this font, variation sequences
    /// included.
    KRYS_NODISCARD bool CanRenderCombiningCharacterSequence(utf8_stringview text) const noexcept;

    void SetIsUsedInSystemFallbackFontCache() noexcept
    {
      _isUsedInSystemFallbackFontCache = true;
    }

    KRYS_NODISCARD bool IsUsedInSystemFallbackFontCache() const noexcept
    {
      return _isUsedInSystemFallbackFontCache;
    }

    KRYS_NODISCARD const Attributes &GetAttributes() const noexcept
    {
      return _attributes;
    }

    KRYS_NODISCARD boo::ColorGlyphType ColorGlyphType(Glyph glyph) const noexcept;

  private:
    void PlatformInit() noexcept;
    void PlatformGlyphInit() noexcept;
    void PlatformCharWidthInit() noexcept;

    void InitCharWidths() noexcept;

    KRYS_NODISCARD RefPtr<Font> CreateScaledFont(const FontDescription &fontDescription,
                                                 float scaleFactor) const noexcept;
    KRYS_NODISCARD RefPtr<Font> PlatformCreateScaledFont(const FontDescription &fontDescription,
                                                         float scaleFactor) const noexcept;
    KRYS_NODISCARD RefPtr<Font> CreateHalfWidthFont() const noexcept;
    KRYS_NODISCARD RefPtr<Font> PlatformCreateHalfWidthFont() const noexcept;

    KRYS_NODISCARD DerivedFonts &EnsureDerivedFontData() const noexcept;

    KRYS_NODISCARD FloatRect PlatformBoundsForGlyph(Glyph glyph) const noexcept;
    KRYS_NODISCARD float PlatformWidthForGlyph(Glyph glyph) const noexcept;
  };
}
