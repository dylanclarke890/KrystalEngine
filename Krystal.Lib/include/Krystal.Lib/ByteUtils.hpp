#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Endian.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include <cassert>
#include <cstring>

namespace Krys
{
  struct ByteUtils
  {
    STATIC_CLASS(ByteUtils)

    template <Arithmetic T, Endian::Type Src = Endian::Type::System, Endian::Type Dst = Endian::Type::System>
    NO_DISCARD static constexpr T AsNumeric(const byte *bytes) noexcept
    {
      T value {};
      std::memcpy(&value, bytes, sizeof(T));
      return Endian::Convert<T, Src, Dst>(value);
    }

    template <Arithmetic T, Endian::Type Src = Endian::Type::System, Endian::Type Dst = Endian::Type::System>
    NO_DISCARD static constexpr T AsNumeric(const List<byte> &bytes) noexcept
    {
      return AsNumeric<T, Src, Dst>(bytes.data());
    }

    template <Arithmetic T, Endian::Type Src = Endian::Type::System, Endian::Type Dst = Endian::Type::System>
    NO_DISCARD static constexpr T AsNumeric(const byte *bytes, size_t offset) noexcept
    {
      assert(offset + sizeof(T) <= sizeof(bytes));
      return AsNumeric<T, Src, Dst>(bytes + offset);
    }

    template <Arithmetic T, Endian::Type Src = Endian::Type::System, Endian::Type Dst = Endian::Type::System>
    NO_DISCARD static constexpr List<T> AsNumericArray(const List<byte> &bytes) noexcept
    {
      assert(bytes.size() % sizeof(T) == 0);
      size_t elementCount = bytes.size() / sizeof(T);

      List<T> elements;
      elements.resize(elementCount);

      for (size_t i = 0; i < elementCount; i++)
        elements[i] = AsNumeric<T, Src, Dst>(&bytes[i * sizeof(T)]);

      return elements;
    }

    template <Arithmetic T, Endian::Type Src = Endian::System, Endian::Type Dst = Endian::System>
    NO_DISCARD static constexpr List<byte> From(T value) noexcept
    {
      value = Endian::Convert<T, Src, Dst>(value);

      List<byte> bytes(sizeof(T));
      std::memcpy(bytes.data(), &value, sizeof(T));

      return bytes;
    }

    NO_DISCARD static inline string AsString(const List<byte> &bytes, const size_t length) noexcept
    {
      return string(reinterpret_cast<const char *>(bytes.data()), length);
    }

    NO_DISCARD static inline Span<const byte> AsBytesView(const string &str) noexcept
    {
      return Span<const byte>(reinterpret_cast<const byte *>(str.data()), str.size());
    }

    template <typename T>
    NO_DISCARD static inline Span<const byte> AsBytesView(const List<T> &list) noexcept
    {
      return Span<const byte>(reinterpret_cast<const byte *>(list.data()), list.size() * sizeof(T));
    }

    template <typename T>
    NO_DISCARD static inline Span<const byte> AsBytesView(const Span<T> &span) noexcept
    {
      return Span<const byte>(reinterpret_cast<const byte *>(span.data()), span.size() * sizeof(T));
    }

    template <typename T>
    NO_DISCARD static inline Span<const byte> AsBytesView(const T &object) noexcept
    {
      return Span<const byte>(reinterpret_cast<const byte *>(&object), sizeof(T));
    }

    NO_DISCARD static constexpr size_t AlignNext(size_t size, size_t alignment) noexcept
    {
      assert(alignment != 0 && (alignment & (alignment - 1)) == 0); // alignment must be a power of two
      return (size + alignment - 1) & ~(alignment - 1);
    }

    NO_DISCARD static constexpr size_t AlignPrev(size_t size, size_t alignment) noexcept
    {
      assert(alignment != 0 && (alignment & (alignment - 1)) == 0); // alignment must be a power of two
      return size & ~(alignment - 1);
    }
  };

  NO_DISCARD constexpr size_t operator"" _KB(unsigned long long int value) noexcept
  {
    return static_cast<size_t>(value * 1'024u);
  }

  NO_DISCARD constexpr size_t operator"" _MB(unsigned long long int value) noexcept
  {
    return static_cast<size_t>(value * 1'024u * 1'024u);
  }

  NO_DISCARD constexpr size_t operator"" _GB(unsigned long long int value) noexcept
  {
    return static_cast<size_t>(value * 1'024u * 1'024u * 1'024u);
  }
}
