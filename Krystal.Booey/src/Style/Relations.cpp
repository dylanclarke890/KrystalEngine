#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

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
