/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2013-2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
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

#include "Krystal.Booey/DOM/Mixins/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/GetHTMLOptions.hpp"
#include "Krystal.Booey/DOM/Parsing/ParserContentPolicy.hpp"
#include "Krystal.Booey/HTML/Attributes/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SetHTMLOptions.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class StyleSheetList;
}

namespace krys::boo::html
{
  class CustomElementRegistry;
  class TreeBuilder;
}

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#enumdef-shadowrootmode
  enum class ShadowRootMode : uint8
  {
    Open,
    Closed,
    /// @brief A shadow root the user agent attaches for an element's insides (a form control's), which
    /// pages can't see.
    UserAgent
  };

  /// @see https://dom.spec.whatwg.org/#enumdef-slotassignmentmode
  enum class SlotAssignmentMode : uint8
  {
    Manual,
    Named
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::ShadowRootMode, 3u);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::SlotAssignmentMode, 2u);

namespace krys::boo::dom
{
  struct DelegatesFocus : public StronglyTypedBool<DelegatesFocus>
  {
    using Base::Base;
  };

  struct Clonable : public StronglyTypedBool<Clonable>
  {
    using Base::Base;
  };

  struct Serialisable : public StronglyTypedBool<Serialisable>
  {
    using Base::Base;
  };

  /// @see https://dom.spec.whatwg.org/#dictdef-shadowrootinit
  struct ShadowRootInit
  {
    ShadowRootMode Mode {ShadowRootMode::Open};
    DelegatesFocus DelegatesFocus {false};
    SlotAssignmentMode SlotAssignment {SlotAssignmentMode::Named};
    Clonable Clonable {false};
    Serialisable Serialisable {false};
    /// @see https://dom.spec.whatwg.org/#dom-shadowrootinit-referencetarget
    Maybe<DOMString> ReferenceTarget;
    html::CustomElementRegistry *CustomElementRegistry {nullptr};
  };

  /// @brief The root of a shadow tree, attached to its host element.
  /// @see https://dom.spec.whatwg.org/#interface-shadowroot
  /// @see https://dom.spec.whatwg.org/#concept-shadow-root
  class ShadowRoot : public DocumentFragment, public DocumentOrShadowRoot
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ShadowRoot);

    friend class DocumentAlgorithms;
    friend class ElementAlgorithms;
    friend class NodeAlgorithms;
    friend class html::TreeBuilder;

  public:
    /// @brief Each part name of the shadow tree that the host exports, with the names it's exported as.
    using PartMappings = HashMap<DOMStringAtom, SmallList<DOMStringAtom, 1>>;

  private:
    /// @brief Open, closed, or the user agent's own (treated as closed).
    /// @see https://dom.spec.whatwg.org/#shadowroot-mode
    ShadowRootMode _mode : BitCount<ShadowRootMode>() {ShadowRootMode::Open};
    /// @see https://dom.spec.whatwg.org/#shadowroot-delegates-focus
    bool _delegatesFocus : 1 {false};
    /// @see https://dom.spec.whatwg.org/#shadowroot-available-to-element-internals
    bool _availableToElementInternals : 1 {false};
    /// @see https://dom.spec.whatwg.org/#shadowroot-declarative
    bool _declarative : 1 {false};
    /// @see https://dom.spec.whatwg.org/#shadowroot-slot-assignment
    SlotAssignmentMode _slotAssignment : BitCount<SlotAssignmentMode>() {SlotAssignmentMode::Manual};
    /// @see https://dom.spec.whatwg.org/#shadowroot-clonable
    bool _clonable : 1 {false};
    /// @see https://dom.spec.whatwg.org/#shadowroot-serializable
    bool _serializable : 1 {false};
    /// @see https://dom.spec.whatwg.org/#shadowroot-keep-custom-element-registry-null
    bool _keepCustomElementRegistryNull : 1 {false};
    bool _containsFocusedElement : 1 {false};
    RefPtr<css::StyleSheetList> _styleSheetList;
    /// @brief The ID of the element in the shadow tree that references to the host go to, or null.
    /// @see https://dom.spec.whatwg.org/#shadowroot-reference-target
    Maybe<DOMString> _referenceTarget;
    /// @brief Parsed from the host's `exportparts` when first needed.
    mutable Maybe<PartMappings> _partMappings;

  public:
    ShadowRoot(Document &document, RefPtr<html::CustomElementRegistry> &&registry = nullptr,
               NodeFlags flags = NodeFlags::None) noexcept;
    ~ShadowRoot() noexcept override;

#pragma region ShadowRoot - https://dom.spec.whatwg.org/#shadowroot

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-mode
    KRYS_NODISCARD ShadowRootMode Mode() const noexcept
    {
      return _mode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-delegatesfocus
    KRYS_NODISCARD bool DelegatesFocus() const noexcept
    {
      return _delegatesFocus;
    }

    /// @brief Sets whether a user agent shadow root delegates focus: its control's parts (a date input's
    /// fields) are focused instead of the control, and sequential navigation moves between them.
    void SetUserAgentDelegatesFocus(bool delegatesFocus) noexcept
    {
      krys_debug_assert(_mode == ShadowRootMode::UserAgent);
      _delegatesFocus = delegatesFocus;
    }

    /// @brief Whether the shadow root was attached while its host was a custom element, so that the host's
    /// internals expose it.
    /// @see https://dom.spec.whatwg.org/#shadowroot-available-to-element-internals
    KRYS_NODISCARD bool IsAvailableToElementInternals() const noexcept
    {
      return _availableToElementInternals;
    }

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-slotassignment
    KRYS_NODISCARD SlotAssignmentMode SlotAssignment() const noexcept
    {
      return _slotAssignment;
    }

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-clonable
    KRYS_NODISCARD bool Clonable() const noexcept
    {
      return _clonable;
    }

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-serializable
    KRYS_NODISCARD bool Serialisable() const noexcept
    {
      return _serializable;
    }

    /// @brief The ID of the element in the shadow tree that references to the host (a label's `for`, a
    /// button's `commandfor`) go to, or null for the host itself.
    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-referencetarget
    KRYS_NODISCARD const Maybe<DOMString> &ReferenceTarget() const noexcept
    {
      return _referenceTarget;
    }
    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-referencetarget
    void SetReferenceTarget(Maybe<DOMString> referenceTarget) noexcept;
    /// @brief Tells the observers of the host's ID (form controls whose `form` names it) that what a
    /// reference to the host resolves to may have changed.
    void NotifyHostIdTargetObservers() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-host
    KRYS_NODISCARD Element *Host() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-onslotchange
    /// @see https://dom.spec.whatwg.org/#shadowroot-onslotchange
    KRYS_DECLARE_EVENT_HANDLER(SlotChange, u8"slotchange")

#pragma endregion

#pragma region ShadowRoot (DOM Parsing) - https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-parsing-and-serialization

    /// @brief Replaces the shadow root's children with @p html, parsed as HTML in its host's context,
    /// sanitized by @p options (the default configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-shadowroot-sethtml
    ExceptionOr<void> SetHTML(DOMStringView html, html::SetHTMLOptions &&options = {}) noexcept;

    /// @brief Appends @p html, parsed as HTML in the host's context, sanitized by @p options (the default
    /// configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-shadowroot-appendhtml
    ExceptionOr<void> AppendHTML(DOMStringView html, html::SetHTMLOptions &&options = {}) noexcept;

    /// @brief Prepends @p html, parsed as HTML in the host's context, sanitized by @p options (the default
    /// configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-shadowroot-prependhtml
    ExceptionOr<void> PrependHTML(DOMStringView html, html::SetHTMLOptions &&options = {}) noexcept;

    /// @brief Replaces the shadow root's children with @p html, parsed as HTML in its host's context;
    /// sanitized only if @p options gives a sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-shadowroot-sethtmlunsafe
    ExceptionOr<void> SetHTMLUnsafe(DOMStringView html, html::SetHTMLUnsafeOptions &&options = {}) noexcept;

    /// @brief Appends @p html, parsed as HTML in the host's context; sanitized only if @p options gives a
    /// sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-shadowroot-appendhtmlunsafe
    ExceptionOr<void> AppendHTMLUnsafe(DOMStringView html,
                                       html::SetHTMLUnsafeOptions &&options = {}) noexcept;

    /// @brief Prepends @p html, parsed as HTML in the host's context; sanitized only if @p options gives a
    /// sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-shadowroot-prependhtmlunsafe
    ExceptionOr<void> PrependHTMLUnsafe(DOMStringView html,
                                        html::SetHTMLUnsafeOptions &&options = {}) noexcept;

    /// @brief The shadow root's children serialised as HTML, with the shadow roots @p options asks for.
    /// @see https://html.spec.whatwg.org/#dom-shadowroot-gethtml
    KRYS_NODISCARD DOMString GetHTML(GetHTMLOptions &&options = {}) const noexcept;

    /// @brief The children serialized; in an XML document, an `InvalidStateError` if they can't be serialized
    /// as well-formed XML.
    /// @see https://html.spec.whatwg.org/#dom-shadowroot-innerhtml
    KRYS_NODISCARD ExceptionOr<DOMString> InnerHTML() const noexcept;
    /// @see https://html.spec.whatwg.org/#dom-shadowroot-innerhtml
    ExceptionOr<void> InnerHTML(DOMStringView markup) noexcept;

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (CSSOM) - https://drafts.csswg.org/cssom/#extensions-to-the-document-or-shadow-root-interface

    /// @brief The style sheets of the shadow root, in order.
    /// @see https://drafts.csswg.org/cssom/#dom-documentorshadowroot-stylesheets
    KRYS_NODISCARD css::StyleSheetList &StyleSheets() noexcept;

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (Web Animations) - https://drafts.csswg.org/web-animations-1/#extensions-to-the-documentorshadowroot-interface-mixin

    /// @brief The relevant animations whose effects target elements in this shadow tree, in composite order.
    /// @see https://drafts.csswg.org/web-animations-1/#dom-documentorshadowroot-getanimations
    KRYS_NODISCARD List<RefPtr<WebAnimation>> GetAnimations() noexcept;

#pragma endregion

    /// @brief Whether the focused element is in this shadow tree (the host matches `:focus`).
    KRYS_NODISCARD bool ContainsFocusedElement() const noexcept
    {
      return _containsFocusedElement;
    }

    void SetContainsFocusedElement(bool flag) noexcept
    {
      _containsFocusedElement = flag;
    }

    /// @brief The part names the host's `exportparts` attribute exports from the shadow tree.
    /// @see https://drafts.csswg.org/css-shadow-parts/#exportparts-attr
    KRYS_NODISCARD const PartMappings &GetPartMappings() const noexcept;
    /// @brief Forgets the part mappings, after the host's `exportparts` changes.
    void InvalidatePartMappings() noexcept
    {
      _partMappings = null;
    }

  private:
    /// @brief Replaces the shadow root's children with @p markup parsed in its host's context.
    ExceptionOr<void> ReplaceChildrenWithMarkup(DOMStringView markup,
                                                EnumSet<ParserContentPolicy> parserContentPolicy) noexcept;

  protected:
    /// @see https://dom.spec.whatwg.org/#get-the-parent
    KRYS_NODISCARD EventTarget *GetParent(Event &event) const noexcept override;

#pragma region Extension Hooks

    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;
    void OnAdopt(Document &oldDocument) noexcept override;

#pragma endregion
  };

  /// @brief The shadow roots of the slots that @p node is assigned to: its assigned slot's, then that slot's
  /// assigned slot's, and so on.
  KRYS_NODISCARD List<Ref<ShadowRoot>> AssignedShadowRootsIfSlotted(const Node &node) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::ShadowRoot)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsShadowRootNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();