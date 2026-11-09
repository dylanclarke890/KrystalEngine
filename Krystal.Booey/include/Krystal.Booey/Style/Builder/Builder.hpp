/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  class CustomPropertyValue;
  class Value;
  enum class GlobalKeyword : uint8;
}

namespace krys::boo::style
{
  class CustomProperty;
  struct MatchResult;

  /// @brief Builds a style from the declarations matched for an element: cascades them, then applies the
  /// winning value of each property by priority, resolving `var()` references, custom properties (with cycle
  /// detection) and `revert`/`revert-layer` along the way.
  class Builder
  {
    KRYS_NON_COPY_MOVABLE(Builder);

    using ResolvedCustomProperty = Variant<Ref<const CustomProperty>, css::GlobalKeyword>;

    struct RollbackCascadeKey
    {
      uint32 Origin {0};
      uint32 ScopeOrdinal {0};
      uint32 CascadeLayerPriority {0};

      KRYS_NODISCARD bool operator==(const RollbackCascadeKey &) const noexcept = default;
    };

    struct RollbackCascadeKeyHash
    {
      KRYS_NODISCARD size_t operator()(const RollbackCascadeKey &key) const noexcept;
    };

    enum class CustomPropertyCycleTracking : bool
    {
      Enabled,
      Disabled
    };

  private:
    const PropertyCascade _cascade;
    // Rollback cascades are built on demand to resolve 'revert' and 'revert-layer' keywords.
    HashMap<RollbackCascadeKey, UniquePtr<const PropertyCascade>, RollbackCascadeKeyHash> _rollbackCascades;

    const UniquePtr<BuilderState> _state;

  public:
    Builder(RenderStyle &, BuilderContext &&, const MatchResult &,
            PropertyCascade::IncludedProperties &&includedProperties = PropertyCascade::NormalProperties(),
            const HashSet<AnimatableCSSProperty> *animatedProperties = nullptr,
            const HashSet<AnimatableCSSProperty> *transitioningProperties = nullptr) noexcept;
    ~Builder() noexcept;

    void ApplyAllProperties() noexcept;
    void ApplyTopPriorityProperties() noexcept;
    void ApplyHighPriorityProperties() noexcept;
    void ApplyNonHighPriorityProperties() noexcept;
    void AdjustAfterApplying() noexcept;

    void ApplyProperty(css::PropertyId id) noexcept
    {
      ApplyProperties(id, id);
    }
    void ApplyCustomProperty(const css::CSSOMStringAtom &name) noexcept;

    KRYS_NODISCARD RefPtr<const CustomProperty>
      ResolveCustomPropertyForContainerQueries(const css::CustomPropertyValue &) noexcept;

    KRYS_NODISCARD BuilderState &State() noexcept
    {
      return *_state;
    }

    KRYS_NODISCARD HashSet<AnimatableCSSProperty> OverriddenAnimatedProperties() const noexcept
    {
      return _cascade.OverriddenAnimatedProperties();
    }

  private:
    void ApplyProperties(css::PropertyId firstProperty, css::PropertyId lastProperty) noexcept;
    void ApplyLogicalGroupProperties() noexcept;
    void ApplyCustomProperties() noexcept;
    void ApplyCustomPropertyImpl(const css::CSSOMStringAtom &, const PropertyCascade::Property &) noexcept;

    template <CustomPropertyCycleTracking trackCycles>
    void ApplyPropertiesImpl(css::PropertyId firstProperty, css::PropertyId lastProperty) noexcept;
    void ApplyCascadeProperty(const PropertyCascade::Property &) noexcept;
    KRYS_NODISCARD bool ApplyRollbackCascadeProperty(const PropertyCascade &, css::PropertyId) noexcept;
    KRYS_NODISCARD bool ApplyRollbackCascadeCustomProperty(const PropertyCascade &,
                                                           const css::CSSOMStringAtom &) noexcept;
    void ApplyProperty(css::PropertyId, css::Value &, PropertyCascade::Origin) noexcept;
    void ApplyCustomProperty(const css::CSSOMStringAtom &name, ResolvedCustomProperty &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ResolveInternalAutoBaseFunction(css::Value &) noexcept;
    KRYS_NODISCARD Ref<css::Value> ResolveVariableReferences(css::PropertyId, css::Value &) noexcept;
    KRYS_NODISCARD Maybe<ResolvedCustomProperty>
      ResolveCustomPropertyValue(css::CustomPropertyValue &) noexcept;

    KRYS_NODISCARD const PropertyCascade *EnsureRollbackCascadeForRevert() noexcept;
    KRYS_NODISCARD const PropertyCascade *EnsureRollbackCascadeForRevertLayer() noexcept;

    KRYS_NODISCARD static RollbackCascadeKey MakeRollbackCascadeKey(PropertyCascade::Origin,
                                                                    ScopeOrdinal = ScopeOrdinal::Element,
                                                                    CascadeLayerPriority = 0) noexcept;
  };
}
