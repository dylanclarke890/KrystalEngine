#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLSelectElement;

  /// @see https://html.spec.whatwg.org/#htmloptgroupelement
  class HTMLOptGroupElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLOptGroupElement);

  private:
    WeakPtr<HTMLSelectElement> _ownerSelect;
    bool _isDisabled {false};

  private:
    WeakPtr<HTMLSlotElement> _legendSlot;
    WeakPtr<HTMLSlotElement> _contentsSlot;
    WeakPtr<dom::Element> _labelElement;
    WeakPtr<dom::Element> _slottedLegend;

  public:
    explicit HTMLOptGroupElement(dom::Document &document) noexcept;
    KRYS_NODISCARD static Ref<HTMLOptGroupElement> Create(dom::Document &document) noexcept;

    /// @brief The slot of the shadow tree that takes @p child: a first child element that's a legend takes
    /// the legend slot, which otherwise shows the label, and the other children the contents slot.
    /// @see https://html.spec.whatwg.org/#the-select-element-2
    KRYS_NODISCARD HTMLSlotElement *SlotForHostChild(const dom::Node &child) const noexcept;

    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept final
    {
      return _isDisabled;
    }

#pragma region HTMLOptGroupElement - https://html.spec.whatwg.org/#htmloptgroupelement

    /// @see https://html.spec.whatwg.org/#dom-optgroup-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-optgroup-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-optgroup-label
    KRYS_NODISCARD dom::DOMString Label() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-optgroup-label
    void Label(dom::DOMString value) noexcept;

#pragma endregion

    /// @brief The `label` attribute, with its white space stripped and collapsed.
    KRYS_NODISCARD dom::DOMString GroupLabelText() const noexcept;

    /// @brief The select whose list of options this optgroup is in, if any.
    KRYS_NODISCARD HTMLSelectElement *OwnerSelectElement() const noexcept
    {
      return _ownerSelect.get();
    }

    bool AccessKeyAction(bool sendMouseEvents) noexcept final;

  protected:
#pragma region Extension Hooks

    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnChildrenChanged() noexcept override;
    void DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept override;
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &child) const noexcept override;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

#pragma endregion

  private:
    void UpdateOwnerSelect() noexcept;
    void RecalcSelectOptions() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLOptGroupElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLOptGroupElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLOptGroupElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
