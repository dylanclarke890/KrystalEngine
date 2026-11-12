/*
 * Copyright (C) 2004 Zack Rusin <zack@kde.org>
 * Copyright (C) 2004-2019 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#pragma once

#include "Krystal.Booey/CSS/CSSOM/CSSStyleProperties.hpp"
#include "Krystal.Booey/Style/Extractor/Extractor.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::css
{
  class MutableStyleProperties;

  /// @brief The read-only declaration block of an element's resolved values, as returned by
  /// `getComputedStyle()`.
  class CSSComputedStyleDeclaration final : public CSSStyleProperties,
                                            public RefCounted<CSSComputedStyleDeclaration>
  {
  private:
    enum class IsEmpty : bool
    {
      No,
      Yes
    };

    const Ref<dom::Element> _element;
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier {null};
    bool _isEmpty {false};

  public:
    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }

    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration>
      Create(dom::Element &, const Maybe<style::PseudoElementIdentifier> &) noexcept;
    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration> CreateEmpty(dom::Element &) noexcept;

    ~CSSComputedStyleDeclaration() noexcept override;

    KRYS_NODISCARD CSSOMString GetPropertyValue(PropertyId) const noexcept;

  private:
    CSSComputedStyleDeclaration(dom::Element &, IsEmpty) noexcept;
    CSSComputedStyleDeclaration(dom::Element &, const Maybe<style::PseudoElementIdentifier> &) noexcept;

    // CSSOM functions. Don't make these public.
    KRYS_NODISCARD CSSRule *ParentRule() const noexcept final;
    KRYS_NODISCARD uint32 Length() const noexcept final;
    KRYS_NODISCARD CSSOMString Item(uint32 index) const noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyValue(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyPriority(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyShorthand(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD bool IsPropertyImplicit(const CSSOMString &propertyName) noexcept final;
    dom::ExceptionOr<void> SetProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                       const CSSOMString &priority) noexcept final;
    dom::ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &propertyName) noexcept final;
    KRYS_NODISCARD CSSOMString CssText() const noexcept final;
    dom::ExceptionOr<void> SetCssText(const CSSOMString &) noexcept final;
    KRYS_NODISCARD CSSOMString GetPropertyValueInternal(PropertyId) noexcept final;
    dom::ExceptionOr<void> SetPropertyInternal(PropertyId, const CSSOMString &value,
                                               IsImportant) noexcept final;
    KRYS_NODISCARD Ref<MutableStyleProperties> CopyProperties() const noexcept final;

    KRYS_NODISCARD Ref<dom::Element> ProtectedElement() const noexcept
    {
      return _element;
    }

    KRYS_NODISCARD const FixedVector<PropertyId> &ExposedComputedCSSPropertyIds() const noexcept;

    KRYS_NODISCARD style::Extractor GetExtractor() const noexcept;
  };
}
