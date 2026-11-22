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
#include "Krystal.Core/Containers/FixedList.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class MutableStyleProperties;
}

namespace krys::boo
{
  class Element;

  /// @brief The read-only declaration block of an element's resolved values, as returned by
  /// `getComputedStyle()`.
  class CSSComputedStyleDeclaration final : public CSSStyleProperties,
                                            public RefCounted<CSSComputedStyleDeclaration>
  {
  private:
    enum class IsEmpty : bool
    {
      No,
      Yes,
    };

    const Ref<Element> _element;
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
      Create(Element &, const Maybe<style::PseudoElementIdentifier> &) noexcept;

    KRYS_NODISCARD static Ref<CSSComputedStyleDeclaration> CreateEmpty(Element &) noexcept;

    ~CSSComputedStyleDeclaration() noexcept override;

    KRYS_NODISCARD CSSOMString GetPropertyValue(css::PropertyId) const noexcept;

  private:
    CSSComputedStyleDeclaration(Element &, IsEmpty) noexcept;

    CSSComputedStyleDeclaration(Element &, const Maybe<style::PseudoElementIdentifier> &) noexcept;

    // CSSOM functions. Don't make these public.
    KRYS_NODISCARD CSSRule *ParentRule() const noexcept final;

    KRYS_NODISCARD uint32 Length() const noexcept final;

    KRYS_NODISCARD CSSOMString Item(uint32 index) const noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyValue(const CSSOMString &propertyName) noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyPriority(const CSSOMString &propertyName) noexcept final;

    ExceptionOr<void> SetProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                  const CSSOMString &priority) noexcept final;

    ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &propertyName) noexcept final;

    KRYS_NODISCARD CSSOMString CssText() const noexcept final;

    ExceptionOr<void> SetCssText(const CSSOMString &) noexcept final;

    KRYS_NODISCARD CSSOMString GetPropertyValueInternal(css::PropertyId) noexcept final;

    ExceptionOr<void> SetPropertyInternal(css::PropertyId, const CSSOMString &value,
                                          css::IsImportant) noexcept final;

    KRYS_NODISCARD Ref<css::MutableStyleProperties> CopyProperties() const noexcept final;

    KRYS_NODISCARD Ref<Element> ProtectedElement() const noexcept
    {
      return _element;
    }

    KRYS_NODISCARD const FixedList<css::PropertyId> &ExposedComputedCSSPropertyIds() const noexcept;

    KRYS_NODISCARD style::Extractor GetExtractor() const noexcept;
  };
}
