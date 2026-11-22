#pragma once

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"

namespace krys::gfx
{
  /// @brief What a buffer can be bound as.
  enum class BufferUsage : uint8
  {
    Vertex,
    Index,
    Uniform,
    Storage,
    CopySource,
    CopyDestination,
  };

  consteval BufferUsage MaxEnumValue(BufferUsage) noexcept
  {
    return BufferUsage::CopyDestination;
  }

  /// @brief How often the CPU writes a buffer.
  enum class MemoryUsage : uint8
  {
    /// @brief Written rarely (at creation, or with `Device::WriteBuffer`), read by the GPU.
    GpuOnly,
    /// @brief Written often by the CPU.
    Dynamic,
  };

  struct BufferDesc
  {
    uint64 Size {0u};
    EnumSet<BufferUsage> Usage {BufferUsage::Vertex};
    MemoryUsage Memory {MemoryUsage::GpuOnly};
    /// @brief Copied into the start of the buffer; may be shorter than `Size`.
    Span<const byte> InitialData;
    utf8_stringview DebugName;
  };
}
