#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Maths/Clamp.hpp"
#include "Krystal.Maths/Round.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Maths
{
#pragma region Pack2x8To16

  template <ConvertibleTo<uint8> T>
  NO_DISCARD constexpr uint16 Pack2x8To16(T a, T b) noexcept
  requires UnsignedIntegral<T>
  {
    return (static_cast<uint16>(a) << 8) | static_cast<uint16>(b);
  }

  template <ConvertibleTo<uint8> T>
  NO_DISCARD constexpr uint16 Pack2x8To16(const Vector2<T> &v) noexcept
  requires UnsignedIntegral<T>
  {
    return Pack2x8To16(v.x, v.y);
  }

  template <ConvertibleTo<uint16> T>
  NO_DISCARD constexpr Vector2<uint8> Unpack16To2x8(T value) noexcept
  requires UnsignedIntegral<T>
  {
    uint8 a = static_cast<uint8>((value >> 8) & 0xFF);
    uint8 b = static_cast<uint8>(value & 0xFF);
    return {a, b};
  }

  template <ConvertibleTo<int8> T>
  NO_DISCARD constexpr int16 Pack2x8To16(T a, T b) noexcept
  requires SignedIntegral<T>
  {
    return (static_cast<int16>(a) << 8) | static_cast<int16>(b);
  }

  template <ConvertibleTo<int8> T>
  NO_DISCARD constexpr int16 Pack2x8To16(const Vector2<T> &v) noexcept
  requires SignedIntegral<T>
  {
    return Pack2x8To16(v.x, v.y);
  }

  template <ConvertibleTo<int16> T>
  NO_DISCARD constexpr Vector2<int8> Unpack16To2x8(T value) noexcept
  requires SignedIntegral<T>
  {
    int8 a = static_cast<int8>((value >> 8) & 0xFF);
    int8 b = static_cast<int8>(value & 0xFF);
    return {a, b};
  }

#pragma endregion

#pragma region Pack4x8To32

  template <ConvertibleTo<uint8> T>
  NO_DISCARD constexpr uint32 Pack4x8To32(T a, T b, T c, T d) noexcept
  requires UnsignedIntegral<T>
  {
    return (static_cast<uint32>(a) << 24) | (static_cast<uint32>(b) << 16) | (static_cast<uint32>(c) << 8)
           | static_cast<uint32>(d);
  }

  template <ConvertibleTo<uint8> T>
  NO_DISCARD constexpr uint32 Pack4x8To32(const Vector4<T> &v) noexcept
  requires UnsignedIntegral<T>
  {
    return Pack4x8To32(v.x, v.y, v.z, v.w);
  }

  template <ConvertibleTo<uint32> T>
  NO_DISCARD constexpr Vector4<uint8> Unpack32To4x8(T value) noexcept
  requires UnsignedIntegral<T>
  {
    uint8 a = static_cast<uint8>((value >> 24) & 0xFF);
    uint8 b = static_cast<uint8>((value >> 16) & 0xFF);
    uint8 c = static_cast<uint8>((value >> 8) & 0xFF);
    uint8 d = static_cast<uint8>(value & 0xFF);
    return {a, b, c, d};
  }

  template <ConvertibleTo<int8> T>
  NO_DISCARD constexpr int32 Pack4x8To32(T a, T b, T c, T d) noexcept
  requires SignedIntegral<T>
  {
    return (static_cast<int32>(a) << 24) | (static_cast<int32>(b) << 16) | (static_cast<int32>(c) << 8)
           | static_cast<int32>(d);
  }

  template <ConvertibleTo<int8> T>
  NO_DISCARD constexpr int32 Pack4x8To32(const Vector4<T> &v) noexcept
  requires SignedIntegral<T>
  {
    return Pack4x8To32(v.x, v.y, v.z, v.w);
  }

  template <ConvertibleTo<int32> T>
  NO_DISCARD constexpr Vector4<int8> Unpack32To4x8(T value) noexcept
  requires SignedIntegral<T>
  {
    int8 a = static_cast<int8>((value >> 24) & 0xFF);
    int8 b = static_cast<int8>((value >> 16) & 0xFF);
    int8 c = static_cast<int8>((value >> 8) & 0xFF);
    int8 d = static_cast<int8>(value & 0xFF);
    return {a, b, c, d};
  }

#pragma endregion

#pragma region Pack2x16To32

  template <ConvertibleTo<uint16> T>
  NO_DISCARD constexpr uint32 Pack2x16To32(T a, T b) noexcept
  requires UnsignedIntegral<T>
  {
    return (static_cast<uint32>(a) << 16) | static_cast<uint32>(b);
  }

  template <ConvertibleTo<uint16> T>
  NO_DISCARD constexpr uint32 Pack2x16To32(const Vector2<T> &v) noexcept
  requires UnsignedIntegral<T>
  {
    return Pack2x16To32(v.x, v.y);
  }

  template <ConvertibleTo<uint32> T>
  NO_DISCARD constexpr Vector2<uint16> Unpack32To2x16(T value) noexcept
  requires UnsignedIntegral<T>
  {
    uint16 a = static_cast<uint16>((value >> 16) & 0xFFFF);
    uint16 b = static_cast<uint16>(value & 0xFFFF);
    return {a, b};
  }

  template <ConvertibleTo<int16> T>
  NO_DISCARD constexpr int32 Pack2x16To32(T a, T b) noexcept
  requires SignedIntegral<T>
  {
    return (static_cast<int32>(a) << 16) | static_cast<int32>(b);
  }

  template <ConvertibleTo<int16> T>
  NO_DISCARD constexpr int32 Pack2x16To32(const Vector2<T> &v) noexcept
  requires SignedIntegral<T>
  {
    return Pack2x16To32(v.x, v.y);
  }

  template <ConvertibleTo<int32> T>
  NO_DISCARD constexpr Vector2<int16> Unpack32To2x16(T value) noexcept
  requires SignedIntegral<T>
  {
    int16 a = static_cast<int16>((value >> 16) & 0xFFFF);
    int16 b = static_cast<int16>(value & 0xFFFF);
    return {a, b};
  }

#pragma endregion

#pragma region Pack4x16To64

  template <ConvertibleTo<uint16> T>
  NO_DISCARD constexpr uint64 Pack4x16To64(T a, T b, T c, T d) noexcept
  requires UnsignedIntegral<T>
  {
    return (static_cast<uint64>(a) << 48) | (static_cast<uint64>(b) << 32) | (static_cast<uint64>(c) << 16)
           | static_cast<uint64>(d);
  }

  template <ConvertibleTo<uint16> T>
  NO_DISCARD constexpr uint64 Pack4x16To64(const Vector4<T> &v) noexcept
  requires UnsignedIntegral<T>
  {
    return Pack4x16To64(v.x, v.y, v.z, v.w);
  }

  template <ConvertibleTo<uint64> T>
  NO_DISCARD constexpr Vector4<uint16> Unpack64To4x16(T value) noexcept
  requires UnsignedIntegral<T>
  {
    uint16 a = static_cast<uint16>((value >> 48) & 0xFF'FF'FF'FF);
    uint16 b = static_cast<uint16>((value >> 32) & 0xFF'FF'FF'FF);
    uint16 c = static_cast<uint16>((value >> 16) & 0xFF'FF'FF'FF);
    uint16 d = static_cast<uint16>(value & 0xFF);
    return {a, b, c, d};
  }

  template <ConvertibleTo<int16> T>
  NO_DISCARD constexpr int64 Pack4x16To64(T a, T b, T c, T d) noexcept
  requires SignedIntegral<T>
  {
    return (static_cast<int64>(a) << 48) | (static_cast<int64>(b) << 32) | (static_cast<int64>(c) << 16)
           | static_cast<int64>(d);
  }

  template <ConvertibleTo<int16> T>
  NO_DISCARD constexpr int64 Pack4x16To64(const Vector4<T> &v) noexcept
  requires SignedIntegral<T>
  {
    return Pack4x16To64(v.x, v.y, v.z, v.w);
  }

  template <ConvertibleTo<int64> T>
  NO_DISCARD constexpr Vector4<int16> Unpack64To4x16(T value) noexcept
  requires SignedIntegral<T>
  {
    int16 a = static_cast<int16>((value >> 48) & 0xFF'FF'FF'FF);
    int16 b = static_cast<int16>((value >> 32) & 0xFF'FF'FF'FF);
    int16 c = static_cast<int16>((value >> 16) & 0xFF'FF'FF'FF);
    int16 d = static_cast<int16>(value & 0xFF);
    return {a, b, c, d};
  }

#pragma endregion

#pragma region Pack2x32To64

  template <ConvertibleTo<uint32> T>
  NO_DISCARD constexpr uint64 Pack2x32To64(T a, T b) noexcept
  requires UnsignedIntegral<T>
  {
    return (static_cast<uint64>(a) << 32) | static_cast<uint64>(b);
  }

  template <ConvertibleTo<uint32> T>
  NO_DISCARD constexpr uint64 Pack2x32To64(const Vector2<T> &v) noexcept
  requires UnsignedIntegral<T>
  {
    return Pack2x32To64(v.x, v.y);
  }

  template <ConvertibleTo<uint64> T>
  NO_DISCARD constexpr Vector2<uint32> Unpack64To2x32(T value) noexcept
  requires UnsignedIntegral<T>
  {
    uint32 a = static_cast<uint32>((value >> 32) & 0xFF'FF'FF'FF);
    uint32 b = static_cast<uint32>(value & 0xFF'FF'FF'FF);
    return {a, b};
  }

  template <ConvertibleTo<int32> T>
  NO_DISCARD constexpr int64 Pack2x32To64(T a, T b) noexcept
  requires SignedIntegral<T>
  {
    return (static_cast<int64>(a) << 32) | static_cast<int64>(b);
  }

  template <ConvertibleTo<int32> T>
  NO_DISCARD constexpr int64 Pack2x32To64(const Vector2<T> &v) noexcept
  requires SignedIntegral<T>
  {
    return Pack2x32To64(v.x, v.y);
  }

  template <ConvertibleTo<int64> T>
  NO_DISCARD constexpr Vector2<int32> Unpack64To2x32(T value) noexcept
  requires SignedIntegral<T>
  {
    int32 a = static_cast<int32>((value >> 32) & 0xFF'FF'FF'FF);
    int32 b = static_cast<int32>(value & 0xFF'FF'FF'FF);
    return {a, b};
  }

#pragma endregion

#pragma region PackNormFloatTo8

  NO_DISCARD constexpr int8 PackNormFloatToInt8(float value) noexcept
  {
    return static_cast<int8>(Round(Clamp(value, -1.0f, 1.0f) * 127.0f));
  }

  NO_DISCARD constexpr float UnpackInt8ToNormFloat(int8 value) noexcept
  {
    return static_cast<float>(value) / 127.0f;
  }

  NO_DISCARD constexpr uint8 PackNormFloatToUint8(float value) noexcept
  {
    return static_cast<uint8>(Round(Clamp(value, 0.0f, 1.0f) * 255.0f));
  }

  NO_DISCARD constexpr float UnpackUint8ToNormFloat(uint8 value) noexcept
  {
    return static_cast<float>(value) / 255.0f;
  }

#pragma endregion

#pragma region PackNormFloatTo16

  NO_DISCARD constexpr int16 PackNormFloatToInt16(float value) noexcept
  {
    return static_cast<int16>(Round(Clamp(value, -1.0f, 1.0f) * 32767.0f));
  }

  NO_DISCARD constexpr float UnpackInt16ToNormFloat(int16 value) noexcept
  {
    return static_cast<float>(value) / 32767.0f;
  }

  NO_DISCARD constexpr uint16 PackNormFloatToUint16(float value) noexcept
  {
    return static_cast<uint16>(Round(Clamp(value, 0.0f, 1.0f) * 65535.0f));
  }

  NO_DISCARD constexpr float UnpackUint16ToNormFloat(uint16 value) noexcept
  {
    return static_cast<float>(value) / 65535.0f;
  }

#pragma endregion
}
