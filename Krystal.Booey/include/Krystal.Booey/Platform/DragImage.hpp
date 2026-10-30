#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class Image;
  class LocalFrame;

  namespace dom
  {
    class Element;
    class Node;
  }

  namespace url
  {
    class URL;
  }

  /// @brief A drag image: an image buffer the host draws under the cursor, its size in device pixels.
  using DragImageRef = RefPtr<ImageBuffer>;

  /// @brief The size of @p image in device pixels; empty if there's none.
  KRYS_NODISCARD IntSize DragImageSize(const DragImageRef &image) noexcept;

  // These functions should be memory neutral, eg. if they return a newly allocated image,
  // they should release the input image. As a corollary these methods don't guarantee
  // the input image ref will still be valid after they have been called.
  KRYS_NODISCARD DragImageRef FitDragImageToMaxSize(DragImageRef image, const IntSize &srcSize,
                                                    const IntSize &dstSize) noexcept;
  KRYS_NODISCARD DragImageRef ScaleDragImage(DragImageRef image, FloatSize scale) noexcept;
  KRYS_NODISCARD DragImageRef PlatformAdjustDragImageForDeviceScaleFactor(DragImageRef image,
                                                                          float deviceScaleFactor) noexcept;
  KRYS_NODISCARD DragImageRef DissolveDragImageToFraction(DragImageRef image, float delta) noexcept;
  /// @brief A drag image of @p image at its size, oriented by @p orientation; null if it has no size.
  KRYS_NODISCARD DragImageRef CreateDragImageFromImage(Image *image, ImageOrientation orientation) noexcept;
  /// @brief The icon for a dragged image's file, which the host doesn't give: null.
  KRYS_NODISCARD DragImageRef CreateDragImageIconForCachedImageFilename(utf8_stringview filename) noexcept;

  /// @brief A drag image and what goes with it.
  /// FIXME: The text indicator, with text indicators.
  struct DragImageData
  {
    DragImageRef DragImage;
  };

  /// @brief A snapshot of @p node, as it's painted when dragged.
  KRYS_NODISCARD DragImageRef CreateDragImageForNode(LocalFrame &frame, dom::Node &node) noexcept;
  /// @brief A snapshot of the frame's selection.
  KRYS_NODISCARD DragImageData CreateDragImageForSelection(LocalFrame &frame,
                                                           bool forceBlackText = false) noexcept;
  /// @brief A label for a dragged link: its text in bold over its URL, or the URL alone if there's no text.
  KRYS_NODISCARD DragImageData CreateDragImageForLink(dom::Element &element, url::URL &url,
                                                      utf8_stringview label,
                                                      float deviceScaleFactor) noexcept;

  /// @brief Where a link's drag image goes relative to the cursor.
  KRYS_NODISCARD IntPoint DragOffsetForLinkDragImage(const DragImageRef &dragImage) noexcept;
  /// @brief The point of a link's drag image that's under the cursor, as fractions of its size.
  KRYS_NODISCARD FloatPoint AnchorPointForLinkDragImage(const DragImageRef &dragImage) noexcept;

  /// @brief A drag image, owned.
  /// FIXME: The text indicator and visible path, with text indicators.
  class DragImage final
  {
    KRYS_NON_COPYABLE(DragImage);

  private:
    DragImageRef _dragImageRef;

  public:
    DragImage() noexcept = default;
    explicit DragImage(DragImageRef dragImageRef) noexcept : _dragImageRef(krys::move(dragImageRef))
    {
    }
    DragImage(DragImage &&) noexcept = default;
    DragImage &operator=(DragImage &&) noexcept = default;

    explicit operator bool() const noexcept
    {
      return !!_dragImageRef;
    }

    KRYS_NODISCARD const DragImageRef &Get() const noexcept
    {
      return _dragImageRef;
    }
  };
}
