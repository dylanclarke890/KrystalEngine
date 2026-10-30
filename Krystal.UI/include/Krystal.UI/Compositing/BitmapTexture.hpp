#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Gfx/Device.hpp"

namespace krys::ui
{
  /// @brief A texture a compositor draws: a layer's tile, or a surface.
  class BitmapTexture final : public RefCounted<BitmapTexture>
  {
    KRYS_NON_COPY_MOVABLE(BitmapTexture);

  public:
    enum class Flags : uint8
    {
      SupportsAlpha,
      DepthBuffer,
    };

  private:
    gfx::Device &_device;
    EnumSet<Flags> _flags;
    boo::IntSize _size;
    gfx::TextureHandle _texture;

  public:
    KRYS_NODISCARD static Ref<BitmapTexture> Create(gfx::Device &device, const boo::IntSize &size,
                                                    EnumSet<Flags> flags = {}) noexcept;
    ~BitmapTexture() noexcept;

    KRYS_NODISCARD const boo::IntSize &Size() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD EnumSet<Flags> GetFlags() const noexcept
    {
      return _flags;
    }
    KRYS_NODISCARD bool IsOpaque() const noexcept
    {
      return !_flags.Contains(Flags::SupportsAlpha);
    }

    /// @brief The texture, invalid when it couldn't be created.
    KRYS_NODISCARD gfx::TextureHandle Handle() const noexcept
    {
      return _texture;
    }

    void Reset(const boo::IntSize &size, EnumSet<Flags> flags = {}) noexcept;

    KRYS_NODISCARD int32 NumberOfBytes() const noexcept
    {
      return Size().Width() * Size().Height() * 32 >> 3;
    }

  private:
    BitmapTexture(gfx::Device &device, const boo::IntSize &size, EnumSet<Flags> flags) noexcept;

    void CreateTexture() noexcept;
    void DestroyTexture() noexcept;
  };
}
