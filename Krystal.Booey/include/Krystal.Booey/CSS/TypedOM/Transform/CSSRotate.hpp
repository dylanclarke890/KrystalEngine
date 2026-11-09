/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTransformComponent.hpp"

namespace krys::boo::css
{
  /// @brief A `rotate()`, `rotate3d()` or single-axis rotation.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssrotate
  class CSSRotate final : public CSSTransformComponent
  {
  private:
    Ref<CSSNumericValue> _x;
    Ref<CSSNumericValue> _y;
    Ref<CSSNumericValue> _z;
    Ref<CSSNumericValue> _angle;

  public:
    /// @brief A 3D rotation by @p angle about the vector (@p x, @p y, @p z).
    /// @return A `TypeError` if @p angle isn't an angle, or @p x, @p y or @p z isn't a number.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssrotate-cssrotate-x-y-z-angle
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSRotate>>
      Create(CSSNumberish x, CSSNumberish y, CSSNumberish z, Ref<CSSNumericValue> angle) noexcept;
    /// @brief A 2D rotation by @p angle.
    /// @return A `TypeError` if @p angle isn't an angle.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-cssrotate-cssrotate-angle
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSRotate>> Create(Ref<CSSNumericValue> angle) noexcept;
    /// @brief Reifies a rotation function.
    KRYS_NODISCARD static dom::ExceptionOr<Ref<CSSRotate>> Create(const FunctionValue &function,
                                                                  dom::Document &document) noexcept;

    KRYS_NODISCARD const CSSNumericValue &X() const noexcept
    {
      return *_x;
    }
    KRYS_NODISCARD const CSSNumericValue &Y() const noexcept
    {
      return *_y;
    }
    KRYS_NODISCARD const CSSNumericValue &Z() const noexcept
    {
      return *_z;
    }
    KRYS_NODISCARD const CSSNumericValue &Angle() const noexcept
    {
      return *_angle;
    }
    /// @return A `TypeError` if @p x isn't a number.
    dom::ExceptionOr<void> SetX(CSSNumberish x) noexcept;
    /// @return A `TypeError` if @p y isn't a number.
    dom::ExceptionOr<void> SetY(CSSNumberish y) noexcept;
    /// @return A `TypeError` if @p z isn't a number.
    dom::ExceptionOr<void> SetZ(CSSNumberish z) noexcept;
    /// @return A `TypeError` if @p angle isn't an angle.
    dom::ExceptionOr<void> SetAngle(Ref<CSSNumericValue> angle) noexcept;

    void Serialise(StringBuilder &builder) const noexcept final;
    KRYS_NODISCARD dom::ExceptionOr<Ref<DOMMatrix>> ToMatrix() const noexcept final;

    KRYS_NODISCARD CSSTransformType TransformType() const noexcept final
    {
      return CSSTransformType::Rotate;
    }
    KRYS_NODISCARD RefPtr<Value> ToCSSValue() const noexcept final;

  private:
    CSSRotate(bool is2D, Ref<CSSNumericValue> x, Ref<CSSNumericValue> y, Ref<CSSNumericValue> z,
              Ref<CSSNumericValue> angle) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSRotate)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSTransformComponent &component) noexcept
  {
    return component.TransformType() == krys::boo::css::CSSTransformType::Rotate;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
