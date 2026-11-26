#include "Krystal.Web/Accessibility/ARIAMixin.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Web/Html/CustomElement/ElementInternals.hpp"

namespace krys::web::ax::mixins
{
  template <typename TTarget>
  RefPtr<Element> ARIAMixin<TTarget>::AriaActiveDescendantElement() const noexcept
  {
    // The getter steps are to return the result of running this's get the attr-associated element.
    return ReflectedTarget().ElementForAttributeInternal(u8"aria-activedescendant");
  }

  template <typename TTarget>
  void ARIAMixin<TTarget>::AriaActiveDescendantElement(Element *element) noexcept
  {
    html::CEReactionsScope ceReactions;
    ReflectedTarget().SetElementAttribute(u8"aria-activedescendant", element);
  }

  template <typename TTarget>
  Maybe<DOMString> ARIAMixin<TTarget>::ReflectedString(DOMStringAtom name) const noexcept
  {
    return html::Reflect<Maybe<DOMString>>(ReflectedTarget(), krys::move(name));
  }

  template <typename TTarget>
  void ARIAMixin<TTarget>::SetReflectedString(DOMStringAtom name, Maybe<DOMString> value) noexcept
  {
    html::CEReactionsScope ceReactions;
    html::Reflect<Maybe<DOMString>>(ReflectedTarget(), krys::move(name), krys::move(value));
  }

  template <typename TTarget>
  Maybe<List<Ref<Element>>> ARIAMixin<TTarget>::AttrAssociatedElements(DOMStringAtom name) const noexcept
  {
    // 1. Let elements be the result of running this's get the attr-associated elements.
    // 2. If the contents of elements is equal to the contents of this's cached attr-associated elements, then
    //    return this's cached attr-associated elements object.
    // 3. Let elementsAsFrozenArray be elements, converted to a FrozenArray<T>?.
    // 4. Set this's cached attr-associated elements to elements.
    // 5. Set this's cached attr-associated elements object to elementsAsFrozenArray.
    // 6. Return elementsAsFrozenArray.
    // (Steps 2 to 5 keep the array object script sees the same; without script, the list is returned.)
    return ReflectedTarget().ElementsForAttributeInternal(krys::move(name));
  }

  template <typename TTarget>
  void ARIAMixin<TTarget>::SetAttrAssociatedElements(DOMStringAtom name,
                                                     const Maybe<List<Ref<Element>>> &elements) noexcept
  {
    html::CEReactionsScope ceReactions;
    ReflectedTarget().SetElementsArrayAttribute(krys::move(name), elements);
  }

  template class ARIAMixin<Element>;
  template class ARIAMixin<html::ElementInternals>;
}
