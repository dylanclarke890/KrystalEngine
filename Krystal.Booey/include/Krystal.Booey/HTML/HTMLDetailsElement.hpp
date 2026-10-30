#pragma once

#include "Krystal.Booey/DOM/ToggleEventTask.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
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
    explicit HTMLDetailsElement(dom::Document &document) noexcept;
    ~HTMLDetailsElement() noexcept override;

    KRYS_NODISCARD static Ref<HTMLDetailsElement> Create(dom::Document &document) noexcept;

#pragma region HTMLDetailsElement - https://html.spec.whatwg.org/#htmldetailselement

    /// @see https://html.spec.whatwg.org/#dom-details-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-details-name
    void Name(dom::DOMString &&value) noexcept;

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
    KRYS_NODISCARD HTMLSlotElement *SlotForHostChild(const dom::Node &child) const noexcept;

    /// @brief Reassigns the element's children to its slots after a `summary` child was inserted, removed or
    /// moved.
    void SummaryChildChanged() noexcept;

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return true;
    }

  protected:
#pragma region Extensibility Hooks

    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value, dom::DOMStringAtom namespaceURI) noexcept final;
    void OnPostConnection() noexcept final;

#pragma endregion

    void DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept final;

  private:
    /// @brief The other elements in the element's details name group.
    /// @see https://html.spec.whatwg.org/#details-name-group
    KRYS_NODISCARD List<Ref<HTMLDetailsElement>> OtherElementsInNameGroup() noexcept;

    /// @see https://html.spec.whatwg.org/#ensure-details-exclusivity-by-closing-the-given-element-if-needed
    void EnsureDetailsExclusivityAfterMutation() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLDetailsElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLDetailsElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLDetailsElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
