/*
 * Copyright (C) 2012, 2013 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPath.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  /// @brief A path that can be built once and filled, stroked or clipped to by 2D contexts.
  /// @see https://html.spec.whatwg.org/#path2d
  class Path2D final : public RefCounted<Path2D>, public CanvasPath
  {
  public:
    KRYS_NODISCARD static Ref<Path2D> Create() noexcept
    {
      return AdoptRef(*new Path2D);
    }
    KRYS_NODISCARD static Ref<Path2D> Create(const Path &path) noexcept
    {
      return AdoptRef(*new Path2D(path));
    }
    KRYS_NODISCARD static Ref<Path2D> Create(const Path2D &path) noexcept
    {
      return Create(path.GetPath());
    }
    /// @brief A path from SVG path data (`<path d>`'s syntax).
    KRYS_NODISCARD static Ref<Path2D> Create(dom::DOMStringView pathData) noexcept;

    ~Path2D() noexcept override = default;

    /// @see https://html.spec.whatwg.org/#dom-path2d-addpath
    dom::ExceptionOr<void> AddPath(const Path2D &path, css::DOMMatrix2DInit &&transform = {}) noexcept;

    KRYS_NODISCARD const Path &GetPath() const noexcept
    {
      return _path;
    }

  private:
    Path2D() noexcept = default;
    explicit Path2D(const Path &path) noexcept : CanvasPath(path)
    {
    }
  };
}
