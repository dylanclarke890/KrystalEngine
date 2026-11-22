/*
 * Copyright (C) 2010, 2011 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Events/ToggleEventTask.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo
{
  class HTMLSlotElement;
  class HTMLSummaryElement;

  /// @brief A disclosure widget: its first `summary` child is its caption, and the rest of its contents show
  /// only while it's open. Elements with the same `name` form an exclusive accordion: opening one closes the
  /// others.
  /// @see https://html.spec.whatwg.org/#the-details-element
  class HTMLDetailsElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDetailsElement);

  private:
    WeakPtr<HTMLSlotElement> _summarySlot;
    WeakPtr<HTMLSummaryElement> _defaultSummary;
    RefPtr<HTMLSlotElement> _defaultSlot;
    /// @see https://html.spec.whatwg.org/#details-toggle-task-tracker
    RefPtr<dom::ToggleEventTask> _toggleEventTask;

  public:
    explicit HTMLDetailsElement(Document &document) noexcept;

    ~HTMLDetailsElement() noexcept override;

    KRYS_NODISCARD static Ref<HTMLDetailsElement> Create(Document &document) noexcept;

#pragma region HTMLDetailsElement - https://html.spec.whatwg.org/#htmldetailselement

    /// @see https://html.spec.whatwg.org/#dom-details-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-details-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-details-open
    KRYS_NODISCARD bool Open() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-details-open
    void Open(bool value) noexcept;

#pragma endregion

    /// @brief Opens the element if it's closed, and closes it if it's open.
    void ToggleOpen() noexcept;

    /// @brief Whether @p summary is the element's active summary: its first `summary` child, or its default
    /// summary if it has none.
    KRYS_NODISCARD bool IsActiveSummary(const HTMLSummaryElement &summary) const noexcept;

    /// @see https://html.spec.whatwg.org/#queue-a-details-toggle-event-task
    void QueueDetailsToggleEventTask(dom::ToggleState oldState, dom::ToggleState newState) noexcept;

    /// @brief The slot of the element's shadow tree that takes @p child: the summary slot for the first
    /// `summary` child, the content slot otherwise.
    KRYS_NODISCARD HTMLSlotElement *SlotForHostChild(const Node &child) const noexcept;

    /// @brief Reassigns the element's children to its slots after a `summary` child was inserted, removed or
    /// moved.
    void SummaryChildChanged() noexcept;

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return true;
    }

  protected:
#pragma region Extensibility Hooks

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept final;

    void OnPostConnection() noexcept final;

#pragma endregion

    void DidAddUserAgentShadowRoot(boo::ShadowRoot &root) noexcept final;

  private:
    /// @brief The other elements in the element's details name group.
    /// @see https://html.spec.whatwg.org/#details-name-group
    KRYS_NODISCARD List<Ref<HTMLDetailsElement>> OtherElementsInNameGroup() noexcept;

    /// @see https://html.spec.whatwg.org/#ensure-details-exclusivity-by-closing-the-given-element-if-needed
    void EnsureDetailsExclusivityAfterMutation() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLDetailsElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLDetailsElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLDetailsElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
