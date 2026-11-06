#include "Krystal.Text/HarfBuzz.hpp"
#include "Krystal.Text/FreeTypeFace.hpp"
#include <freetype/freetype.h>
#include <hb.h>

namespace krys::text
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

  void FreeTypeFace::ApplyVariations(hb_font_t *font) const noexcept
  {
    if (_variations.empty())
    {
      return;
    }
    List<hb_variation_t> variations;
    for (const auto &variation : _variations)
    {
      variations.push_back({static_cast<hb_tag_t>(variation.Tag), variation.Value});
    }
    hb_font_set_variations(font, variations.data(), static_cast<unsigned>(variations.size()));
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
}
