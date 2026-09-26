#pragma once

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

  enum class Axis : uint8
  {
    Block = 1 << 0,
    Inline = 1 << 1,
    Width = 1 << 2,
    Height = 1 << 3,
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

  void Serialize(CSSOMString &builder, const ContainerQuery &query) noexcept;
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::cq::Axis, 4uz);
}