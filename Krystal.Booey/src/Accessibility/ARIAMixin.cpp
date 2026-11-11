#include "Krystal.Booey/Accessibility/ARIAMixin.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/ElementInternals.hpp"

namespace krys::boo::aria::mixins
{
  template <typename TTarget>
  RefPtr<dom::Element> ARIAMixin<TTarget>::AriaActiveDescendantElement() const noexcept
  {
    // The getter steps are to return the result of running this's get the attr-associated element.
    return ReflectedTarget().ElementForAttributeInternal(dom::DOMStringAtom(u8"aria-activedescendant"));
  }

  template <typename TTarget>
  void ARIAMixin<TTarget>::AriaActiveDescendantElement(dom::Element *element) noexcept
  {
    html::CEReactionsScope ceReactions;
    ReflectedTarget().SetElementAttribute(dom::DOMStringAtom(u8"aria-activedescendant"), element);
  }

  template <typename TTarget>
  Maybe<dom::DOMString> ARIAMixin<TTarget>::ReflectedString(dom::DOMStringAtom name) const noexcept
  {
    return html::Reflect<Maybe<dom::DOMString>>(ReflectedTarget(), name);
  }

  template <typename TTarget>
  void ARIAMixin<TTarget>::SetReflectedString(dom::DOMStringAtom name, Maybe<dom::DOMString> value) noexcept
  {
    html::CEReactionsScope ceReactions;
    html::Reflect<Maybe<dom::DOMString>>(ReflectedTarget(), name, krys::move(value));
  }

  template <typename TTarget>
  Maybe<List<Ref<dom::Element>>>
    ARIAMixin<TTarget>::AttrAssociatedElements(dom::DOMStringAtom name) const noexcept
  {
    // 1. Let elements be the result of running this's get the attr-associated elements.
    // 2. If the contents of elements is equal to the contents of this's cached attr-associated elements, then
    //    return this's cached attr-associated elements object.
    // 3. Let elementsAsFrozenArray be elements, converted to a FrozenArray<T>?.
    // 4. Set this's cached attr-associated elements to elements.
    // 5. Set this's cached attr-associated elements object to elementsAsFrozenArray.
    // 6. Return elementsAsFrozenArray.
    // (Steps 2 to 5 keep the array object script sees the same; without script, the list is returned.)
    return ReflectedTarget().ElementsForAttributeInternal(name);
  }

  template <typename TTarget>
  void ARIAMixin<TTarget>::SetAttrAssociatedElements(dom::DOMStringAtom name,
                                                     const Maybe<List<Ref<dom::Element>>> &elements) noexcept
  {
    html::CEReactionsScope ceReactions;
    ReflectedTarget().SetElementsArrayAttribute(name, elements);
  }

  template class ARIAMixin<dom::Element>;
  template class ARIAMixin<html::ElementInternals>;
}
