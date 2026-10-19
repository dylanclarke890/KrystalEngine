#include "Krystal.Booey/Graphics/ImageBufferDisplayListBackend.hpp"

namespace krys::boo
{
  UniquePtr<ImageBufferDisplayListBackend>
    ImageBufferDisplayListBackend::Create(const Parameters &parameters) noexcept
  {
    return CreateUnique<ImageBufferDisplayListBackend>(parameters);
  }

  ImageBufferDisplayListBackend::ImageBufferDisplayListBackend(const Parameters &parameters) noexcept
      : ImageBufferBackend(parameters), _drawingContext(FloatSize(parameters.BackendSize))
  {
  }

  GraphicsContext &ImageBufferDisplayListBackend::Context() noexcept
  {
    return _drawingContext;
  }

  Ref<const display_list::DisplayList> ImageBufferDisplayListBackend::CopyDisplayList() noexcept
  {
    return _drawingContext.CopyDisplayList();
  }
}
