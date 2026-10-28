#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class DestinationColorSpace;
  class FloatSize;
  class ImageBuffer;
  enum class RenderingMode : uint8;
  enum class RenderingPurpose : uint8;

  /// @brief The host's graphics: the image buffers it makes with its own backends.
  // FIXME: The display's identifier, WebGL and WebGPU contexts, and serialised image buffers, once there are
  // any; image buffers' pixel formats come with the formats (sRGB, 8 bits a channel).
  class GraphicsClient
  {
    friend class ImageBuffer;
    KRYS_NON_COPY_MOVABLE(GraphicsClient);

  public:
    GraphicsClient() noexcept = default;
    virtual ~GraphicsClient() noexcept = default;

  private:
    // Called by passing GraphicsClient into ImageBuffer functions.
    KRYS_NODISCARD virtual RefPtr<ImageBuffer>
      CreateImageBuffer(const FloatSize &, RenderingMode, RenderingPurpose, float resolutionScale,
                        const DestinationColorSpace &) const noexcept = 0;
  };
}
