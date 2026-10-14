#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  /// @brief A dependency of an element's style on its tree position, recorded while matching selectors.
  struct Relation
  {
    enum Type : uint8
    {
      AffectedByEmpty,
      AffectedByPreviousSibling,
      DescendantsAffectedByPreviousSibling,
      // For AffectsNextSibling 'value' tells how many element siblings to mark starting with 'element'.
      AffectsNextSibling,
      ChildrenAffectedByForwardPositionalRules,
      DescendantsAffectedByForwardPositionalRules,
      ChildrenAffectedByBackwardPositionalRules,
      DescendantsAffectedByBackwardPositionalRules,
      ChildrenAffectedByFirstChildRules,
      ChildrenAffectedByLastChildRules,
      FirstChild,
      LastChild,
      NthChildIndex,
      AffectedByHasWithPositionalPseudoClass,
    };

    const dom::Element *Element;
    Type RelationType;
    uint32 Value;

    Relation(const dom::Element &element, Type type, uint32 value = 1) noexcept
        : Element(&element), RelationType(type), Value(value)
    {
    }
  };

  using Relations = SmallList<Relation, 8>;

  // FIXME: commitRelationsToRenderStyle, copyRelations and commitRelations come with the style resolver, once
  // RenderStyle and the element style flags exist.
}
