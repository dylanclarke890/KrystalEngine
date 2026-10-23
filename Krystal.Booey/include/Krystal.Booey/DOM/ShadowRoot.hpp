#pragma once

#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/Enums/ShadowRootMode.hpp"
#include "Krystal.Booey/DOM/Enums/SlotAssignmentMode.hpp"

namespace krys::boo::css
{
  class StyleSheetList;
}

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#interface-shadowroot
  class ShadowRoot : public DocumentFragment, public DocumentOrShadowRoot
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ShadowRoot);

    friend class DocumentAlgorithms;
    friend class ElementAlgorithms;
    friend class NodeAlgorithms;

  private:
    ShadowRootMode _mode : BitCount<ShadowRootMode>() {ShadowRootMode::Open};
    bool _delegatesFocus : 1 {false};
    bool _availableToElementInternals : 1 {false};
    bool _declarative : 1 {false};
    SlotAssignmentMode _slotAssignment : BitCount<SlotAssignmentMode>() {SlotAssignmentMode::Manual};
    bool _clonable : 1 {false};
    bool _serializable : 1 {false};
    bool _keepCustomElementRegistryNull : 1 {false};
    bool _containsFocusedElement : 1 {false};
    RefPtr<css::StyleSheetList> _styleSheetList;

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

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-host
    KRYS_NODISCARD Element *Host() const noexcept;

    // TODO(impl): SHADOW-ROOT - The onslotchange attribute is an event handler IDL attribute for the
    // onslotchange event handler, whose event handler event type is slotchange.

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (CSSOM) - https://drafts.csswg.org/cssom/#extensions-to-the-document-or-shadow-root-interface

    /// @brief The style sheets of the shadow root, in order.
    /// @see https://drafts.csswg.org/cssom/#dom-documentorshadowroot-stylesheets
    KRYS_NODISCARD css::StyleSheetList &StyleSheets() noexcept;

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