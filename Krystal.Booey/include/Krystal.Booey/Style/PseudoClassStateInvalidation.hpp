#pragma once

#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::style
{
  /// @brief Pseudo-class invalidations in progress: each invalidates before the change when it's added and
  /// after the change when the list is destroyed.
  using PseudoClassChangeInvalidations = List<UniquePtr<PseudoClassChangeInvalidation>>;

  /// @brief Starts the invalidations for the pseudo-class states that a change to one of @p element's
  /// attributes may flip. The selector checker derives those states (`:link`, `:checked`, `:lang()` and
  /// others) from attributes, and from those of the element's ancestors for some of them.
  /// @param oldValue The value before the change, or null if the attribute is being added.
  /// @param newValue The value after the change, or null if the attribute is being removed.
  void InvalidatePseudoClassesForAttributeChange(PseudoClassChangeInvalidations &invalidations,
                                                 dom::Element &element,
                                                 const dom::QualifiedName &attributeName,
                                                 Maybe<dom::DOMStringView> oldValue,
                                                 Maybe<dom::DOMStringView> newValue) noexcept;

  /// @brief Starts the invalidations for the pseudo-class states that inserting, removing or changing
  /// @p child (a child of @p parent) may flip elsewhere in the tree: a form's default button, the controls a
  /// disabled fieldset's first legend exempts and a text area's `:placeholder-shown`.
  void InvalidatePseudoClassesForChildChange(PseudoClassChangeInvalidations &invalidations,
                                             dom::ContainerNode &parent, dom::Node &child) noexcept;
}
