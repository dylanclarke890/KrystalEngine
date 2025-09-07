#pragma once

#include "Krystal.Core/Core.hpp"

namespace Krys::Gfx
{
  enum class VertexAttributeType
  {
    Float,
    Int32,
    UInt32,
    Double
  };

  struct IsNormalized : TypedBool<IsNormalized>
  {
    explicit constexpr IsNormalized(bool value) noexcept : TypedBool<IsNormalized>(value)
    {
    }
  };

  struct VertexBufferElement
  {
    VertexAttributeType Type;
    uint32 Count;
    bool Normalized;
    bool Enabled;

    constexpr VertexBufferElement(VertexAttributeType type, uint32 count) noexcept
        : Type(type), Count(count), Normalized(false), Enabled(true)
    {
    }

    constexpr VertexBufferElement(VertexAttributeType type, uint32 count, IsNormalized normalized) noexcept
        : Type(type), Count(count), Normalized(normalized), Enabled(true)
    {
    }

    constexpr VertexBufferElement(VertexAttributeType type, uint32 count, IsEnabled enabled) noexcept
        : Type(VertexAttributeType::Float), Count(count), Normalized(false), Enabled(enabled)
    {
    }

    constexpr VertexBufferElement(VertexAttributeType type, uint32 count, IsNormalized normalized,
                                  IsEnabled enabled) noexcept
        : Type(type), Count(count), Normalized(normalized), Enabled(enabled)
    {
    }

    static uint32 GetSizeOfType(VertexAttributeType type) noexcept
    {
      switch (type)
      {
        case VertexAttributeType::Int32:  return sizeof(int32);
        case VertexAttributeType::UInt32: return sizeof(uint32);
        case VertexAttributeType::Float:  return sizeof(float);
        case VertexAttributeType::Double: return sizeof(double);
        default:                          return 0;
      }
    }
  };

  using VertexBufferLayout = List<VertexBufferElement>;

  namespace VertexLayouts
  {
    static const VertexBufferLayout Basic({
      {VertexAttributeType::Float, 3}, // Position
      {VertexAttributeType::Float, 3}, // Normal
      {VertexAttributeType::Float, 2}, // TexCoord
    });
  }
}