/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
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

#include "Krystal.Booey/SVG/GradientAttributes.hpp"
#include "Krystal.Booey/SVG/SVGLengthValue.hpp"

namespace krys::boo::svg
{
  /// @brief A radial gradient's attributes, collected from it and the gradients it references.
  struct RadialGradientAttributes : GradientAttributes
  {
  private:
    // Properties
    SVGLengthValue _cx {SVGLengthMode::Width, u8"50%"};
    SVGLengthValue _cy {SVGLengthMode::Height, u8"50%"};
    SVGLengthValue _r {SVGLengthMode::Other, u8"50%"};
    SVGLengthValue _fx;
    SVGLengthValue _fy;
    SVGLengthValue _fr;

    // Property states
    bool _cxSet : 1 {false};
    bool _cySet : 1 {false};
    bool _rSet : 1 {false};
    bool _fxSet : 1 {false};
    bool _fySet : 1 {false};
    bool _frSet : 1 {false};

  public:
    KRYS_NODISCARD SVGLengthValue Cx() const noexcept
    {
      return _cx;
    }
    KRYS_NODISCARD SVGLengthValue Cy() const noexcept
    {
      return _cy;
    }
    KRYS_NODISCARD SVGLengthValue R() const noexcept
    {
      return _r;
    }
    KRYS_NODISCARD SVGLengthValue Fx() const noexcept
    {
      return _fx;
    }
    KRYS_NODISCARD SVGLengthValue Fy() const noexcept
    {
      return _fy;
    }
    KRYS_NODISCARD SVGLengthValue Fr() const noexcept
    {
      return _fr;
    }

    void SetCx(SVGLengthValue value) noexcept
    {
      _cx = value;
      _cxSet = true;
    }
    void SetCy(SVGLengthValue value) noexcept
    {
      _cy = value;
      _cySet = true;
    }
    void SetR(SVGLengthValue value) noexcept
    {
      _r = value;
      _rSet = true;
    }
    void SetFx(SVGLengthValue value) noexcept
    {
      _fx = value;
      _fxSet = true;
    }
    void SetFy(SVGLengthValue value) noexcept
    {
      _fy = value;
      _fySet = true;
    }
    void SetFr(SVGLengthValue value) noexcept
    {
      _fr = value;
      _frSet = true;
    }

    KRYS_NODISCARD bool HasCx() const noexcept
    {
      return _cxSet;
    }
    KRYS_NODISCARD bool HasCy() const noexcept
    {
      return _cySet;
    }
    KRYS_NODISCARD bool HasR() const noexcept
    {
      return _rSet;
    }
    KRYS_NODISCARD bool HasFx() const noexcept
    {
      return _fxSet;
    }
    KRYS_NODISCARD bool HasFy() const noexcept
    {
      return _fySet;
    }
    KRYS_NODISCARD bool HasFr() const noexcept
    {
      return _frSet;
    }
  };
}
