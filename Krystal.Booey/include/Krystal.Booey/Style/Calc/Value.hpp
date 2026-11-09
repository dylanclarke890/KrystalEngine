/*
 * Copyright (C) 2011, 2012 Google Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  enum class Category : uint8;
}

namespace krys::boo::style
{
  struct ZoomFactor;
  struct ZoomNeeded;
}

namespace krys::boo::style::calc
{
  /// @brief Boxes a `style::calc::Tree` along with the `css::Category` and `css::Range` used to construct it.
  class Value final : public RefCounted<Value>
  {
  private:
    css::Category _category;
    css::Range _range;
    calc::Tree _tree;

  public:
    KRYS_NODISCARD static Ref<Value> Create(css::Category, css::Range, calc::Tree &&) noexcept;

    ~Value() noexcept;

    KRYS_NODISCARD double Evaluate(double percentResolutionLength, const ZoomFactor &usedZoom) const noexcept;
    KRYS_NODISCARD double Evaluate(double percentResolutionLength, const ZoomNeeded &) const noexcept;

    KRYS_NODISCARD css::Category GetCategory() const noexcept
    {
      return _category;
    }

    KRYS_NODISCARD css::Range GetRange() const noexcept
    {
      return _range;
    }

    KRYS_NODISCARD const calc::Tree &GetTree() const noexcept
    {
      return _tree;
    }

    KRYS_NODISCARD calc::Tree CopyTree() const noexcept;
    KRYS_NODISCARD Child CopyRoot() const noexcept;

    KRYS_NODISCARD bool operator==(const Value &) const noexcept;

  private:
    explicit Value(css::Category, css::Range, calc::Tree &&) noexcept;
  };
}
