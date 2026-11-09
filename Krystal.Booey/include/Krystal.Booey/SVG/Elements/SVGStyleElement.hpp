/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2013-2024 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/HTML/Internals/InlineStyleSheetOwner.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;
}

namespace krys::boo::svg
{
  /// @brief The `<style>` element in SVG: a style sheet made from its text, as HTML's.
  /// @see https://svgwg.org/svg2-draft/styling.html#InterfaceSVGStyleElement
  class SVGStyleElement final : public SVGElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGStyleElement);

  private:
    html::InlineStyleSheetOwner _styleSheetOwner;

  public:
    explicit SVGStyleElement(dom::Document &document) noexcept;
    ~SVGStyleElement() noexcept override;

    /// @brief The associated CSS style sheet, if any.
    /// @see https://drafts.csswg.org/cssom/#dom-linkstyle-sheet
    KRYS_NODISCARD css::CSSStyleSheet *Sheet() const noexcept
    {
      return _styleSheetOwner.Sheet();
    }

    /// @see https://svgwg.org/svg2-draft/styling.html#__svg__SVGStyleElement__disabled
    KRYS_NODISCARD bool Disabled() const noexcept;
    void Disabled(bool value) noexcept;

    /// @see https://svgwg.org/svg2-draft/styling.html#__svg__SVGStyleElement__type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;
    void Type(dom::DOMString value) noexcept;

    /// @see https://svgwg.org/svg2-draft/styling.html#__svg__SVGStyleElement__media
    KRYS_NODISCARD dom::DOMString Media() const noexcept;
    void Media(dom::DOMString value) noexcept;

    /// @see https://svgwg.org/svg2-draft/styling.html#__svg__SVGStyleElement__title
    KRYS_NODISCARD dom::DOMString Title() const noexcept;
    void Title(dom::DOMString value) noexcept;

    bool SheetLoaded() noexcept final
    {
      return _styleSheetOwner.SheetLoaded(*this);
    }

    void StartLoadingDynamicSheet() noexcept final
    {
      _styleSheetOwner.StartLoadingDynamicSheet(*this);
    }

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void OnInsert() noexcept final;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept final;
    void OnChildrenChanged() noexcept final;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }

    void FinishParsingChildren() noexcept final;
    void NotifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred) noexcept final;

    KRYS_NODISCARD bool IsLoading() const noexcept
    {
      return _styleSheetOwner.IsLoading();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGStyleElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGStyleElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
