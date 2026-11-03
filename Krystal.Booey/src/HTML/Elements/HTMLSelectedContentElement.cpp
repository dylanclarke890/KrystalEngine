#include "Krystal.Booey/HTML/Elements/HTMLSelectedContentElement.hpp"
#include "Krystal.Booey/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::html
{
  HTMLSelectedContentElement::HTMLSelectedContentElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::SelectedContent)
  {
  }

#pragma region Extension Hooks

  // https://html.spec.whatwg.org/#the-selectedcontent-element:html-element-insertion-steps
  void HTMLSelectedContentElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();

    // Recalculate a selectedcontent element's disabledness given selectedcontent.
    (void)RecalculateDisabledness();
  }

  // https://html.spec.whatwg.org/#the-selectedcontent-element:html-element-post-connection-steps
  void HTMLSelectedContentElement::OnPostConnection() noexcept
  {
    HTMLElement::OnPostConnection();

    // 1. Let select be the result of recalculate a selectedcontent element's disabledness given
    // selectedcontent.
    RefPtr<HTMLSelectElement> select = ShareRefPtr(RecalculateDisabledness());

    // 2. If selectedcontent's disabled is true, select is null, or select has the multiple attribute, then
    //    return.
    if (_disabled || !select || select->HasAttribute(u8"multiple"))
    {
      return;
    }

    // 3. Update a selectedcontent given select and selectedcontent.
    FormControlAlgorithms::UpdateSelectedContent(*select, *this);
  }

  // https://html.spec.whatwg.org/#the-selectedcontent-element:html-element-moving-steps
  void HTMLSelectedContentElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);

    // 1. Let select be the result of recalculate a selectedcontent element's disabledness given movedNode.
    RefPtr<HTMLSelectElement> select = ShareRefPtr(RecalculateDisabledness());

    // 2. If movedNode's disabled is true, select is null, or select has the multiple attribute, then return.
    if (_disabled || !select || select->HasAttribute(u8"multiple"))
    {
      return;
    }

    // 3. Queue a microtask to update a selectedcontent given select and movedNode.
    EventLoop::Current().QueueMicrotask(
      [select, selectedcontent = ShareRef(*this)]
      { FormControlAlgorithms::UpdateSelectedContent(*select, *selectedcontent); });
  }

#pragma endregion

  HTMLSelectElement *HTMLSelectedContentElement::RecalculateDisabledness() noexcept
  {
    // 1. Let nearestSelectAncestor be null.
    HTMLSelectElement *nearestSelectAncestor = nullptr;

    // 2. Set selectedcontent's disabled to false.
    _disabled = false;

    // 3. For each ancestor of selectedcontent's ancestors, in reverse tree order:
    for (auto *ancestor = ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      // 3a. If ancestor is a select element:
      if (auto *select = DynamicDowncast<HTMLSelectElement>(*ancestor))
      {
        // 3a.i. If nearestSelectAncestor is null, then set nearestSelectAncestor to ancestor and continue.
        if (!nearestSelectAncestor)
        {
          nearestSelectAncestor = select;
          continue;
        }

        // 3a.ii. Set selectedcontent's disabled to true and break.
        _disabled = true;
        break;
      }

      // 3b. If ancestor is an option element or a selectedcontent element, then set selectedcontent's
      // disabled
      //     to true and break.
      if (Is<HTMLOptionElement>(*ancestor) || Is<HTMLSelectedContentElement>(*ancestor))
      {
        _disabled = true;
        break;
      }
    }

    // 4. Return nearestSelectAncestor.
    return nearestSelectAncestor;
  }
}