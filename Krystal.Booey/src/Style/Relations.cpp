#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Update.hpp"

namespace krys::boo::style
{
  UniquePtr<Relations> CommitRelationsToRenderStyle(RenderStyle &style, const dom::Element &element,
                                                    const Relations &relations) noexcept
  {
    UniquePtr<Relations> remainingRelations;

    auto appendStyleRelation = [&remainingRelations](const Relation &relation)
    {
      if (!remainingRelations)
      {
        remainingRelations = CreateUnique<Relations>();
      }
      remainingRelations->push_back(relation);
    };

    for (auto &relation : relations)
    {
      if (relation.Element != &element)
      {
        appendStyleRelation(relation);
        continue;
      }
      switch (relation.RelationType)
      {
        case Relation::AffectedByEmpty:
        {
          style.SetEmptyState(relation.Value);
          appendStyleRelation(relation);
          break;
        }
        case Relation::FirstChild:
        {
          style.SetFirstChildState();
          break;
        }
        case Relation::LastChild:
        {
          style.SetLastChildState();
          break;
        }
        case Relation::AffectedByPreviousSibling:
        case Relation::DescendantsAffectedByPreviousSibling:
        case Relation::AffectsNextSibling:
        case Relation::ChildrenAffectedByForwardPositionalRules:
        case Relation::DescendantsAffectedByForwardPositionalRules:
        case Relation::ChildrenAffectedByBackwardPositionalRules:
        case Relation::DescendantsAffectedByBackwardPositionalRules:
        case Relation::ChildrenAffectedByFirstChildRules:
        case Relation::ChildrenAffectedByLastChildRules:
        case Relation::NthChildIndex:
        case Relation::AffectedByHasWithPositionalPseudoClass:
        {
          appendStyleRelation(relation);
          break;
        }
      }
    }
    return remainingRelations;
  }

  void CommitRelations(UniquePtr<Relations> relations, Update &update) noexcept
  {
    if (!relations)
    {
      return;
    }
    for (auto &relation : *relations)
    {
      auto &element = const_cast<dom::Element &>(*relation.Element);
      switch (relation.RelationType)
      {
        case Relation::AffectedByEmpty:           element.SetStyleAffectedByEmpty(); break;
        case Relation::AffectedByPreviousSibling: element.SetStyleIsAffectedByPreviousSibling(); break;
        case Relation::DescendantsAffectedByPreviousSibling:
          element.SetDescendantsAffectedByPreviousSibling();
          break;
        case Relation::AffectsNextSibling:
        {
          auto sibling = ShareRefPtr(&element);
          for (uint32 i = 0; i < relation.Value && sibling; ++i, sibling = sibling->NextElementSibling())
          {
            sibling->SetAffectsNextSiblingElementStyle();
          }
          break;
        }
        case Relation::ChildrenAffectedByForwardPositionalRules:
          element.SetChildrenAffectedByForwardPositionalRules();
          break;
        case Relation::DescendantsAffectedByForwardPositionalRules:
          element.SetDescendantsAffectedByForwardPositionalRules();
          break;
        case Relation::ChildrenAffectedByBackwardPositionalRules:
          element.SetChildrenAffectedByBackwardPositionalRules();
          break;
        case Relation::DescendantsAffectedByBackwardPositionalRules:
          element.SetDescendantsAffectedByBackwardPositionalRules();
          break;
        case Relation::ChildrenAffectedByFirstChildRules:
          element.SetChildrenAffectedByFirstChildRules();
          break;
        case Relation::ChildrenAffectedByLastChildRules: element.SetChildrenAffectedByLastChildRules(); break;
        case Relation::AffectedByHasWithPositionalPseudoClass:
          element.SetAffectedByHasWithPositionalPseudoClass();
          break;
        case Relation::FirstChild:
          if (auto *style = update.ElementStyle(element))
          {
            style->SetFirstChildState();
          }
          break;
        case Relation::LastChild:
          if (auto *style = update.ElementStyle(element))
          {
            style->SetLastChildState();
          }
          break;
        case Relation::NthChildIndex: element.SetChildIndex(relation.Value); break;
      }
    }
  }

  void CopyRelations(RenderStyle &to, const RenderStyle &from) noexcept
  {
    if (from.EmptyState())
    {
      to.SetEmptyState(true);
    }
    if (from.FirstChildState())
    {
      to.SetFirstChildState();
    }
    if (from.LastChildState())
    {
      to.SetLastChildState();
    }
  }
}
