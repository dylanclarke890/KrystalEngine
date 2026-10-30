#pragma once

#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/ImageBufferBackend.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief An image buffer backend that records what's drawn into a display list, for a drawing backend to
  /// render when it draws the buffer.
  /// @note Krystal.UI renders the recorded drawing into a texture each time it draws the buffer.
  class ImageBufferDisplayListBackend final : public ImageBufferBackend
  {
  private:
    display_list::RecorderImpl _drawingContext;

  public:
    static constexpr RenderingMode BackendRenderingMode = RenderingMode::DisplayList;

    KRYS_NODISCARD static UniquePtr<ImageBufferDisplayListBackend> Create(const Parameters &) noexcept;

    KRYS_NODISCARD GraphicsContext &Context() noexcept override;

    /// @brief What's been drawn into the buffer so far.
    KRYS_NODISCARD Ref<const display_list::DisplayList> CopyDisplayList() noexcept;

    explicit ImageBufferDisplayListBackend(const Parameters &) noexcept;
  };
}
