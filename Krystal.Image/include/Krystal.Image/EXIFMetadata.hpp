#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Image/ImageOrientation.hpp"

namespace krys::img
{
  /// @brief What an image's EXIF metadata says about showing it: its orientation, and the size it prefers.
  struct EXIFMetadata
  {
    ImageOrientation Orientation {ImageOrientation::Orientation::None};
    /// @brief The preferred density-corrected size: the EXIF pixel dimensions, when the image's resolution
    /// (in inches) scales its size to them.
    Maybe<SizeI> DensityCorrectedSize;

    /// @brief Reads the metadata of an image of @p physicalSize from @p tiff, the TIFF block EXIF stores
    /// (from its byte order mark, `II` or `MM`). What's missing or malformed is left at its default.
    KRYS_NODISCARD static EXIFMetadata Read(Span<const uint8> tiff, const SizeI &physicalSize) noexcept;
  };
}
