#include "Krystal.Booey/HTML/HTMLDetailsElement.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/CSS/UserAgentStyleSheets.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLSummaryElement.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"

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
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLDetailsElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  bool HTMLDetailsElement::Open() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"open");
  }

  void HTMLDetailsElement::Open(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"open", krys::move(value));
  }

#pragma endregion

  void HTMLDetailsElement::DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept
  {
    auto &document = NodeDocument();

    auto summarySlot = CreateRef<HTMLSlotElement>(document);
    summarySlot->Name(dom::DOMString(SummarySlotName));

    auto defaultSummary = CreateRef<HTMLSummaryElement>(document);
    (void)defaultSummary->AppendChild(
      *CreateRef<dom::Text>(document, dom::DOMString(DefaultDetailsSummaryText())));
    _defaultSummary = WeakPtr<HTMLSummaryElement>::WithRef(*defaultSummary);

    (void)summarySlot->AppendChild(*defaultSummary);
    (void)root.AppendChild(*summarySlot);
    _summarySlot = WeakPtr<HTMLSlotElement>::WithRef(*summarySlot);

    auto defaultSlot = CreateRef<HTMLSlotElement>(document);
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

  bool HTMLDetailsElement::IsActiveSummary(const HTMLSummaryElement &summary) const noexcept
  {
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

  void HTMLDetailsElement::QueueDetailsToggleEventTask(dom::ToggleState oldState,
                                                       dom::ToggleState newState) noexcept
  {
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
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"open")
    {
      if (oldValue.has_value() != value.has_value())
      {
        krys_debug_assert(_defaultSlot);
        if (value)
        {
          (void)_defaultSlot->RemoveInlineStyleProperty(css::PropertyId::ContentVisibility);
          QueueDetailsToggleEventTask(dom::ToggleState::Closed, dom::ToggleState::Open);
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
          QueueDetailsToggleEventTask(dom::ToggleState::Open, dom::ToggleState::Closed);
        }
      }
    }
    else if (localName == u8"name")
    {
      EnsureDetailsExclusivityAfterMutation();
    }
  }

  void HTMLDetailsElement::OnPostConnection() noexcept
  {
    HTMLElement::OnPostConnection();
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

  void HTMLDetailsElement::EnsureDetailsExclusivityAfterMutation() noexcept
  {
    if (HasAttribute(u8"open") && !GetAttribute(u8"name").value_or(dom::DOMString()).empty())
    {
      for (auto &otherDetailsElement : OtherElementsInNameGroup())
      {
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
