#include "Krystal.Booey/HTML/CustomStateSet.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"

namespace krys::boo::html
{
  CustomStateSet::CustomStateSet(dom::Element &element) noexcept : _element(CreateWeakPtr(&element))
  {
  }

  CustomStateSet::~CustomStateSet() noexcept = default;

  bool CustomStateSet::Add(dom::DOMStringAtom state) noexcept
  {
    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (auto *element = _element.get())
    {
      styleInvalidation.emplace(*element, css::SelectorPseudoClass::State,
                                style::PseudoClassChangeInvalidation::AnyValue);
    }

    return _states.add(state).second;
  }

  bool CustomStateSet::Delete(dom::DOMStringAtom state) noexcept
  {
    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (auto *element = _element.get())
    {
      styleInvalidation.emplace(*element, css::SelectorPseudoClass::State,
                                style::PseudoClassChangeInvalidation::AnyValue);
    }

    return _states.remove(state);
  }

  void CustomStateSet::Clear() noexcept
  {
    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (auto *element = _element.get())
    {
      styleInvalidation.emplace(*element, css::SelectorPseudoClass::State,
                                style::PseudoClassChangeInvalidation::AnyValue);
    }

    _states.clear();
  }

  bool CustomStateSet::Has(dom::DOMStringAtom state) const noexcept
  {
    return _states.contains(state);
  }

  size_t CustomStateSet::Size() const noexcept
  {
    return _states.size();
  }
}
