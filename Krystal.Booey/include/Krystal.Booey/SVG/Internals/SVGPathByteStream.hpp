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
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::svg
{
  /// @brief An SVG path in a compact binary form: each segment is its command byte followed by its numbers
  /// and flags. The bytes are only meant to be read back by `SVGPathByteStreamSource`.
  class SVGPathByteStream final
  {
  public:
    using Bytes = List<uint8>;
    using DataIterator = Bytes::const_iterator;

  private:
    // FIXME: Upstream shares the bytes between copies and caches the platform Path built from them; add both
    // when Path is ported.
    Bytes _bytes;

  public:
    SVGPathByteStream() noexcept = default;

    /// @brief Parses `string` as SVG path data; on a parse error the stream holds the segments before it.
    explicit SVGPathByteStream(dom::DOMStringView string) noexcept;

    explicit SVGPathByteStream(Bytes &&data) noexcept : _bytes(krys::move(data))
    {
    }

    KRYS_NODISCARD static const SVGPathByteStream &Empty() noexcept;

    KRYS_NODISCARD friend bool operator==(const SVGPathByteStream &,
                                          const SVGPathByteStream &) noexcept = default;

    KRYS_NODISCARD UniquePtr<SVGPathByteStream> Copy() const noexcept
    {
      return CreateUnique<SVGPathByteStream>(*this);
    }

    KRYS_NODISCARD DataIterator begin() const noexcept
    {
      return _bytes.begin();
    }

    KRYS_NODISCARD DataIterator end() const noexcept
    {
      return _bytes.end();
    }

    void Append(uint8 byte) noexcept
    {
      _bytes.push_back(byte);
    }

    void Append(Span<const uint8> bytes) noexcept
    {
      _bytes.insert(_bytes.end(), bytes.begin(), bytes.end());
    }

    void Append(const SVGPathByteStream &other) noexcept
    {
      Append(Span<const uint8>(other._bytes));
    }

    void Clear() noexcept
    {
      _bytes.clear();
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _bytes.empty();
    }

    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return static_cast<uint32>(_bytes.size());
    }

    // Making empty for now instead of sharing Vectors.
    void ShrinkToFit() noexcept
    {
    }

    KRYS_NODISCARD const Bytes &GetBytes() const noexcept
    {
      return _bytes;
    }
  };
}
