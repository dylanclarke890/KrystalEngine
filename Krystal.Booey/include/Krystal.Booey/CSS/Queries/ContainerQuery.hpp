#pragma once

#include "Krystal.Booey/CSS/Queries/ContainerQueryAxis.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css::cq
{
  /// @brief The axes a container must support size queries on for the feature to be evaluated.
  KRYS_NODISCARD Axis RequiredAxesForFeature(const mq::Feature &feature) noexcept;

  enum class ContainsUnknownFeature : bool
  {
    No,
    Yes
  };

  /// @brief The prelude of an `@container` rule: an optional container name and a condition.
  struct ContainerQuery
  {
    CSSOMStringAtom Name {CSSOMStringAtom::Null()};
    mq::Condition Condition;
    Axis RequiredAxes {Axis::None};
    cq::ContainsUnknownFeature ContainsUnknownFeature {cq::ContainsUnknownFeature::No};
  };

  void Serialise(StringBuilder &builder, const ContainerQuery &query) noexcept;
}
