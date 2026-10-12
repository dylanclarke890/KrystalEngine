#pragma once

#include "Krystal.Booey/CSS/Queries/ContainerQueryAxis.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::css
{
  class Value;
}

namespace krys::boo::css::cq
{
  namespace FeatureSchemas
  {
    const mq::FeatureSchema &Width();
    const mq::FeatureSchema &Height();
    const mq::FeatureSchema &InlineSize();
    const mq::FeatureSchema &BlockSize();
    const mq::FeatureSchema &AspectRatio();
    const mq::FeatureSchema &Orientation();
  };

  KRYS_NODISCARD Axis RequiredAxesForFeature(const mq::Feature &feature) noexcept;

  enum class ContainsUnknownFeature : bool
  {
    No,
    Yes
  };

  struct ContainerQuery
  {
    CSSOMStringAtom Name;
    mq::Condition Condition;
    cq::Axis RequiredAxes;
    ContainsUnknownFeature ContainsUnknownFeature;
  };

  void Serialise(CSSOMString &builder, const ContainerQuery &query) noexcept;
}
