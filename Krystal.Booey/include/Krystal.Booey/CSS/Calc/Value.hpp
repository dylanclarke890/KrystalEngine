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

#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::style::calc
{
  class Value;
}

namespace krys::boo::css
{
  class AllowedSymbols;
  class SymbolTable;
  class TokenRange;
  class ToLengthConversionData;

  struct ComputedStyleDependencies;
  struct NoConversionDataRequiredTag;
  struct PropertyParserOptions;
  struct PropertyParserState;
  struct SerialisationContext;

  enum class Category : uint8;
  enum class UnitType : uint8;
}

namespace krys::boo::css::calc
{
  /// @brief Boxes a `calc::Tree` along with the `css::Category` and `css::Range` used to construct it.
  class Value final : public RefCounted<Value>
  {
  private:
    css::Category _category;
    css::Range _range;
    calc::Tree _tree;

  public:
    KRYS_NODISCARD static RefPtr<Value> Parse(TokenRange &, PropertyParserState &, css::Category, css::Range,
                                              AllowedSymbols, PropertyParserOptions) noexcept;

    KRYS_NODISCARD static Ref<Value> Create(const style::calc::Value &, const RenderStyle &) noexcept;
    KRYS_NODISCARD static Ref<Value> Create(css::Category, css::Range, calc::Tree &&) noexcept;

    ~Value() noexcept;

    // Creates a copy of the calc::Tree with non-canonical dimensions and any symbols present in the provided
    // symbol table resolved.
    KRYS_NODISCARD Ref<Value> CopySimplified(const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD Ref<Value> CopySimplified(const ToLengthConversionData &,
                                             const SymbolTable &) const noexcept;
    KRYS_NODISCARD Ref<Value> CopySimplified(NoConversionDataRequiredTag) const noexcept;
    KRYS_NODISCARD Ref<Value> CopySimplified(NoConversionDataRequiredTag, const SymbolTable &) const noexcept;

    KRYS_NODISCARD css::Category GetCategory() const noexcept
    {
      return _category;
    }

    KRYS_NODISCARD css::Range GetRange() const noexcept
    {
      return _range;
    }

    KRYS_NODISCARD UnitType PrimitiveType() const noexcept;

    // Returns whether the calc::Tree requires `ToLengthConversionData` to fully resolve.
    KRYS_NODISCARD bool RequiresConversionData() const noexcept
    {
      return _tree.RequiresConversionData;
    }

    KRYS_NODISCARD double DoubleValue(const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD double DoubleValue(const ToLengthConversionData &, const SymbolTable &) const noexcept;
    KRYS_NODISCARD double DoubleValue(NoConversionDataRequiredTag) const noexcept;
    KRYS_NODISCARD double DoubleValue(NoConversionDataRequiredTag, const SymbolTable &) const noexcept;
    KRYS_NODISCARD double DoubleValueDeprecated() const noexcept;

    KRYS_NODISCARD double ComputeLengthPx(const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD double ComputeLengthPx(const ToLengthConversionData &, const SymbolTable &) const noexcept;

    KRYS_NODISCARD Ref<style::calc::Value> CreateCalculationValue(NoConversionDataRequiredTag) const noexcept;
    KRYS_NODISCARD Ref<style::calc::Value> CreateCalculationValue(NoConversionDataRequiredTag,
                                                                  const SymbolTable &) const noexcept;
    KRYS_NODISCARD Ref<style::calc::Value>
      CreateCalculationValue(const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD Ref<style::calc::Value> CreateCalculationValue(const ToLengthConversionData &,
                                                                  const SymbolTable &) const noexcept;

    void CollectComputedStyleDependencies(ComputedStyleDependencies &) const noexcept;

    KRYS_NODISCARD CSSOMString CssText(const SerialisationContext &) const noexcept;
    KRYS_NODISCARD bool Equals(const Value &) const noexcept;

    KRYS_NODISCARD const calc::Tree &GetTree() const noexcept
    {
      return _tree;
    }

  private:
    explicit Value(css::Category, css::Range, calc::Tree &&) noexcept;

    KRYS_NODISCARD double ClampToPermittedRange(double) const noexcept;
  };
}
