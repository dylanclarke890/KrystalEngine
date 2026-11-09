/*
 * Copyright (C) 2022-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterGraph.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo
{
  /// @brief The graph of a filter's primitive elements, before they create their effects.
  class SVGFilterPrimitiveGraph final : public SVGFilterGraph<svg::SVGFilterPrimitiveStandardAttributes>
  {
  public:
    SVGFilterPrimitiveGraph() noexcept = default;

    void AddNamedNode(const dom::DOMStringAtom &name,
                      Ref<svg::SVGFilterPrimitiveStandardAttributes> &&node) noexcept override
    {
      if (name.View().empty())
      {
        _lastNode = krys::move(node);
        return;
      }

      _lastNode = krys::move(node);
      _namedNodes.insert_or_assign(name, Ref<svg::SVGFilterPrimitiveStandardAttributes>(_lastNode));
    }

  private:
    KRYS_NODISCARD RefPtr<svg::SVGFilterPrimitiveStandardAttributes>
      GetNamedNode(const dom::DOMStringAtom &name) const noexcept override
    {
      if (name.View().empty())
      {
        return _lastNode;
      }

      auto it = _namedNodes.find(name);
      return it != _namedNodes.end() ? RefPtr<svg::SVGFilterPrimitiveStandardAttributes>(it->second)
                                     : nullptr;
    }
  };
}
