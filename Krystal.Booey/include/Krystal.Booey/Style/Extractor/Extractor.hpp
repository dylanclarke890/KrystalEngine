/*
 * Copyright (C) 2019-2023 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/Extractor/ExtractorState.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderElement;
  class RenderStyle;
}

namespace krys::boo::css
{
  class MutableStyleProperties;
  class PrimitiveValue;
  class Value;
  class ValuePool;
  struct SerialisationContext;
}

namespace krys::boo::dom
{
  class Element;
  class Node;
}

namespace krys::boo::style
{
  /// @brief Extracts the computed (or resolved) values of an element's properties.
  class Extractor
  {
  private:
    RefPtr<dom::Element> _element;
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier;

  public:
    explicit Extractor(dom::Node *) noexcept;
    Extractor(dom::Node *, const Maybe<style::PseudoElementIdentifier> &) noexcept;
    explicit Extractor(dom::Element *) noexcept;
    Extractor(dom::Element *, const Maybe<style::PseudoElementIdentifier> &) noexcept;

    enum class UpdateLayout : bool
    {
      No,
      Yes
    };

    /// @brief Extract a css::Value for the specified property.
    KRYS_NODISCARD RefPtr<css::Value> PropertyValue(
      css::PropertyId, UpdateLayout = UpdateLayout::Yes,
      ExtractorState::PropertyValueType = ExtractorState::PropertyValueType::Resolved) const noexcept;

    /// @brief Extract a serialised value for the specified property.
    KRYS_NODISCARD css::CSSOMString PropertyValueSerialisation(
      css::PropertyId, const css::SerialisationContext &, UpdateLayout = UpdateLayout::Yes,
      ExtractorState::PropertyValueType = ExtractorState::PropertyValueType::Resolved) const noexcept;

    /// @brief Extract a css::Value for the specified property using the provided RenderStyle and
    /// RenderElement.
    KRYS_NODISCARD RefPtr<css::Value> PropertyValueInStyle(
      const RenderStyle &, css::PropertyId, css::ValuePool &, const RenderElement * = nullptr,
      ExtractorState::PropertyValueType = ExtractorState::PropertyValueType::Resolved) const noexcept;

    /// @brief Extract a serialised value for the specified property using the provided RenderStyle and
    /// RenderElement.
    KRYS_NODISCARD css::CSSOMString PropertyValueSerialisationInStyle(
      const RenderStyle &, css::PropertyId, const css::SerialisationContext &, css::ValuePool &,
      const RenderElement * = nullptr,
      ExtractorState::PropertyValueType = ExtractorState::PropertyValueType::Resolved) const noexcept;

    /// @brief Extract a css::Value for the specified custom property.
    KRYS_NODISCARD RefPtr<css::Value>
      CustomPropertyValue(const css::CSSOMStringAtom &propertyName) const noexcept;

    /// @brief Extract a serialised value for the specified custom property.
    KRYS_NODISCARD css::CSSOMString
      CustomPropertyValueSerialisation(const css::CSSOMStringAtom &propertyName,
                                       const css::SerialisationContext &) const noexcept;

    /// @brief Extract a serialised value for the specified custom property using the provided RenderStyle.
    KRYS_NODISCARD css::CSSOMString
      CustomPropertyValueSerialisationInStyle(const RenderStyle &, const css::CSSOMStringAtom &propertyName,
                                              const css::SerialisationContext &) const noexcept;

    // Helper methods for HTML editing.
    KRYS_NODISCARD Ref<css::MutableStyleProperties>
      CopyProperties(Span<const css::PropertyId>) const noexcept;
    KRYS_NODISCARD Ref<css::MutableStyleProperties> CopyProperties() const noexcept;
    KRYS_NODISCARD RefPtr<css::PrimitiveValue> GetFontSizeCSSValuePreferringKeyword() const noexcept;
    KRYS_NODISCARD bool UseFixedFontDefaultSize() const noexcept;
    KRYS_NODISCARD bool PropertyMatches(css::PropertyId, const css::Value *) const noexcept;

    static bool UpdateStyleIfNeededForProperty(dom::Element &, css::PropertyId) noexcept;

  private:
    /// @brief The renderer we should use for resolving layout-dependent properties.
    KRYS_NODISCARD const RenderElement *ComputeRenderer() const noexcept;

    /// @brief The RenderStyle we should use for resolving non-layout-dependent properties.
    KRYS_NODISCARD const RenderStyle *ComputeStyle(css::PropertyId, UpdateLayout,
                                                   UniquePtr<RenderStyle> &) const noexcept;

    /// @brief The RenderStyle we should use for resolving custom properties.
    KRYS_NODISCARD const RenderStyle *ComputeStyleForCustomProperty(UniquePtr<RenderStyle> &) const noexcept;
  };
}
