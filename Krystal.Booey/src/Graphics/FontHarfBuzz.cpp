#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FreeTypeFace.hpp"
#include "Krystal.Booey/Graphics/HbUniquePtr.hpp"
#include <algorithm>
#include <cmath>
#include <freetype/freetype.h>
#include <hb.h>
#include <limits>

namespace krys::boo
{
  void HbPtrDeleter<hb_font_t>::operator()(hb_font_t *ptr) const noexcept
  {
    hb_font_destroy(ptr);
  }

  void HbPtrDeleter<hb_buffer_t>::operator()(hb_buffer_t *ptr) const noexcept
  {
    hb_buffer_destroy(ptr);
  }

  void HbPtrDeleter<hb_face_t>::operator()(hb_face_t *ptr) const noexcept
  {
    hb_face_destroy(ptr);
  }

  void HbPtrDeleter<hb_blob_t>::operator()(hb_blob_t *ptr) const noexcept
  {
    hb_blob_destroy(ptr);
  }

  namespace
  {
    constexpr char32 TextVariationSelector = 0xFE0E;
    constexpr char32 EmojiVariationSelector = 0xFE0F;

    KRYS_NODISCARD hb_position_t FloatToHarfBuzzPosition(float value) noexcept
    {
      static constexpr float HbPosition = 1 << 16;
      return static_cast<hb_position_t>(std::clamp<double>(std::round(value * HbPosition),
                                                           std::numeric_limits<hb_position_t>::min(),
                                                           std::numeric_limits<hb_position_t>::max()));
    }

    /// @brief The glyph of @p unicode (with @p variation) in @p font's parent, which reads the face's tables.
    KRYS_NODISCARD Maybe<hb_codepoint_t> GlyphFor(hb_font_t *font, const Font &context,
                                                  hb_codepoint_t unicode,
                                                  Maybe<hb_codepoint_t> variation) noexcept
    {
      if (FontCascade::TreatAsSpace(unicode))
      {
        unicode = text::Space;
      }
      else if (FontCascade::TreatAsZeroWidthSpaceInComplexScript(unicode))
      {
        unicode = text::ZeroWidthSpace;
      }

      auto *parent = hb_font_get_parent(font);
      hb_codepoint_t returnValue;
      if (hb_font_get_glyph(parent, unicode, variation.value_or(0), &returnValue))
      {
        return returnValue;
      }

      if (!variation)
      {
        return null;
      }

      // If we failed to get a glyph with a variation, try to get a glyph without
      // the variation unless emoji is requested and font doesn't support colors
      // or text is requested and it's a color font.
      // FIXME: it would be better to check if the font has a color/text glyph for the given codepoint.
      bool isColorBitmapFont = context.PlatformData().IsColorBitmapFont();
      if (*variation == EmojiVariationSelector && !isColorBitmapFont)
      {
        return null;
      }

      if (*variation == TextVariationSelector && isColorBitmapFont)
      {
        return null;
      }

      if (hb_font_get_glyph(parent, unicode, 0, &returnValue))
      {
        return returnValue;
      }
      return null;
    }

    KRYS_NODISCARD hb_font_funcs_t *HarfBuzzFontFunctions() noexcept
    {
      static hb_font_funcs_t *fontFunctions = []
      {
        auto *functions = hb_font_funcs_create();

        hb_font_funcs_set_nominal_glyph_func(
          functions,
          [](hb_font_t *font, void *context, hb_codepoint_t unicode, hb_codepoint_t *glyph,
             void *) -> hb_bool_t
          {
            auto hbGlyph = GlyphFor(font, *static_cast<const Font *>(context), unicode, null);
            if (!hbGlyph)
            {
              return false;
            }
            *glyph = *hbGlyph;
            return true;
          },
          nullptr, nullptr);

        hb_font_funcs_set_variation_glyph_func(
          functions,
          [](hb_font_t *font, void *context, hb_codepoint_t unicode, hb_codepoint_t variation,
             hb_codepoint_t *glyph, void *) -> hb_bool_t
          {
            auto hbGlyph = GlyphFor(font, *static_cast<const Font *>(context), unicode, variation);
            if (!hbGlyph)
            {
              return false;
            }
            *glyph = *hbGlyph;
            return true;
          },
          nullptr, nullptr);

        // Advances and extents come from the font, so shaped text measures as the simple path does.
        hb_font_funcs_set_glyph_h_advance_func(
          functions,
          [](hb_font_t *, void *context, hb_codepoint_t glyph, void *) -> hb_position_t
          {
            auto &font = *static_cast<const Font *>(context);
            return FloatToHarfBuzzPosition(
              font.WidthForGlyph(static_cast<Glyph>(glyph), Font::SyntheticBoldInclusion::Exclude));
          },
          nullptr, nullptr);

        hb_font_funcs_set_glyph_extents_func(
          functions,
          [](hb_font_t *, void *context, hb_codepoint_t glyph, hb_glyph_extents_t *extents,
             void *) -> hb_bool_t
          {
            auto &font = *static_cast<const Font *>(context);
            auto bounds = font.BoundsForGlyph(static_cast<Glyph>(glyph));
            extents->x_bearing = FloatToHarfBuzzPosition(bounds.X());
            extents->y_bearing = FloatToHarfBuzzPosition(-bounds.Y());
            extents->width = FloatToHarfBuzzPosition(bounds.Width());
            extents->height = FloatToHarfBuzzPosition(-bounds.Height());
            return true;
          },
          nullptr, nullptr);

        hb_font_funcs_make_immutable(functions);
        return functions;
      }();
      return fontFunctions;
    }
  }

  hb_face_t *FreeTypeFace::HarfBuzzFace() const noexcept
  {
    if (!_harfBuzzFace)
    {
      // The font data outlives the face: this face owns both.
      auto bytes = _data->Bytes();
      HbUniquePtr<hb_blob_t> blob(hb_blob_create(reinterpret_cast<const char *>(bytes.data()),
                                                 static_cast<unsigned>(bytes.size()), HB_MEMORY_MODE_READONLY,
                                                 nullptr, nullptr));
      // The low 16 bits of a FreeType face index are the face's index in a collection.
      auto faceIndex = static_cast<unsigned>(_face->face_index & 0xFFFF);
      _harfBuzzFace.reset(hb_face_create(blob.get(), faceIndex));
    }
    return _harfBuzzFace.get();
  }

  hb_font_t *Font::HarfBuzzFont() const noexcept
  {
    if (_harfBuzzFont)
    {
      return _harfBuzzFont.get();
    }

    auto *face = PlatformData().Face();
    if (!face)
    {
      return nullptr;
    }

    auto *hbFace = face->HarfBuzzFace();
    if (!hbFace || hb_face_get_glyph_count(hbFace) == 0)
    {
      return nullptr;
    }

    // FIXME: Variable fonts are shaped at their default instance.
    HbUniquePtr<hb_font_t> parent(hb_font_create(hbFace));

    // Create a subfont with custom functions so that the missing ones are taken from the parent.
    _harfBuzzFont.reset(hb_font_create_sub_font(parent.get()));
    hb_font_set_funcs(_harfBuzzFont.get(), HarfBuzzFontFunctions(), const_cast<Font *>(this), nullptr);

    float size = PlatformData().Size();
    auto scale = FloatToHarfBuzzPosition(size);
    hb_font_set_scale(_harfBuzzFont.get(), scale, scale);
    hb_font_set_ptem(_harfBuzzFont.get(), size);
    hb_font_make_immutable(_harfBuzzFont.get());
    return _harfBuzzFont.get();
  }
}
