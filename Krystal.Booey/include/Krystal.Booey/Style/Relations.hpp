#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  class Update;

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

  /// @brief Records the relations of @p element that its own style holds (`:empty`, `:first-child`,
  /// `:last-child`) on @p style.
  /// @return The relations left to commit to the elements once the style is applied, or null if there are
  /// none.
  KRYS_NODISCARD UniquePtr<Relations> CommitRelationsToRenderStyle(RenderStyle &style,
                                                                   const dom::Element &element,
                                                                   const Relations &relations) noexcept;

  /// @brief Copies the relation state (`:empty`, `:first-child`, `:last-child`) of @p from to @p to.
  void CopyRelations(RenderStyle &to, const RenderStyle &from) noexcept;

  /// @brief Sets the style flags of the elements @p relations are about, and the relation state of their new
  /// styles in @p update.
  void CommitRelations(UniquePtr<Relations> relations, Update &update) noexcept;
}
