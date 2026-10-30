#include "Krystal.Booey/HTML/FormAssociatedElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"

namespace krys::boo::html
{
  FormAssociatedElement::~FormAssociatedElement() noexcept
  {
    krys_assert(!_form);
  }

  HTMLFormElement *FormAssociatedElement::Form() const noexcept
  {
    return _form.get();
  }

  void FormAssociatedElement::SetForm(HTMLFormElement *newForm) noexcept
  {
    if (_form.get() != newForm)
    {
      SetFormInternal(newForm);
    }
  }

  RefPtr<HTMLFormElement> FormAssociatedElement::FormForBindings() const noexcept
  {
    return ShareRefPtr(Form());
  }

  void FormAssociatedElement::SetFormSetByParser(HTMLFormElement *form) noexcept
  {
    _formSetByParser = CreateWeakPtr(form);
  }

  void FormAssociatedElement::SetFormInternal(HTMLFormElement *newForm) noexcept
  {
    krys_debug_assert(_form.get() != newForm);
    _form = CreateWeakPtr(newForm);
  }

  void FormAssociatedElement::ElementInsertedIntoAncestor(dom::Element &element, bool) noexcept
  {
    krys_debug_assert(&AsHTMLElement() == &element);
    if (_formSetByParser)
    {
      // The form could have been removed by a script during parsing.
      if (_formSetByParser->IsConnected())
      {
        SetForm(_formSetByParser.get());
      }
      _formSetByParser = nullptr;
    }

    if (_form && &dom::TreeQueries::Root(element) != &dom::TreeQueries::Root(*_form))
    {
      SetForm(nullptr);
    }
  }

  void FormAssociatedElement::ElementRemovedFromAncestor(dom::Element &element, bool) noexcept
  {
    krys_debug_assert(&AsHTMLElement() == &element);
    if (_form && &dom::TreeQueries::Root(element) != &dom::TreeQueries::Root(*_form))
    {
      SetForm(nullptr);
      ResetFormOwner();
    }
  }
}
