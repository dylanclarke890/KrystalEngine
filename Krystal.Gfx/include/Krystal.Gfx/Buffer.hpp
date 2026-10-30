#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::gfx
{
  /// @brief What a buffer can be bound as.
  enum class BufferUsage : uint8
  {
    None = 0,
    Vertex = 1 << 0,
    Index = 1 << 1,
    Uniform = 1 << 2,
    Storage = 1 << 3,
    CopySource = 1 << 4,
    CopyDestination = 1 << 5,
  };

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
    BufferUsage Usage {BufferUsage::Vertex};
    MemoryUsage Memory {MemoryUsage::GpuOnly};
    /// @brief Copied into the start of the buffer; may be shorter than `Size`.
    Span<const byte> InitialData {};
    utf8_stringview DebugName {};
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::gfx::BufferUsage, 6uz)
