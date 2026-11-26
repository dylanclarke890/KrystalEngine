/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Web/Graphics/Paths/Path.hpp"
#include "Krystal.Web/Style/Values/Types.hpp"

namespace krys::web::style
{
  // All types that want to expose a generated web::Path must specialize PathComputation the following member
  // function:
  //
  //    template<> struct krys::web::style::PathComputation<StyleType> {
  //        web::Path operator()(const StyleType&, const RectF&);
  //    };

  template <typename StyleType>
  struct PathComputation;

  struct PathComputationInvoker
  {
    template <typename StyleType, typename Reference>
    KRYS_NODISCARD web::Path operator()(const StyleType &value, const Reference &reference) const noexcept
    {
      return PathComputation<StyleType> {}(value, reference);
    }
  };

  constexpr inline PathComputationInvoker ComputePath {};

  // Specialization for `css::FunctionNotation`.
  template <css::ValueId Name, typename StyleType>
  struct PathComputation<css::FunctionNotation<Name, StyleType>>
  {
    template <typename Reference>
    KRYS_NODISCARD web::Path operator()(const css::FunctionNotation<Name, StyleType> &value,
                                        const Reference &reference) noexcept
    {
      return krys::web::style::ComputePath(value.Parameters, reference);
    }
  };
}
