/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/DataRef.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <utility>

namespace krys::boo::svg
{
  /// @brief An SVG path in a compact binary form: each segment is its command byte followed by its numbers
  /// and flags. The bytes are only meant to be read back by `SVGPathByteStreamSource`.
  /// @note Copies share their bytes until one is changed, and the path built from them is cached with them.
  class SVGPathByteStream final
  {
  public:
    using Bytes = List<uint8>;
    using DataIterator = Bytes::const_iterator;

    /// @brief The bytes of one or more streams, and the path built from them.
    class Data final : public RefCounted<Data>
    {
    private:
      Bytes _bytes;
      mutable Maybe<Path> _path;

    public:
      KRYS_NODISCARD static Ref<Data> Create(Bytes &&bytes) noexcept
      {
        return AdoptRef(*new Data(krys::move(bytes)));
      }

      KRYS_NODISCARD static Ref<Data> Empty() noexcept;

      KRYS_NODISCARD Ref<Data> Copy() const noexcept
      {
        return AdoptRef(*new Data(*this));
      }

      KRYS_NODISCARD friend bool operator==(const Data &a, const Data &b) noexcept
      {
        return a._bytes == b._bytes;
      }

      KRYS_NODISCARD const Bytes &GetBytes() const noexcept
      {
        return _bytes;
      }

      KRYS_NODISCARD const Maybe<Path> &CachedPath() const noexcept
      {
        return _path;
      }

      void CachePath(const Path &path) const noexcept
      {
        _path = path;
      }

      void Append(Span<const uint8> bytes) noexcept
      {
        _path.reset();
        _bytes.insert(_bytes.end(), bytes.begin(), bytes.end());
      }

    private:
      Data() noexcept = default;

      explicit Data(Bytes &&bytes) noexcept : _bytes(krys::move(bytes))
      {
      }

      Data(const Data &other) noexcept : _bytes(other._bytes), _path(other._path)
      {
      }
    };

  private:
    DataRef<Data> _data {Data::Empty()};

  public:
    SVGPathByteStream() noexcept = default;

    /// @brief Parses `string` as SVG path data; on a parse error the stream holds the segments before it.
    explicit SVGPathByteStream(dom::DOMStringView string) noexcept;

    explicit SVGPathByteStream(Bytes &&data) noexcept : _data(Data::Create(krys::move(data)))
    {
    }

    SVGPathByteStream(const SVGPathByteStream &) noexcept = default;
    SVGPathByteStream &operator=(const SVGPathByteStream &) noexcept = default;

    /// @note A moved-from stream is empty.
    SVGPathByteStream(SVGPathByteStream &&other) noexcept
        : _data(std::exchange(other._data, DataRef<Data>(Data::Empty())))
    {
    }

    SVGPathByteStream &operator=(SVGPathByteStream &&other) noexcept
    {
      if (this != &other)
      {
        _data = std::exchange(other._data, DataRef<Data>(Data::Empty()));
      }
      return *this;
    }

    KRYS_NODISCARD static const SVGPathByteStream &Empty() noexcept;

    KRYS_NODISCARD friend bool operator==(const SVGPathByteStream &a, const SVGPathByteStream &b) noexcept
    {
      return a._data == b._data;
    }

    KRYS_NODISCARD UniquePtr<SVGPathByteStream> Copy() const noexcept
    {
      return CreateUnique<SVGPathByteStream>(*this);
    }

    KRYS_NODISCARD DataIterator begin() const noexcept
    {
      return _data->GetBytes().begin();
    }

    KRYS_NODISCARD DataIterator end() const noexcept
    {
      return _data->GetBytes().end();
    }

    void Append(uint8 byte) noexcept
    {
      Append(Span<const uint8>(&byte, 1));
    }

    void Append(Span<const uint8> bytes) noexcept
    {
      _data.Access().Append(bytes);
    }

    void Append(const SVGPathByteStream &other) noexcept
    {
      // Copy first: `other` may be this stream.
      Bytes bytes = other._data->GetBytes();
      Append(Span<const uint8>(bytes));
    }

    void Clear() noexcept
    {
      _data = Data::Empty();
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _data->GetBytes().empty();
    }

    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return static_cast<uint32>(_data->GetBytes().size());
    }

    // Making empty for now instead of sharing Vectors.
    void ShrinkToFit() noexcept
    {
    }

    KRYS_NODISCARD const Bytes &GetBytes() const noexcept
    {
      return _data->GetBytes();
    }

    /// @brief The path built from the bytes, if one has been built since they last changed.
    KRYS_NODISCARD const Maybe<Path> &CachedPath() const noexcept
    {
      return _data->CachedPath();
    }

    /// @brief Keeps @p path, built from the bytes, with them (and so with every stream that shares them).
    void CachePath(const Path &path) const noexcept
    {
      _data->CachePath(path);
    }
  };
}
