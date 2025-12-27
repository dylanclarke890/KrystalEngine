#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <bit>

namespace Krys::Maths
{
  struct Morton
  {
    STATIC_CLASS(Morton)

#pragma region uint8

    NO_DISCARD static constexpr uint16 Encode(uint8 x, uint8 y) noexcept
    {
      uint16 a(x);
      uint16 b(y);

      a = ((a << 4u) | a) & static_cast<uint16>(0x0F0F);
      b = ((b << 4u) | b) & static_cast<uint16>(0x0F0F);

      a = ((a << 2u) | a) & static_cast<uint16>(0x3333);
      b = ((b << 2u) | b) & static_cast<uint16>(0x3333);

      a = ((a << 1u) | a) & static_cast<uint16>(0x5555);
      b = ((b << 1u) | b) & static_cast<uint16>(0x5555);

      return a | static_cast<uint16>(b << 1u);
    }

    NO_DISCARD static constexpr uint32 Encode(uint8 x, uint8 y, uint8 z) noexcept
    {
      uint32 a(x);
      uint32 b(y);
      uint32 c(z);

      a = ((a << 16u) | a) & static_cast<uint32>(0xFF'00'00'FFu);
      b = ((b << 16u) | b) & static_cast<uint32>(0xFF'00'00'FFu);
      c = ((c << 16u) | c) & static_cast<uint32>(0xFF'00'00'FFu);

      a = ((a << 8u) | a) & static_cast<uint32>(0x0F'00'F0'0Fu);
      b = ((b << 8u) | b) & static_cast<uint32>(0x0F'00'F0'0Fu);
      c = ((c << 8u) | c) & static_cast<uint32>(0x0F'00'F0'0Fu);

      a = ((a << 4u) | a) & static_cast<uint32>(0xC3'0C'30'C3u);
      b = ((b << 4u) | b) & static_cast<uint32>(0xC3'0C'30'C3u);
      c = ((c << 4u) | c) & static_cast<uint32>(0xC3'0C'30'C3u);

      a = ((a << 2u) | a) & static_cast<uint32>(0x49'24'92'49u);
      b = ((b << 2u) | b) & static_cast<uint32>(0x49'24'92'49u);
      c = ((c << 2u) | c) & static_cast<uint32>(0x49'24'92'49u);

      return a | (b << 1u) | (c << 2u);
    }

    NO_DISCARD static constexpr uint32 Encode(uint8 x, uint8 y, uint8 z, uint8 w) noexcept
    {
      uint32 a(x);
      uint32 b(y);
      uint32 c(z);
      uint32 d(w);

      a = ((a << 12u) | a) & static_cast<uint32>(0x00'0F'00'0Fu);
      b = ((b << 12u) | b) & static_cast<uint32>(0x00'0F'00'0Fu);
      c = ((c << 12u) | c) & static_cast<uint32>(0x00'0F'00'0Fu);
      d = ((d << 12u) | d) & static_cast<uint32>(0x00'0F'00'0Fu);

      a = ((a << 6u) | a) & static_cast<uint32>(0x03'03'03'03u);
      b = ((b << 6u) | b) & static_cast<uint32>(0x03'03'03'03u);
      c = ((c << 6u) | c) & static_cast<uint32>(0x03'03'03'03u);
      d = ((d << 6u) | d) & static_cast<uint32>(0x03'03'03'03u);

      a = ((a << 3u) | a) & static_cast<uint32>(0x11'11'11'11u);
      b = ((b << 3u) | b) & static_cast<uint32>(0x11'11'11'11u);
      c = ((c << 3u) | c) & static_cast<uint32>(0x11'11'11'11u);
      d = ((d << 3u) | d) & static_cast<uint32>(0x11'11'11'11u);

      return a | (b << 1u) | (c << 2u) | (d << 3u);
    }

    NO_DISCARD static constexpr uint16 Encode(const Vector2<uint8> &v) noexcept
    {
      return Encode(v.x, v.y);
    }

    NO_DISCARD static constexpr uint32 Encode(const Vector3<uint8> &v) noexcept
    {
      return Encode(v.x, v.y, v.z);
    }

    NO_DISCARD static constexpr uint32 Encode(const Vector4<uint8> &v) noexcept
    {
      return Encode(v.x, v.y, v.z, v.w);
    }

    NO_DISCARD static constexpr Vector2<uint8> Decode(uint16 x)
    {
      uint16 a(x);
      uint16 b(x >> 1u);

      a = a & static_cast<uint16>(0x5555);
      b = b & static_cast<uint16>(0x5555);

      a = ((a >> 1u) | a) & static_cast<uint16>(0x3333);
      b = ((b >> 1u) | b) & static_cast<uint16>(0x3333);

      a = ((a >> 2u) | a) & static_cast<uint16>(0x0F0F);
      b = ((b >> 2u) | b) & static_cast<uint16>(0x0F0F);

      a = ((a >> 4u) | a) & static_cast<uint16>(0x00FF);
      b = ((b >> 4u) | b) & static_cast<uint16>(0x00FF);

      a = ((a >> 8u) | a) & static_cast<uint16>(0xFFFF);
      b = ((b >> 8u) | b) & static_cast<uint16>(0xFFFF);

      return Vector2<uint8>((uint8)a, (uint8)b);
    }

#pragma endregion

#pragma region int8

    NO_DISCARD static constexpr int16 Encode(int8 x, int8 y) noexcept
    {
      uint8 a = std::bit_cast<uint8>(x);
      uint8 b = std::bit_cast<uint8>(y);

      uint16 result = Encode(a, b);
      return std::bit_cast<int16>(result);
    }

    NO_DISCARD static constexpr int32 Encode(int8 x, int8 y, int8 z)
    {
      uint8 a = std::bit_cast<uint8>(x);
      uint8 b = std::bit_cast<uint8>(y);
      uint8 c = std::bit_cast<uint8>(z);

      uint32 result = Encode(a, b, c);
      return std::bit_cast<int32>(result);
    }

    NO_DISCARD static constexpr int32 Encode(int8 x, int8 y, int8 z, int8 w)
    {
      uint8 a = std::bit_cast<uint8>(x);
      uint8 b = std::bit_cast<uint8>(y);
      uint8 c = std::bit_cast<uint8>(z);
      uint8 d = std::bit_cast<uint8>(w);

      uint32 result = Encode(a, b, c, d);
      return std::bit_cast<int32>(result);
    }

    NO_DISCARD static constexpr int16 Encode(const Vector2<int8> &v) noexcept
    {
      return Encode(v.x, v.y);
    }

    NO_DISCARD static constexpr int32 Encode(const Vector3<int8> &v) noexcept
    {
      return Encode(v.x, v.y, v.z);
    }

    NO_DISCARD static constexpr int32 Encode(const Vector4<int8> &v) noexcept
    {
      return Encode(v.x, v.y, v.z, v.w);
    }

#pragma endregion

#pragma region uint16

    NO_DISCARD static constexpr uint32 Encode(uint16 x, uint16 y) noexcept
    {
      uint32 a(x);
      uint32 b(y);

      a = ((a << 8u) | a) & static_cast<uint32>(0x00'FF'00'FF);
      b = ((b << 8u) | b) & static_cast<uint32>(0x00'FF'00'FF);

      a = ((a << 4u) | a) & static_cast<uint32>(0x0F'0F'0F'0F);
      b = ((b << 4u) | b) & static_cast<uint32>(0x0F'0F'0F'0F);

      a = ((a << 2u) | a) & static_cast<uint32>(0x33'33'33'33);
      b = ((b << 2u) | b) & static_cast<uint32>(0x33'33'33'33);

      a = ((a << 1u) | a) & static_cast<uint32>(0x55'55'55'55);
      b = ((b << 1u) | b) & static_cast<uint32>(0x55'55'55'55);

      return a | (b << 1u);
    }

    NO_DISCARD static constexpr uint64 Encode(uint16 x, uint16 y, uint16 z) noexcept
    {
      uint64 a(x);
      uint64 b(y);
      uint64 c(z);

      a = ((a << 32u) | a) & static_cast<uint64>(0xFF'FF'00'00'00'00'FF'FFull);
      b = ((b << 32u) | b) & static_cast<uint64>(0xFF'FF'00'00'00'00'FF'FFull);
      c = ((c << 32u) | c) & static_cast<uint64>(0xFF'FF'00'00'00'00'FF'FFull);

      a = ((a << 16u) | a) & static_cast<uint64>(0x00'FF'00'00'FF'00'00'FFull);
      b = ((b << 16u) | b) & static_cast<uint64>(0x00'FF'00'00'FF'00'00'FFull);
      c = ((c << 16u) | c) & static_cast<uint64>(0x00'FF'00'00'FF'00'00'FFull);

      a = ((a << 8u) | a) & static_cast<uint64>(0xF0'0F'00'F0'0F'00'F0'0Full);
      b = ((b << 8u) | b) & static_cast<uint64>(0xF0'0F'00'F0'0F'00'F0'0Full);
      c = ((c << 8u) | c) & static_cast<uint64>(0xF0'0F'00'F0'0F'00'F0'0Full);

      a = ((a << 4u) | a) & static_cast<uint64>(0x30'C3'0C'30'C3'0C'30'C3ull);
      b = ((b << 4u) | b) & static_cast<uint64>(0x30'C3'0C'30'C3'0C'30'C3ull);
      c = ((c << 4u) | c) & static_cast<uint64>(0x30'C3'0C'30'C3'0C'30'C3ull);

      a = ((a << 2u) | a) & static_cast<uint64>(0x92'49'24'92'49'24'92'49ull);
      b = ((b << 2u) | b) & static_cast<uint64>(0x92'49'24'92'49'24'92'49ull);
      c = ((c << 2u) | c) & static_cast<uint64>(0x92'49'24'92'49'24'92'49ull);

      return a | (b << 1u) | (c << 2u);
    }

    NO_DISCARD static constexpr uint64 Encode(uint16 x, uint16 y, uint16 z, uint16 w) noexcept
    {
      uint64 a(x);
      uint64 b(y);
      uint64 c(z);
      uint64 d(w);

      a = ((a << 24u) | a) & static_cast<uint64>(0x00'00'00'FF'00'00'00'FFull);
      b = ((b << 24u) | b) & static_cast<uint64>(0x00'00'00'FF'00'00'00'FFull);
      c = ((c << 24u) | c) & static_cast<uint64>(0x00'00'00'FF'00'00'00'FFull);
      d = ((d << 24u) | d) & static_cast<uint64>(0x00'00'00'FF'00'00'00'FFull);

      a = ((a << 12u) | a) & static_cast<uint64>(0x00'0F'00'0F'00'0F'00'0Full);
      b = ((b << 12u) | b) & static_cast<uint64>(0x00'0F'00'0F'00'0F'00'0Full);
      c = ((c << 12u) | c) & static_cast<uint64>(0x00'0F'00'0F'00'0F'00'0Full);
      d = ((d << 12u) | d) & static_cast<uint64>(0x00'0F'00'0F'00'0F'00'0Full);

      a = ((a << 6u) | a) & static_cast<uint64>(0x03'03'03'03'03'03'03'03ull);
      b = ((b << 6u) | b) & static_cast<uint64>(0x03'03'03'03'03'03'03'03ull);
      c = ((c << 6u) | c) & static_cast<uint64>(0x03'03'03'03'03'03'03'03ull);
      d = ((d << 6u) | d) & static_cast<uint64>(0x03'03'03'03'03'03'03'03ull);

      a = ((a << 3u) | a) & static_cast<uint64>(0x11'11'11'11'11'11'11'11ull);
      b = ((b << 3u) | b) & static_cast<uint64>(0x11'11'11'11'11'11'11'11ull);
      c = ((c << 3u) | c) & static_cast<uint64>(0x11'11'11'11'11'11'11'11ull);
      d = ((d << 3u) | d) & static_cast<uint64>(0x11'11'11'11'11'11'11'11ull);

      return a | (b << 1u) | (c << 2u) | (d << 3u);
    }

    NO_DISCARD static constexpr uint32 Encode(const Vector2<uint16> &v) noexcept
    {
      return Encode(v.x, v.y);
    }

    NO_DISCARD static constexpr uint64 Encode(const Vector3<uint16> &v) noexcept
    {
      return Encode(v.x, v.y, v.z);
    }

    NO_DISCARD static constexpr uint64 Encode(const Vector4<uint16> &v) noexcept
    {
      return Encode(v.x, v.y, v.z, v.w);
    }

    NO_DISCARD static constexpr Vector2<uint16> Decode(uint32 x)
    {
      uint32 a(x);
      uint32 b(x >> 1u);

      a = a & static_cast<uint32>(0x55'55'55'55);
      b = b & static_cast<uint32>(0x55'55'55'55);

      a = ((a >> 1u) | a) & static_cast<uint32>(0x33'33'33'33);
      b = ((b >> 1u) | b) & static_cast<uint32>(0x33'33'33'33);

      a = ((a >> 2u) | a) & static_cast<uint32>(0x0F'0F'0F'0F);
      b = ((b >> 2u) | b) & static_cast<uint32>(0x0F'0F'0F'0F);

      a = ((a >> 4u) | a) & static_cast<uint32>(0x00'FF'00'FF);
      b = ((b >> 4u) | b) & static_cast<uint32>(0x00'FF'00'FF);

      a = ((a >> 8u) | a) & static_cast<uint32>(0x00'00'FF'FF);
      b = ((b >> 8u) | b) & static_cast<uint32>(0x00'00'FF'FF);

      return Vector2<uint16>((uint16)a, (uint16)b);
    }

#pragma endregion

#pragma region int16

    NO_DISCARD static constexpr int32 Encode(int16 x, int16 y) noexcept
    {
      uint16 a = std::bit_cast<uint16>(x);
      uint16 b = std::bit_cast<uint16>(y);

      uint32 result = Encode(a, b);
      return std::bit_cast<int32>(result);
    }

    NO_DISCARD static constexpr int64 Encode(int16 x, int16 y, int16 z) noexcept
    {
      uint16 a = std::bit_cast<uint16>(x);
      uint16 b = std::bit_cast<uint16>(y);
      uint16 c = std::bit_cast<uint16>(z);

      uint64 result = Encode(a, b, c);
      return std::bit_cast<int64>(result);
    }

    NO_DISCARD static constexpr int64 Encode(int16 x, int16 y, int16 z, int16 w) noexcept
    {
      uint16 a = std::bit_cast<uint16>(x);
      uint16 b = std::bit_cast<uint16>(y);
      uint16 c = std::bit_cast<uint16>(z);
      uint16 d = std::bit_cast<uint16>(w);

      uint64 result = Encode(a, b, c, d);
      return std::bit_cast<int64>(result);
    }

    NO_DISCARD static constexpr int32 Encode(const Vector2<int16> &v) noexcept
    {
      return Encode(v.x, v.y);
    }

    NO_DISCARD static constexpr int64 Encode(const Vector3<int16> &v) noexcept
    {
      return Encode(v.x, v.y, v.z);
    }

    NO_DISCARD static constexpr int64 Encode(const Vector4<int16> &v) noexcept
    {
      return Encode(v.x, v.y, v.z, v.w);
    }

#pragma endregion

#pragma region uint32

    NO_DISCARD static constexpr uint64 Encode(uint32 x, uint32 y) noexcept
    {
      uint64 a(x);
      uint64 b(y);

      a = ((a << 16u) | a) & static_cast<uint64>(0x00'00'FF'FF'00'00'FF'FFull);
      b = ((b << 16u) | b) & static_cast<uint64>(0x00'00'FF'FF'00'00'FF'FFull);

      a = ((a << 8u) | a) & static_cast<uint64>(0x00'FF'00'FF'00'FF'00'FFull);
      b = ((b << 8u) | b) & static_cast<uint64>(0x00'FF'00'FF'00'FF'00'FFull);

      a = ((a << 4u) | a) & static_cast<uint64>(0x0F'0F'0F'0F'0F'0F'0F'0Full);
      b = ((b << 4u) | b) & static_cast<uint64>(0x0F'0F'0F'0F'0F'0F'0F'0Full);

      a = ((a << 2u) | a) & static_cast<uint64>(0x33'33'33'33'33'33'33'33ull);
      b = ((b << 2u) | b) & static_cast<uint64>(0x33'33'33'33'33'33'33'33ull);

      a = ((a << 1u) | a) & static_cast<uint64>(0x55'55'55'55'55'55'55'55ull);
      b = ((b << 1u) | b) & static_cast<uint64>(0x55'55'55'55'55'55'55'55ull);

      return a | (b << 1u);
    }

    NO_DISCARD static constexpr uint64 Encode(uint32 x, uint32 y, uint32 z) noexcept
    {
      uint64 a(x);
      uint64 b(y);
      uint64 c(z);

      a = ((a << 32u) | a) & static_cast<uint64>(0xFF'FF'00'00'00'00'FF'FFull);
      b = ((b << 32u) | b) & static_cast<uint64>(0xFF'FF'00'00'00'00'FF'FFull);
      c = ((c << 32u) | c) & static_cast<uint64>(0xFF'FF'00'00'00'00'FF'FFull);

      a = ((a << 16u) | a) & static_cast<uint64>(0x00'FF'00'00'FF'00'00'FFull);
      b = ((b << 16u) | b) & static_cast<uint64>(0x00'FF'00'00'FF'00'00'FFull);
      c = ((c << 16u) | c) & static_cast<uint64>(0x00'FF'00'00'FF'00'00'FFull);

      a = ((a << 8u) | a) & static_cast<uint64>(0xF0'0F'00'F0'0F'00'F0'0Full);
      b = ((b << 8u) | b) & static_cast<uint64>(0xF0'0F'00'F0'0F'00'F0'0Full);
      c = ((c << 8u) | c) & static_cast<uint64>(0xF0'0F'00'F0'0F'00'F0'0Full);

      a = ((a << 4u) | a) & static_cast<uint64>(0x30'C3'0C'30'C3'0C'30'C3ull);
      b = ((b << 4u) | b) & static_cast<uint64>(0x30'C3'0C'30'C3'0C'30'C3ull);
      c = ((c << 4u) | c) & static_cast<uint64>(0x30'C3'0C'30'C3'0C'30'C3ull);

      a = ((a << 2u) | a) & static_cast<uint64>(0x92'49'24'92'49'24'92'49ull);
      b = ((b << 2u) | b) & static_cast<uint64>(0x92'49'24'92'49'24'92'49ull);
      c = ((c << 2u) | c) & static_cast<uint64>(0x92'49'24'92'49'24'92'49ull);

      return a | (b << 1u) | (c << 2u);
    }

    NO_DISCARD static constexpr uint64 Encode(const Vector2<uint32> &v) noexcept
    {
      return Encode(v.x, v.y);
    }

    NO_DISCARD static constexpr uint64 Encode(const Vector3<uint32> &v) noexcept
    {
      return Encode(v.x, v.y, v.z);
    }

    NO_DISCARD static constexpr Vector2<uint32> Decode(uint64 x)
    {
      uint64 a(x);
      uint64 b(x >> 1u);

      a = a & static_cast<uint64>(0x55'55'55'55'55'55'55'55ull);
      b = b & static_cast<uint64>(0x55'55'55'55'55'55'55'55ull);

      a = ((a >> 1u) | a) & static_cast<uint64>(0x33'33'33'33'33'33'33'33ull);
      b = ((b >> 1u) | b) & static_cast<uint64>(0x33'33'33'33'33'33'33'33ull);

      a = ((a >> 2u) | a) & static_cast<uint64>(0x0F'0F'0F'0F'0F'0F'0F'0Full);
      b = ((b >> 2u) | b) & static_cast<uint64>(0x0F'0F'0F'0F'0F'0F'0F'0Full);

      a = ((a >> 4u) | a) & static_cast<uint64>(0x00'FF'00'FF'00'FF'00'FFull);
      b = ((b >> 4u) | b) & static_cast<uint64>(0x00'FF'00'FF'00'FF'00'FFull);

      a = ((a >> 8u) | a) & static_cast<uint64>(0x00'00'FF'FF'00'00'FF'FFull);
      b = ((b >> 8u) | b) & static_cast<uint64>(0x00'00'FF'FF'00'00'FF'FFull);

      a = ((a >> 16u) | a) & static_cast<uint64>(0x00'00'00'00'FF'FF'FF'FFull);
      b = ((b >> 16u) | b) & static_cast<uint64>(0x00'00'00'00'FF'FF'FF'FFull);

      return Vector2<uint32>((uint32)a, (uint32)b);
    }

#pragma endregion

#pragma region int32

    NO_DISCARD static constexpr int64 Encode(int32 x, int32 y) noexcept
    {
      uint32 a = std::bit_cast<uint32>(x);
      uint32 b = std::bit_cast<uint32>(y);

      uint64 result = Encode(a, b);
      return std::bit_cast<int64>(result);
    }

    NO_DISCARD static constexpr int64 Encode(int32 x, int32 y, int32 z) noexcept
    {
      uint32 a = std::bit_cast<uint32>(x);
      uint32 b = std::bit_cast<uint32>(y);
      uint32 c = std::bit_cast<uint32>(z);

      uint64 result = Encode(a, b, c);
      return std::bit_cast<int64>(result);
    }

    NO_DISCARD static constexpr int64 Encode(const Vector2<int32> &v) noexcept
    {
      return Encode(v.x, v.y);
    }

    NO_DISCARD static constexpr int64 Encode(const Vector3<int32> &v) noexcept
    {
      return Encode(v.x, v.y, v.z);
    }

#pragma endregion
  };
}
