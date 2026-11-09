/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  /// @brief The path data a `<path>` element's `d` attribute reflects, as a byte stream, which an animation
  /// of `d` interpolates.
  /// @note Holds no segment items: SVG 2 removed the segment list interface.
  class SVGPathSegList final : public SVGProperty
  {
    friend class SVGAnimatedPathSegListAnimator;

  private:
    SVGPathByteStream _pathByteStream;
    mutable Maybe<Path> _path;

  public:
    KRYS_NODISCARD static Ref<SVGPathSegList> Create(SVGPropertyOwner *owner,
                                                     SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGPathSegList(owner, access));
    }

    KRYS_NODISCARD static Ref<SVGPathSegList> Create(const SVGPathSegList &other,
                                                     SVGPropertyAccess access) noexcept
    {
      return AdoptRef(*new SVGPathSegList(other, access));
    }

    SVGPathSegList &operator=(const SVGPathSegList &other) noexcept
    {
      PathByteStreamWillChange();
      _pathByteStream = other.PathByteStream();
      return *this;
    }

    void ClearByteStreamData() noexcept
    {
      PathByteStreamWillChange();
      _pathByteStream.Clear();
    }

    KRYS_NODISCARD const SVGPathByteStream &PathByteStream() const noexcept
    {
      return _pathByteStream;
    }

    KRYS_NODISCARD SVGPathByteStream &PathByteStream() noexcept
    {
      return _pathByteStream;
    }

    bool Parse(dom::DOMStringView value) noexcept
    {
      PathByteStreamWillChange();
      return BuildSVGPathByteStreamFromString(value, _pathByteStream, PathParsingMode::UnalteredParsing);
    }

    KRYS_NODISCARD Path GetPath() const noexcept
    {
      if (!_path)
      {
        _path = BuildPathFromByteStream(PathByteStream());
      }
      return *_path;
    }

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      dom::DOMString value;
      (void)BuildStringFromByteStream(PathByteStream(), value, PathParsingMode::UnalteredParsing);
      return value;
    }

  private:
    SVGPathSegList(SVGPropertyOwner *owner, SVGPropertyAccess access) noexcept : SVGProperty(owner, access)
    {
    }

    SVGPathSegList(const SVGPathSegList &other, SVGPropertyAccess access) noexcept
        : SVGProperty(other._owner, access), _pathByteStream(other.PathByteStream())
    {
    }

    void PathByteStreamWillChange() noexcept
    {
      _path = {};
    }
  };
}
