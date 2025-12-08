#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/StronglyTypedValue.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  enum class VertexAttributeType
  {
    Float,
    Int32,
    UInt32,
    Double,
    UnsignedByte,
  };

  enum class VertexInputRate
  {
    PerVertex,
    PerInstance
  };

  struct IsNormalized : public StronglyTypedBool<IsNormalized>
  {
    using StronglyTypedBool<IsNormalized>::StronglyTypedBool;
  };

  struct VertexBufferElement
  {
    VertexAttributeType Type;
    uint32 Count;
    bool Normalized;
    bool Enabled;
    VertexInputRate InputRate;

    constexpr VertexBufferElement(VertexAttributeType type, uint32 count,
                                  VertexInputRate inputRate = VertexInputRate::PerVertex) noexcept
        : Type(type), Count(count), Normalized(false), Enabled(true), InputRate(inputRate)
    {
    }

    constexpr VertexBufferElement(VertexAttributeType type, uint32 count, IsNormalized normalized,
                                  VertexInputRate inputRate = VertexInputRate::PerVertex) noexcept
        : Type(type), Count(count), Normalized(normalized), Enabled(true), InputRate(inputRate)
    {
    }

    constexpr VertexBufferElement(VertexAttributeType type, uint32 count, IsEnabled enabled,
                                  VertexInputRate inputRate = VertexInputRate::PerVertex) noexcept
        : Type(type), Count(count), Normalized(false), Enabled(enabled), InputRate(inputRate)
    {
    }

    constexpr VertexBufferElement(VertexAttributeType type, uint32 count, IsNormalized normalized,
                                  IsEnabled enabled,
                                  VertexInputRate inputRate = VertexInputRate::PerVertex) noexcept
        : Type(type), Count(count), Normalized(normalized), Enabled(enabled), InputRate(inputRate)
    {
    }

    static uint32 GetSizeOfType(VertexAttributeType type) noexcept
    {
      switch (type)
      {
        case VertexAttributeType::Int32:        return sizeof(int32);
        case VertexAttributeType::UInt32:       return sizeof(uint32);
        case VertexAttributeType::Float:        return sizeof(float);
        case VertexAttributeType::Double:       return sizeof(double);
        case VertexAttributeType::UnsignedByte: return sizeof(byte);
        default:                                return 0;
      }
    }
  };

  using VertexBufferLayout = List<VertexBufferElement>;

  namespace Vertex
  {
    struct Position2D_UV
    {
      Maths::Vec2 Position;
      Maths::Vec2 TexCoord;

      constexpr static VertexBufferLayout Layout()
      {
        return {
          {VertexAttributeType::Float, 2}, // Position
          {VertexAttributeType::Float, 2}, // TexCoord
        };
      }
    };

    struct Position2D_Colourf_UV
    {
      Maths::Vec2 Position;
      Colourf Colour;
      Maths::Vec2 TexCoord;

      constexpr static VertexBufferLayout Layout()
      {
        return {
          {VertexAttributeType::Float, 2}, // Position
          {VertexAttributeType::Float, 4}, // Colour
          {VertexAttributeType::Float, 2}, // TexCoord
        };
      }
    };

    struct Position2D_ColourbPremultiplied_UV
    {
      Maths::Vec2 Position;
      ColourbPremultiplied Colour;
      Maths::Vec2 TexCoord;

      constexpr static VertexBufferLayout Layout()
      {
        return {
          {VertexAttributeType::Float, 2},                            // Position
          {VertexAttributeType::UnsignedByte, 4, IsNormalized(true)}, // Colour
          {VertexAttributeType::Float, 2},                            // TexCoord
        };
      }
    };

    struct Position3D_UV
    {
      Maths::Vec3 Position;
      Maths::Vec2 TexCoord;

      constexpr static VertexBufferLayout Layout()
      {
        return {
          {VertexAttributeType::Float, 3}, // Position
          {VertexAttributeType::Float, 2}, // TexCoord
        };
      }
    };

    struct Position3D_Normal
    {
      Maths::Vec3 Position;
      Maths::Vec3 Normal;

      constexpr static VertexBufferLayout Layout()
      {
        return {
          {VertexAttributeType::Float, 3}, // Position
          {VertexAttributeType::Float, 3}, // Normal
        };
      }
    };

    struct Position3D_Colourf_UV
    {
      Maths::Vec3 Position;
      Colourf Colour;
      Maths::Vec2 TexCoord;

      constexpr static VertexBufferLayout Layout()
      {
        return {
          {VertexAttributeType::Float, 3}, // Position
          {VertexAttributeType::Float, 4}, // Colour
          {VertexAttributeType::Float, 2}, // TexCoord
        };
      }
    };

    struct Position3D_Normal_UV
    {
      Maths::Vec3 Position;
      Maths::Vec3 Normal;
      Maths::Vec2 TexCoord;

      constexpr static VertexBufferLayout Layout()
      {
        return {
          {VertexAttributeType::Float, 3}, // Position
          {VertexAttributeType::Float, 3}, // Normal
          {VertexAttributeType::Float, 2}, // TexCoord
        };
      }
    };
  }
}