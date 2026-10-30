#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLFormElement;
  class HTMLSelectElement;

  enum class AllowStyleInvalidation : bool
  {
    No,
    Yes
  };

  /// @see https://html.spec.whatwg.org/#htmloptionelement
  class HTMLOptionElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLOptionElement);

  private:
    WeakPtr<HTMLSelectElement> _ownerSelect;
    bool _disabled {false};
    bool _isSelected {false};
    bool _isDefault {false};

    /// @see https://html.spec.whatwg.org/#concept-option-dirtiness
    bool _dirtiness {false};

  public:
    explicit HTMLOptionElement(dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option
    static dom::ExceptionOr<Ref<HTMLOptionElement>>
      CreateForLegacyFactoryFunction(dom::Document &document, dom::DOMString &&text,
                                     Maybe<dom::DOMString> value, bool defaultSelected,
                                     bool selected) noexcept;

    /// @see https://html.spec.whatwg.org/#concept-option-disabled
    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept final;

    KRYS_NODISCARD bool OwnElementDisabled() const noexcept
    {
      return _disabled;
    }

    KRYS_NODISCARD bool MatchesDefaultPseudoClass() const noexcept final
    {
      return _isDefault;
    }

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return false;
    }

#pragma region HTMLOptionElement - https://html.spec.whatwg.org/#htmloptionelement

    /// @see https://html.spec.whatwg.org/#dom-option-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-form
    KRYS_NODISCARD HTMLFormElement *Form() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-label
    KRYS_NODISCARD dom::DOMString Label() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-label
    void Label(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-defaultselected
    KRYS_NODISCARD bool DefaultSelected() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-defaultselected
    void DefaultSelected(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-selected
    KRYS_NODISCARD bool
      Selected(AllowStyleInvalidation allowStyleInvalidation = AllowStyleInvalidation::Yes) const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-selected
    void Selected(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-value
    KRYS_NODISCARD dom::DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-value
    void Value(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-text
    KRYS_NODISCARD dom::DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-text
    void Text(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-index
    KRYS_NODISCARD int32 Index() const noexcept;

#pragma endregion

    /// @brief The select whose list of options this option is in, if any.
    KRYS_NODISCARD HTMLSelectElement *OwnerSelectElement() const noexcept;

    /// @brief The label shown for the option: its text, ignoring its `label` attribute, in quirks mode.
    KRYS_NODISCARD dom::DOMString DisplayLabel() const noexcept;

    /// @brief The label, indented when the option is in an optgroup.
    KRYS_NODISCARD dom::DOMString TextIndentedToRespectGroupLabel() const noexcept;

    /// @brief Sets the option's selectedness, without making it dirty or telling its select.
    void
      SetSelectedState(bool selected,
                       AllowStyleInvalidation allowStyleInvalidation = AllowStyleInvalidation::Yes) noexcept;

    KRYS_NODISCARD bool SelectedWithoutUpdate() const noexcept
    {
      return _isSelected;
    }

    /// @brief Resets the option's dirtiness, as its select's reset algorithm does.
    void ResetDirtiness() noexcept
    {
      _dirtiness = false;
    }

    bool AccessKeyAction(bool sendMouseEvents) noexcept final;

  protected:
#pragma region Extension Hooks

    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnChildrenChanged() noexcept override;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

#pragma endregion

  private:
    void UpdateOwnerSelect() noexcept;

    KRYS_NODISCARD dom::DOMString CollectOptionInnerText() const noexcept;
    KRYS_NODISCARD dom::DOMString CollectOptionInnerTextCollapsingWhitespace() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLOptionElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLOptionElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLOptionElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
