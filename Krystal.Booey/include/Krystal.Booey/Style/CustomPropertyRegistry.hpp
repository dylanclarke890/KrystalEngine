/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Properties/RegisteredCustomProperty.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  class CustomProperty;
  class Scope;

  /// @brief The custom properties registered in a scope, from the API and from `@property` rules, and a style
  /// holding their initial values.
  class CustomPropertyRegistry
  {
    KRYS_NON_COPY_MOVABLE(CustomPropertyRegistry);

  public:
    enum class ViewportUnitDependency : bool
    {
      No,
      Yes
    };

    enum class ParseInitialValueError : uint8
    {
      NotComputationallyIndependent,
      DidNotParse
    };

  private:
    Scope &_scope;

    HashMap<css::CSSOMStringAtom, UniquePtr<css::RegisteredCustomProperty>> _propertiesFromAPI;
    HashMap<css::CSSOMStringAtom, UniquePtr<css::RegisteredCustomProperty>> _propertiesFromStylesheet;

    mutable UniquePtr<RenderStyle> _initialValuePrototypeStyle;
    mutable bool _hasInvalidPrototypeStyle {false};

  public:
    explicit CustomPropertyRegistry(Scope &) noexcept;
    ~CustomPropertyRegistry() noexcept;

    KRYS_NODISCARD const css::RegisteredCustomProperty *Get(const css::CSSOMStringAtom &) const noexcept;
    KRYS_NODISCARD bool IsInherited(const css::CSSOMStringAtom &) const noexcept;

    bool RegisterFromAPI(css::RegisteredCustomProperty &&) noexcept;
    void RegisterFromStylesheet(const css::PropertyRule::Descriptor &) noexcept;
    void ClearRegisteredFromStylesheets() noexcept;

    KRYS_NODISCARD const RenderStyle &InitialValuePrototypeStyle() const noexcept;

    bool InvalidatePropertiesWithViewportUnits(dom::Document &) noexcept;

    KRYS_NODISCARD static Expected<Pair<RefPtr<const CustomProperty>, ViewportUnitDependency>,
                                   ParseInitialValueError>
      ParseInitialValue(const dom::Document &, const css::CSSOMStringAtom &propertyName,
                        const css::CustomPropertySyntax &, css::TokenRange) noexcept;

  private:
    void Invalidate(const css::CSSOMStringAtom &) noexcept;
    void NotifyAnimationsOfCustomPropertyRegistration(const css::CSSOMStringAtom &) noexcept;
  };
}
