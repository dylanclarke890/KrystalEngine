#include "Krystal.Booey/HTML/Forms/FormListedElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Internals/IdTargetObserver.hpp"
#include "Krystal.Booey/DOM/Internals/IdTargetObserverRegistry.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::html
{
  /// @brief Resets its element's form owner when the element its `form` attribute names changes.
  class FormAttributeTargetObserver final : private dom::IdTargetObserver
  {
  private:
    FormListedElement &_element;

  public:
    FormAttributeTargetObserver(DOMStringAtom id, FormListedElement &element) noexcept;

  private:
    void IdTargetChanged(boo::Element &) noexcept override;
  };

  namespace
  {
    /// @brief The element with the ID @p id in @p element's tree.
    KRYS_NODISCARD boo::Element *ElementForId(const HTMLElement &element, DOMStringView id) noexcept
    {
      auto &root = dom::TreeQueries::Root(const_cast<HTMLElement &>(element));
      if (auto *document = DynamicDowncast<Document>(root))
      {
        return document->GetElementById(id).get();
      }
      if (auto *fragment = DynamicDowncast<DocumentFragment>(root))
      {
        return fragment->GetElementById(id).get();
      }
      return nullptr;
    }

    KRYS_NODISCARD HTMLFormElement *FindAssociatedForm(const HTMLElement &element,
                                                       HTMLFormElement *currentAssociatedForm) noexcept
    {
      if (element.IsConnected())
      {
        if (auto formId = element.GetAttribute(u8"form"))
        {
          // Let candidate be element's resolved form target element: the first element in its tree with the
          // attribute's ID, or the element its shadow root's reference target names. If candidate is a form
          // element, then set newForm to candidate.
          RefPtr<boo::Element> candidate = ShareRefPtr(ElementForId(element, *formId));
          if (candidate)
          {
            candidate = candidate->ResolveReferenceTarget();
          }
          return DynamicDowncast<HTMLFormElement>(candidate.get());
        }
      }

      return currentAssociatedForm ? currentAssociatedForm
                                   : HTMLFormElement::FindClosestFormAncestor(element);
    }
  }

  FormListedElement::FormListedElement() noexcept = default;

  FormListedElement::~FormListedElement() noexcept = default;

  void FormListedElement::DidMoveToNewDocument() noexcept
  {
    auto &element = AsHTMLElement();
    if (element.HasAttribute(u8"form") && element.IsConnected())
    {
      ResetFormAttributeTargetObserver();
    }
  }

  void FormListedElement::ElementInsertedIntoAncestor(boo::Element &element,
                                                      bool connectedToDocument) noexcept
  {
    FormAssociatedElement::ElementInsertedIntoAncestor(element, connectedToDocument);
    if (!connectedToDocument)
    {
      return;
    }

    if (element.HasAttribute(u8"form"))
    {
      ResetFormAttributeTargetObserver();
    }
  }

  void FormListedElement::ElementRemovedFromAncestor(boo::Element &element,
                                                     bool disconnectedFromDocument) noexcept
  {
    krys_debug_assert(&AsHTMLElement() == &element);
    _formAttributeTargetObserver = nullptr;
    FormAssociatedElement::ElementRemovedFromAncestor(element, disconnectedFromDocument);
    if (disconnectedFromDocument && element.HasAttribute(u8"form"))
    {
      SetForm(nullptr);
      ResetFormOwner();
    }
  }

  void FormListedElement::FormOwnerRemovedFromTree(const Node &formRoot) noexcept
  {
    krys_debug_assert(Form());

    auto formHasSameRootNode = [&]() -> Maybe<bool>
    {
      const Node *rootNode = &AsHTMLElement();
      if (auto *currentForm = Form())
      {
        for (auto *ancestor = rootNode->ParentNode(); ancestor; ancestor = ancestor->ParentNode())
        {
          if (ancestor == currentForm)
          {
            return std::nullopt;
          }
          rootNode = ancestor;
        }
      }
      return rootNode == &formRoot;
    }();

    if (!formHasSameRootNode)
    {
      // Form is our ancestor so we don't need to reset our owner, we also no longer need an id observer since
      // we are no longer connected.
      _formAttributeTargetObserver = nullptr;
      return;
    }

    // An element that left its form owner's tree loses it.
    if (!*formHasSameRootNode)
    {
      SetForm(nullptr);
    }
  }

  void FormListedElement::SetFormInternal(HTMLFormElement *newForm) noexcept
  {
    WillChangeForm();
    if (auto *oldForm = Form())
    {
      oldForm->UnregisterFormListedElement(*this);
    }
    FormAssociatedElement::SetFormInternal(newForm);
    if (newForm)
    {
      newForm->RegisterFormListedElement(*this);
    }
    DidChangeForm();
  }

  void FormListedElement::WillChangeForm() noexcept
  {
  }

  void FormListedElement::DidChangeForm() noexcept
  {
  }

  void FormListedElement::FormWillBeDestroyed() noexcept
  {
    krys_debug_assert(Form());
    if (!Form())
    {
      return;
    }

    WillChangeForm();
    FormAssociatedElement::FormWillBeDestroyed();
    DidChangeForm();
  }

  void FormListedElement::ResetFormOwner() noexcept
  {
    // FIXME: Telling the host about newly associated controls (Document::didAssociateFormControl), for
    // autofill.
    SetForm(FindAssociatedForm(AsHTMLElement(), Form()));
  }

  void FormListedElement::ParseAttribute(const DOMStringAtom &localName, Maybe<DOMStringView> value) noexcept
  {
    if (localName == u8"form")
    {
      ParseFormAttribute(value);
    }
  }

  void FormListedElement::ParseFormAttribute(Maybe<DOMStringView> value) noexcept
  {
    auto &element = AsHTMLElement();
    if (!value)
    {
      // Without a form attribute, the form owner is the nearest form ancestor; the old owner, which the
      // attribute chose, isn't kept.
      SetForm(HTMLFormElement::FindClosestFormAncestor(element));
      _formAttributeTargetObserver = nullptr;
    }
    else
    {
      ResetFormOwner();
      if (element.IsConnected())
      {
        ResetFormAttributeTargetObserver();
      }
    }
  }

  bool FormListedElement::CustomError() const noexcept
  {
    return !_customValidationMessage.empty();
  }

  bool FormListedElement::HasBadInput() const noexcept
  {
    return false;
  }

  bool FormListedElement::PatternMismatch() const noexcept
  {
    return false;
  }

  bool FormListedElement::RangeOverflow() const noexcept
  {
    return false;
  }

  bool FormListedElement::RangeUnderflow() const noexcept
  {
    return false;
  }

  bool FormListedElement::StepMismatch() const noexcept
  {
    return false;
  }

  bool FormListedElement::TooShort() const noexcept
  {
    return false;
  }

  bool FormListedElement::TooLong() const noexcept
  {
    return false;
  }

  bool FormListedElement::TypeMismatch() const noexcept
  {
    return false;
  }

  bool FormListedElement::ComputeValidity() const noexcept
  {
    bool someError = TypeMismatch() || StepMismatch() || RangeUnderflow() || RangeOverflow() || TooShort()
                     || TooLong() || PatternMismatch() || ValueMissing() || HasBadInput() || CustomError();
    return !someError;
  }

  bool FormListedElement::ValueMissing() const noexcept
  {
    return false;
  }

  DOMString FormListedElement::CustomValidationMessage() const noexcept
  {
    return _customValidationMessage;
  }

  DOMString FormListedElement::ValidationMessage() const noexcept
  {
    return WillValidate() ? _customValidationMessage : DOMString();
  }

  // https://html.spec.whatwg.org/#dom-cva-setcustomvalidity
  void FormListedElement::SetCustomValidity(DOMStringView error) noexcept
  {
    // 1. Set error to the result of normalizing newlines given error.
    // 2. Set the custom validity error message to error.
    SetCustomValidationMessage(infra::StringAlgorithms::NormalizeNewlines(error));
  }

  void FormListedElement::SetCustomValidationMessage(DOMStringView message) noexcept
  {
    _customValidationMessage = DOMString(message);
  }

  void FormListedElement::ResetFormAttributeTargetObserver() noexcept
  {
    auto &element = AsHTMLElement();
    krys_assert(element.IsConnected());
    _formAttributeTargetObserver = CreateUnique<FormAttributeTargetObserver>(
      DOMStringAtom(element.GetAttribute(u8"form").value_or(DOMString())), *this);
  }

  void FormListedElement::FormAttributeTargetChanged() noexcept
  {
    ResetFormOwner();
  }

  DOMString FormListedElement::Name() const noexcept
  {
    return AsHTMLElement().GetAttribute(u8"name").value_or(DOMString());
  }

  FormAttributeTargetObserver::FormAttributeTargetObserver(DOMStringAtom id,
                                                           FormListedElement &element) noexcept
      : IdTargetObserver(element.AsHTMLElement().TreeScope().IdTargetObserverRegistry(), krys::move(id)),
        _element(element)
  {
  }

  void FormAttributeTargetObserver::IdTargetChanged(boo::Element &) noexcept
  {
    _element.FormAttributeTargetChanged();
  }
}
