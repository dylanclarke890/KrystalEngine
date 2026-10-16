#pragma once

#include "Krystal.Core/Attributes.hpp"
#include <memory>

struct hb_blob_t;
struct hb_buffer_t;
struct hb_face_t;
struct hb_font_t;

namespace krys::boo
{
  template <typename T>
  struct HbPtrDeleter
  {
    void operator()(T *ptr) const noexcept = delete;
  };

  /// @brief Owns a HarfBuzz object.
  template <typename T>
  using HbUniquePtr = std::unique_ptr<T, HbPtrDeleter<T>>;

  template <>
  struct HbPtrDeleter<hb_font_t>
  {
    void operator()(hb_font_t *ptr) const noexcept;
  };

  template <>
  struct HbPtrDeleter<hb_buffer_t>
  {
    void operator()(hb_buffer_t *ptr) const noexcept;
  };

  template <>
  struct HbPtrDeleter<hb_face_t>
  {
    void operator()(hb_face_t *ptr) const noexcept;
  };

  template <>
  struct HbPtrDeleter<hb_blob_t>
  {
    void operator()(hb_blob_t *ptr) const noexcept;
  };
}
