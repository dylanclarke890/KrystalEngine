#pragma once

#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/StronglyTypedValue.hpp"
#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Enums/VertexAttributeType.hpp"
#include "Krystal.Gfx/Enums/VertexInputRate.hpp"

namespace krys::Gfx
{
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
      Vec2 Position;
      Vec2 TexCoord;

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
      Vec2 Position;
      Colourf Colour;
      Vec2 TexCoord;

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
      Vec2 Position;
      ColourbPremultiplied Colour;
      Vec2 TexCoord;

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
      Vec3 Position;
      Vec2 TexCoord;

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
      Vec3 Position;
      Vec3 Normal;

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
      Vec3 Position;
      Colourf Colour;
      Vec2 TexCoord;

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
      Vec3 Position;
      Vec3 Normal;
      Vec2 TexCoord;

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