#pragma once

#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/ImageBufferBackend.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"

namespace krys::boo::tests
{
  /// @brief A stand-in for a host's accelerated backend: it records what's drawn, and counts its flushes and
  /// preparations for display.
  class TestAcceleratedBackend final : public ImageBufferBackend
  {
  private:
    display_list::RecorderImpl _recorder;

  public:
    static constexpr RenderingMode BackendRenderingMode = RenderingMode::Accelerated;

    int32 Flushes {0};
    int32 Preparations {0};

    KRYS_NODISCARD static UniquePtr<TestAcceleratedBackend> Create(const Parameters &parameters) noexcept
    {
      return CreateUnique<TestAcceleratedBackend>(parameters);
    }

    explicit TestAcceleratedBackend(const Parameters &parameters) noexcept
        : ImageBufferBackend(parameters), _recorder(FloatSize(parameters.BackendSize))
    {
    }

    KRYS_NODISCARD GraphicsContext &Context() noexcept override
    {
      return _recorder;
    }
    void FlushContext() noexcept override
    {
      ++Flushes;
    }
    void PrepareForDisplay() noexcept override
    {
      ++Preparations;
    }
  };

  /// @brief A host that makes accelerated buffers with `TestAcceleratedBackend`, and leaves the rest to the
  /// page.
  class AcceleratingChromeClient final : public ChromeClient
  {
  public:
    mutable int32 BuffersMade {0};

    void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept override
    {
    }

    KRYS_NODISCARD RefPtr<ImageBuffer>
      CreateImageBuffer(const FloatSize &size, RenderingMode mode, RenderingPurpose purpose,
                        float resolutionScale, const DestinationColorSpace &) const noexcept override
    {
      if (mode != RenderingMode::Accelerated)
      {
        return nullptr;
      }
      ++BuffersMade;
      return ImageBuffer::Create<TestAcceleratedBackend>(size, resolutionScale, purpose);
    }
  };
}
