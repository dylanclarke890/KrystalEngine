/*
 * Copyright (C) 2011, 2012 Google Inc. All rights reserved.
 * Copyright (C) 2014-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Calc/TreeCopy.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo::style::calc
{
  Ref<Value> Value::Create(css::Category category, css::Range range, calc::Tree &&tree) noexcept
  {
    return AdoptRef(*new Value(category, range, krys::move(tree)));
  }

  Value::Value(css::Category category, css::Range range, calc::Tree &&tree) noexcept
      : _category(category), _range(range), _tree(krys::move(tree))
  {
  }

  Value::~Value() noexcept = default;

  double Value::Evaluate(double percentResolutionLength, const ZoomFactor &usedZoom) const noexcept
  {
    auto result = calc::Evaluate(_tree, percentResolutionLength, usedZoom);
    if (std::isnan(result))
    {
      return 0;
    }
    return std::clamp(result, _range.Min, _range.Max);
  }

  double Value::Evaluate(double percentResolutionLength, const ZoomNeeded &zoomNeeded) const noexcept
  {
    auto result = calc::Evaluate(_tree, percentResolutionLength, zoomNeeded);
    if (std::isnan(result))
    {
      return 0;
    }
    return std::clamp(result, _range.Min, _range.Max);
  }

  calc::Tree Value::CopyTree() const noexcept
  {
    return calc::Copy(_tree);
  }

  Child Value::CopyRoot() const noexcept
  {
    auto tree = CopyTree();
    return {krys::move(tree.Root)};
  }

  bool Value::operator==(const Value &other) const noexcept
  {
    return _tree == other._tree;
  }
}
