#include "Krystal.Booey/DOM/FindRevealAlgorithms.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/HTMLDetailsElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  namespace
  {
    enum class RevealType : bool
    {
      ClosedDetails,
      HiddenUntilFound
    };

    KRYS_NODISCARD RefPtr<html::HTMLDetailsElement> ClosedDetailsElementAncestor(const Node &node) noexcept
    {
      auto *slot = SlotAlgorithms::GetAssignedSlot(node);
      if (slot && slot->UserAgentPart() == css::UserAgentParts::DetailsContent())
      {
        if (auto *details = DynamicDowncast<html::HTMLDetailsElement>(TreeQueries::ShadowHost(*slot)))
        {
          if (!details->HasAttribute(u8"open"))
          {
            return ShareRefPtr(details);
          }
        }
      }
      return nullptr;
    }
  }

  bool RevealClosedDetailsAndHiddenUntilFoundAncestors(Node &node) noexcept
  {
    (void)node.NodeDocument().UpdateStyleIfNeeded();

    // Bail out if there is neither a hidden=until-found or details ancestor.
    if (auto *style = node.GetRenderStyle(); style && !style->AutoRevealsWhenFound())
    {
      return false;
    }

    List<std::pair<Ref<html::HTMLElement>, RevealType>> ancestors;
    for (Node *ancestor = &node; ancestor->ParentElementInComposedTree();
         ancestor = ancestor->ParentElementInComposedTree())
    {
      if (auto *element = DynamicDowncast<html::HTMLElement>(*ancestor);
          element && element->IsHiddenUntilFound())
      {
        ancestors.emplace_back(ShareRef(*element), RevealType::HiddenUntilFound);
      }
      if (auto details = ClosedDetailsElementAncestor(*ancestor))
      {
        ancestors.emplace_back(ShareRef<html::HTMLElement>(*details), RevealType::ClosedDetails);
      }
    }

    auto revealed = false;
    for (auto &[element, revealType] : ancestors)
    {
      if (!element->IsConnected())
      {
        return revealed;
      }
      switch (revealType)
      {
        case RevealType::ClosedDetails:
          if (element->HasAttribute(u8"open"))
          {
            return revealed;
          }
          Downcast<html::HTMLDetailsElement>(*element).Open(true);
          revealed = true;
          break;
        case RevealType::HiddenUntilFound:
          if (!element->IsHiddenUntilFound())
          {
            return revealed;
          }
          {
            auto event = Event::Create(DOMStringAtom(EventNames::BeforeMatch), true, false);
            (void)element->DispatchEvent(*event);
          }
          if (!element->IsConnected() || !element->IsHiddenUntilFound())
          {
            return revealed;
          }
          element->Hidden(false);
          revealed = true;
          break;
      }
    }
    return revealed;
  }
}
