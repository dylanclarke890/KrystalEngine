#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Endian.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <cassert>
#include <cstring>

namespace Krys
{
  struct ByteUtils : NonCopyMovable<ByteUtils>
  {
    ByteUtils() = delete;
    ~ByteUtils() = delete;

    /// @brief Enumerates chunks of bytes from the given byte span, invoking the provided callable for each
    /// chunk.
    template <size_t N, typename TFunc>
    requires Callable<TFunc, FixedSpan<const byte, N>>
    constexpr static void ForEachNBytes(Span<const byte> bytes, TFunc &&func) noexcept
    {
      size_t totalBytes = bytes.size();
      size_t offset = 0uz;
      while (offset < totalBytes)
      {
        size_t chunkSize = std::min(N, totalBytes - offset);
        FixedSpan<const byte, N> chunk {bytes.data() + offset, N};
        func(chunk);
        offset += chunkSize;
      }
    }

    template <Endian::Type Src, Endian::Type Dst, Number T>
    KRYS_NODISCARD static constexpr T AsNumeric(const byte *bytes) noexcept
    {
      T value {};
      std::memcpy(&value, bytes, sizeof(T));
      return Endian::Convert<Src, Dst, T>(value);
    }

    template <Endian::Type Src, Endian::Type Dst, Number T>
    KRYS_NODISCARD static constexpr T AsNumeric(const List<byte> &bytes) noexcept
    {
      return AsNumeric<Src, Dst, T>(bytes.data());
    }

    template <Endian::Type Src, Endian::Type Dst, Number T>
    KRYS_NODISCARD static constexpr T AsNumeric(const byte *bytes, size_t offset) noexcept
    {
      assert(offset + sizeof(T) <= sizeof(bytes));
      return AsNumeric<Src, Dst, T>(bytes + offset);
    }

    template <Endian::Type Src, Endian::Type Dst, Number T>
    KRYS_NODISCARD static constexpr List<T> AsNumericArray(const List<byte> &bytes) noexcept
    {
      assert(bytes.size() % sizeof(T) == 0);

      size_t elementCount = bytes.size() / sizeof(T);
      List<T> elements(elementCount);

      for (size_t i = 0; i < elementCount; i++)
      {
        elements[i] = AsNumeric<Src, Dst, T>(&bytes[i * sizeof(T)]);
      }

      return elements;
    }

    template <Endian::Type Src, Endian::Type Dst, Number T>
    KRYS_NODISCARD static constexpr Array<byte, sizeof(T)> ToBytes(T value) noexcept
    {
      Array<byte, sizeof(T)> bytes;

      value = Endian::Convert<Src, Dst, T>(value);
      std::memcpy(bytes.data(), &value, sizeof(T));

      return bytes;
    }

    template <Endian::Type Src, Endian::Type Dst, Number T>
    KRYS_NODISCARD static constexpr void ToBytes(T value, List<byte> &out) noexcept
    {
      assert(out.size() % sizeof(T) == 0);
      if (out.capacity() < out.size() + sizeof(T))
      {
        out.reserve(out.size() + sizeof(T));
      }

      value = Endian::Convert<Src, Dst, T>(value);
      const byte *bytePtr = reinterpret_cast<const byte *>(&value);
      for (size_t i = 0; i < sizeof(T); i++)
      {
        out.push_back(bytePtr[i]);
      }
    }

    KRYS_NODISCARD static string AsString(const List<byte> &bytes, const size_t length) noexcept
    {
      return string(reinterpret_cast<const char *>(bytes.data()), length);
    }

    KRYS_NODISCARD static Span<const byte> AsBytesView(const string &str) noexcept
    {
      return Span<const byte>(reinterpret_cast<const byte *>(str.data()), str.size());
    }

    template <typename T>
    KRYS_NODISCARD static Span<const byte> AsBytesView(const List<T> &list) noexcept
    {
      return Span<const byte>(reinterpret_cast<const byte *>(list.data()), list.size() * sizeof(T));
    }

    template <typename T>
    KRYS_NODISCARD static Span<const byte> AsBytesView(const Span<T> &span) noexcept
    {
      return Span<const byte>(reinterpret_cast<const byte *>(span.data()), span.size() * sizeof(T));
    }

    template <typename T>
    KRYS_NODISCARD static Span<const byte> AsBytesView(const T &object) noexcept
    {
      return Span<const byte>(reinterpret_cast<const byte *>(&object), sizeof(T));
    }

    KRYS_NODISCARD static constexpr size_t AlignNext(size_t size, size_t alignment) noexcept
    {
      assert(alignment != 0 && (alignment & (alignment - 1)) == 0); // alignment must be a power of two
      return (size + alignment - 1) & ~(alignment - 1);
    }

    KRYS_NODISCARD static constexpr size_t AlignPrev(size_t size, size_t alignment) noexcept
    {
      assert(alignment != 0 && (alignment & (alignment - 1)) == 0); // alignment must be a power of two
      return size & ~(alignment - 1);
    }
  };

  KRYS_NODISCARD constexpr size_t operator""_KB(unsigned long long int value) noexcept
  {
    return static_cast<size_t>(value * 1'024u);
  }

  KRYS_NODISCARD constexpr size_t operator""_MB(unsigned long long int value) noexcept
  {
    return static_cast<size_t>(value * 1'024u * 1'024u);
  }

  KRYS_NODISCARD constexpr size_t operator""_GB(unsigned long long int value) noexcept
  {
    return static_cast<size_t>(value * 1'024u * 1'024u * 1'024u);
  }
}
