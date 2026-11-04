#include "Krystal.Booey/HTML/Elements/HTMLDetailsElement.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/CSS/UserAgentStyleSheets.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSummaryElement.hpp"
#include "Krystal.Booey/Platform/Locale/LocalizedStrings.hpp"

namespace krys::boo::html
{
  namespace
  {
    constexpr utf8_stringview SummarySlotName = u8"summarySlot";
  }

  HTMLDetailsElement::HTMLDetailsElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Details)
  {
  }

  HTMLDetailsElement::~HTMLDetailsElement() noexcept = default;

  Ref<HTMLDetailsElement> HTMLDetailsElement::Create(dom::Document &document) noexcept
  {
    auto details = CreateRef<HTMLDetailsElement>(document);
    details->EnsureUserAgentShadowRoot();
    return details;
  }

#pragma region HTMLDetailsElement

  dom::DOMString HTMLDetailsElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLDetailsElement::Name(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  bool HTMLDetailsElement::Open() const noexcept
  {
    return Reflect<bool>(*this, u8"open");
  }

  void HTMLDetailsElement::Open(bool value) noexcept
  {
    Reflect<bool>(*this, u8"open", krys::move(value));
  }

#pragma endregion

  void HTMLDetailsElement::DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept
  {
    auto &document = NodeDocument();

    auto summarySlot = dom::ElementFactory::CreateHTMLElement<HTMLSlotElement>(document, u8"slot");
    summarySlot->Name(dom::DOMString(SummarySlotName));

    auto defaultSummary = dom::ElementFactory::CreateHTMLElement<HTMLSummaryElement>(document, u8"summary");
    (void)defaultSummary->AppendChild(
      *CreateRef<dom::Text>(document, dom::DOMString(DefaultDetailsSummaryText())));
    _defaultSummary = WeakPtr<HTMLSummaryElement>::WithRef(*defaultSummary);

    (void)summarySlot->AppendChild(*defaultSummary);
    (void)root.AppendChild(*summarySlot);
    _summarySlot = WeakPtr<HTMLSlotElement>::WithRef(*summarySlot);

    auto defaultSlot = dom::ElementFactory::CreateHTMLElement<HTMLSlotElement>(document, u8"slot");
    defaultSlot->SetUserAgentPart(css::UserAgentParts::DetailsContent());
    krys_debug_assert(!HasAttribute(u8"open"));
    (void)defaultSlot->SetInlineStyleProperty(css::PropertyId::ContentVisibility, css::ValueId::Hidden);
    (void)defaultSlot->SetInlineStyleProperty(css::PropertyId::Display, css::ValueId::Block);
    (void)root.AppendChild(*defaultSlot);
    _defaultSlot = krys::move(defaultSlot);

    auto style =
      document.CreateElementNS(dom::DOMStringAtom(infra::Namespaces::HTML), dom::DOMStringAtom(u8"style"))
        .ReleaseValue();
    (void)style->TextContent(dom::DOMString(css::DetailsElementShadowUserAgentStyleSheet));
    (void)root.AppendChild(*style);
  }

  // https://html.spec.whatwg.org/#summary-for-its-parent-details
  bool HTMLDetailsElement::IsActiveSummary(const HTMLSummaryElement &summary) const noexcept
  {
    // A summary element is a summary for its parent details if: this summary element has a parent; that
    // parent is a details element; this summary element is the first summary element child of that details
    // element. (Or it's the default summary, when the details element has none.)
    auto *summarySlot = _summarySlot.get();
    if (!summarySlot)
    {
      return false;
    }
    if (!summarySlot->AssignedNodes())
    {
      return &summary == _defaultSummary.get();
    }

    if (summary.ParentNode() != this)
    {
      return false;
    }

    return dom::SlotAlgorithms::GetAssignedSlot(summary) == summarySlot;
  }

  HTMLSlotElement *HTMLDetailsElement::SlotForHostChild(const dom::Node &child) const noexcept
  {
    krys_debug_assert(child.ParentNode() == this);

    // The first summary child gets assigned to the summary slot.
    if (Is<HTMLSummaryElement>(child))
    {
      for (auto *element = dom::TreeQueries::FirstElementChild(*this); element;
           element = dom::TreeQueries::NextElementSibling(*element))
      {
        if (Is<HTMLSummaryElement>(*element))
        {
          if (element == &child)
          {
            return _summarySlot.get();
          }
          break;
        }
      }
    }
    return _defaultSlot.get();
  }

  void HTMLDetailsElement::SummaryChildChanged() noexcept
  {
    if (auto *root = UserAgentShadowRoot())
    {
      dom::SlotAlgorithms::AssignSlottablesForTree(*root);
    }
  }

  // https://html.spec.whatwg.org/#queue-a-details-toggle-event-task
  void HTMLDetailsElement::QueueDetailsToggleEventTask(dom::ToggleState oldState,
                                                       dom::ToggleState newState) noexcept
  {
    // (The task tracker is a ToggleEventTask, which coalesces the queued events.)
    if (!_toggleEventTask)
    {
      _toggleEventTask = dom::ToggleEventTask::Create(*this);
    }

    _toggleEventTask->Queue(oldState, newState);
  }

  void HTMLDetailsElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                              Maybe<dom::DOMStringView> oldValue,
                                              Maybe<dom::DOMStringView> value,
                                              dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    // https://html.spec.whatwg.org/#the-details-element:concept-element-attributes-change-ext
    // 1. If namespace is not null, then return.
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    // 3. If localName is open:
    if (localName == u8"open")
    {
      // 3a. If one of oldValue or value is null and the other is not null, run the following steps, which are
      //     known as the details notification task steps, for this details element:
      if (oldValue.has_value() != value.has_value())
      {
        krys_debug_assert(_defaultSlot);
        if (value)
        {
          (void)_defaultSlot->RemoveInlineStyleProperty(css::PropertyId::ContentVisibility);

          // 3a.i. If oldValue is null, queue a details toggle event task given the details element, "closed",
          //       and "open".
          QueueDetailsToggleEventTask(dom::ToggleState::Closed, dom::ToggleState::Open);

          // 3b. If oldValue is null and value is not null, then ensure details exclusivity by closing other
          //     elements if needed given element.
          // https://html.spec.whatwg.org/#ensure-details-exclusivity-by-closing-other-elements-if-needed
          // 1. Assert: element has an open attribute.
          // 2. If element does not have a name attribute, or its name attribute is the empty string, then
          //    return.
          // 3. Let groupMembers be a list of elements, containing all elements in element's details name
          // group
          //    except for element, in tree order.
          // 4. For each element otherElement of groupMembers:
          // 4a. If the open attribute is set on otherElement:
          // 4a.i. Assert: otherElement is the only element in groupMembers that has the open attribute set.
          // 4a.ii. Remove the open attribute on otherElement.
          // (Removing it from every member is the same, given the assertion.)
          if (!GetAttribute(u8"name").value_or(dom::DOMString()).empty())
          {
            for (auto &otherDetailsElement : OtherElementsInNameGroup())
            {
              (void)otherDetailsElement->RemoveAttribute(u8"open");
            }
          }
        }
        else
        {
          (void)_defaultSlot->SetInlineStyleProperty(css::PropertyId::ContentVisibility,
                                                     css::ValueId::Hidden);

          // 3a.ii. Otherwise, queue a details toggle event task given the details element, "open", and
          //        "closed".
          QueueDetailsToggleEventTask(dom::ToggleState::Open, dom::ToggleState::Closed);
        }
      }
    }
    // 2. If localName is name, then ensure details exclusivity by closing the given element if needed given
    //    element.
    else if (localName == u8"name")
    {
      EnsureDetailsExclusivityAfterMutation();
    }
  }

  void HTMLDetailsElement::OnPostConnection() noexcept
  {
    HTMLElement::OnPostConnection();

    // https://html.spec.whatwg.org/#the-details-element:html-element-insertion-steps
    // 1. Ensure details exclusivity by closing the given element if needed given insertedNode.
    // (Only once it's connected; the insertion steps can't change attributes.)
    EnsureDetailsExclusivityAfterMutation();
  }

  List<Ref<HTMLDetailsElement>> HTMLDetailsElement::OtherElementsInNameGroup() noexcept
  {
    List<Ref<HTMLDetailsElement>> otherElementsInNameGroup;
    auto detailsElementName = GetAttribute(u8"name");
    for (auto &descendant : dom::DescendantRange(dom::TreeQueries::Root(*this)))
    {
      auto *element = DynamicDowncast<HTMLDetailsElement>(descendant);
      if (element && element != this && element->GetAttribute(u8"name") == detailsElementName)
      {
        otherElementsInNameGroup.push_back(ShareRef(*element));
      }
    }
    return otherElementsInNameGroup;
  }

  // https://html.spec.whatwg.org/#ensure-details-exclusivity-by-closing-the-given-element-if-needed
  void HTMLDetailsElement::EnsureDetailsExclusivityAfterMutation() noexcept
  {
    // 1. If element does not have an open attribute, then return.
    // 2. If element does not have a name attribute, or its name attribute is the empty string, then return.
    if (HasAttribute(u8"open") && !GetAttribute(u8"name").value_or(dom::DOMString()).empty())
    {
      // 3. Let groupMembers be a list of elements, containing all elements in element's details name group
      //    except for element, in tree order.
      // 4. For each element otherElement of groupMembers:
      for (auto &otherDetailsElement : OtherElementsInNameGroup())
      {
        // 4a. If the open attribute is set on otherElement:
        // 4a.i. Remove the open attribute on element.
        // 4a.ii. Break.
        if (otherDetailsElement->HasAttribute(u8"open"))
        {
          ToggleOpen();
          break;
        }
      }
    }
  }

  void HTMLDetailsElement::ToggleOpen() noexcept
  {
    Open(!HasAttribute(u8"open"));
  }
}
