#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <compare>
#include <functional>

namespace krys::gfx
{
  /// @brief A typed reference to a device resource: a 24-bit slot index and an 8-bit generation. 0 is
  /// invalid.
  /// @note The index is stored plus one, so slot 0 with generation 0 is still a valid handle.
  template <typename TTag>
  struct Handle
  {
    constexpr static uint32 IndexBits = 24u;
    constexpr static uint32 IndexMask = (1u << IndexBits) - 1u;
    /// @brief The largest slot index.
    constexpr static uint32 MaxIndex = IndexMask - 1u;

    uint32 Value {0u};

    KRYS_NODISCARD constexpr static Handle Create(uint32 index, uint8 generation) noexcept
    {
      return Handle {(static_cast<uint32>(generation) << IndexBits) | (index + 1u)};
    }

    KRYS_NODISCARD constexpr bool IsValid() const noexcept
    {
      return Value != 0u;
    }

    KRYS_NODISCARD constexpr uint32 Index() const noexcept
    {
      return (Value & IndexMask) - 1u;
    }

    KRYS_NODISCARD constexpr uint8 Generation() const noexcept
    {
      return static_cast<uint8>(Value >> IndexBits);
    }

    constexpr auto operator<=>(const Handle &) const noexcept = default;
  };

  using BufferHandle = Handle<struct BufferTag>;
  using TextureHandle = Handle<struct TextureTag>;
  using SamplerHandle = Handle<struct SamplerTag>;
  using ShaderHandle = Handle<struct ShaderTag>;
  using PipelineHandle = Handle<struct PipelineTag>;
  using SwapchainHandle = Handle<struct SwapchainTag>;
}

namespace std
{
  template <typename TTag>
  struct hash<krys::gfx::Handle<TTag>>
  {
    size_t operator()(const krys::gfx::Handle<TTag> &handle) const noexcept
    {
      return std::hash<krys::uint32> {}(handle.Value);
    }
  };
}
