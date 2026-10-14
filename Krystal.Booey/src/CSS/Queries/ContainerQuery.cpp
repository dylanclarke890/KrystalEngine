#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQueryFeatures.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQuerySerialisation.hpp"

namespace krys::boo::css::cq
{
  Axis RequiredAxesForFeature(const mq::Feature &feature) noexcept
  {
    if (feature.Schema == &Features::Width())
    {
      return Axis::Width;
    }
    if (feature.Schema == &Features::Height())
    {
      return Axis::Height;
    }
    if (feature.Schema == &Features::InlineSize())
    {
      return Axis::Inline;
    }
    if (feature.Schema == &Features::BlockSize())
    {
      return Axis::Block;
    }
    if (feature.Schema == &Features::AspectRatio() || feature.Schema == &Features::Orientation())
    {
      return Axis::Inline | Axis::Block;
    }
    return Axis::None;
  }

  void Serialise(CSSOMString &builder, const ContainerQuery &query) noexcept
  {
    if (query.Name != CSSOMStringAtom::Null() && query.Name != CSSOMStringAtom::Empty())
    {
      SerialiseIdentifier(CSSOMString(query.Name.View()), builder);
      builder += u8' ';
    }

    mq::Serialise(builder, query.Condition);
  }
}
