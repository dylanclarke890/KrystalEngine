/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2022 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
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
 *
 */

#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::html
{
  class HTMLFormElement;

  /// @see https://html.spec.whatwg.org/#htmllabelelement
  class HTMLLabelElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLabelElement);

  private:
    bool _processingClick {false};

  public:
    HTMLLabelElement(dom::Document &document) noexcept;

#pragma region HTMLLabelElement - https://html.spec.whatwg.org/#htmllabelelement

    /// @brief The form owner of the labeled control, if it has one.
    /// @see https://html.spec.whatwg.org/#dom-label-form
    KRYS_NODISCARD HTMLFormElement *Form() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-label-form
    KRYS_NODISCARD RefPtr<HTMLFormElement> FormForBindings() const noexcept;

    /// @brief The labeled control: the element the `for` attribute names, or the first labelable
    /// descendant.
    /// @see https://html.spec.whatwg.org/#labeled-control
    KRYS_NODISCARD RefPtr<HTMLElement> Control() const noexcept;

    /// @brief Runs the labeled control's access key action, if it has one.
    bool AccessKeyAction(bool sendMouseEvents) noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-label-control
    KRYS_NODISCARD RefPtr<HTMLElement> ControlForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-label-htmlfor
    KRYS_NODISCARD dom::DOMString HtmlFor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-label-htmlfor
    void HtmlFor(dom::DOMString value) noexcept;

#pragma endregion

    void SetActive(bool down,
                   style::InvalidationScope invalidationScope = style::InvalidationScope::All) noexcept final;
    void SetHovered(bool over, style::InvalidationScope invalidationScope = style::InvalidationScope::All,
                    HitTestRequest request = {}) noexcept final;

    void DefaultEventHandler(dom::Event &event) noexcept final;

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return true;
    }

  private:
    KRYS_NODISCARD bool IsEventTargetedAtInteractiveDescendants(dom::Event &event) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLLabelElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLLabelElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLLabelElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();