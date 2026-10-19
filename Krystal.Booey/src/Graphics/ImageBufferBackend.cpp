#include "Krystal.Booey/Graphics/ImageBufferBackend.hpp"

namespace krys::boo
{
  ImageBufferBackend::ImageBufferBackend(const Parameters &parameters) noexcept : _parameters(parameters)
  {
  }

  ImageBufferBackend::~ImageBufferBackend() noexcept = default;

  AffineTransform ImageBufferBackend::CalculateBaseTransform(const Parameters &parameters) noexcept
  {
    AffineTransform baseTransform;
    baseTransform.Scale(parameters.ResolutionScale);
    return baseTransform;
  }
}
