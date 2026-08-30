#include "Krystal.HTML/CSS/Properties/Consumers/Motion.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Position.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Shapes.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParsing.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/Motion/CSSRayValue.hpp"
#include "Krystal.HTML/CSS/Values/Motion/RayFunction.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPosition.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  KRYS_NODISCARD static RefPtr<CSSValue> ConsumeRayFunction(CSSTokenRange &range,
                                                            CSSPropertyParserState &state) noexcept
  {
    // ray( <angle> && <ray-size>? && contain? && [at <position>]? )
    // <ray-size> = closest-side | closest-corner | farthest-side | farthest-corner | sides
    // https://drafts.fxtf.org/motion-1/#ray-function

    static constexpr auto sizeMappings = std::to_array<std::pair<CSSValueId, RaySize>>({
      {CSSValueId::ClosestSide, RaySize {Keywords::ClosestSide {}}},
      {CSSValueId::ClosestCorner, RaySize {Keywords::ClosestCorner {}}},
      {CSSValueId::FarthestSide, RaySize {Keywords::FarthestSide {}}},
      {CSSValueId::FarthestCorner, RaySize {Keywords::FarthestCorner {}}},
      {CSSValueId::Sides, RaySize {Keywords::Sides {}}},
    });

    if (range.Peek().Type() != CSSTokenType::Function || range.Peek().FunctionId() != CSSValueId::Ray)
    {
      return {};
    }

    auto args = ConsumeFunction(range);

    Maybe<Angle<>> angle;
    Maybe<RaySize> size;
    Maybe<Keywords::Contain> contain;
    Maybe<CSSPosition> position;

    auto ConsumeAngle = [&] -> bool
    {
      if (angle)
      {
        return false;
      }

      angle = MetaConsumer<Angle<>>::Consume(args, state);
      return angle.has_value();
    };

    auto ConsumeSize = [&] -> bool
    {
      if (size)
      {
        return false;
      }

      auto sizeValue = std::ranges::find_if(sizeMappings, [&](const auto &pair)
                                            { return pair.first == range.Peek().ValueId(); });
      if (sizeValue == sizeMappings.end())
      {
        return false;
      }

      size = sizeValue->second;
      return true;
    };

    auto ConsumeContain = [&] -> bool
    {
      if (contain || !ConsumeIdentRaw<CSSValueId::Contain>(args).has_value())
      {
        return false;
      }

      contain = Keywords::Contain {};
      return true;
    };

    auto ConsumeAtPosition = [&] -> bool
    {
      if (position || !ConsumeIdentRaw<CSSValueId::At>(args).has_value())
      {
        return false;
      }

      position = ConsumePositionUnresolved(args, state);
      return position.has_value();
    };

    while (!args.IsAtEnd())
    {
      if (ConsumeAngle() || ConsumeSize() || ConsumeContain() || ConsumeAtPosition())
      {
        continue;
      }

      return {};
    }

    // The <angle> argument is the only one that is required.
    if (!angle)
    {
      return {};
    }

    return CSSRayValue::Create(
      RayFunction {.parameters = Ray {Krys::Move(*angle), size.value_or(RaySize {Keywords::ClosestSide {}}),
                                      Krys::Move(contain), Krys::Move(position)}});
  }

  RefPtr<CSSValue> ConsumeOffsetPath(CSSTokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // <'offset-path'> = none | <offset-path> || <coord-box>
    //
    // NOTE: The sub-production, <offset-path> (without the quotation marks) is distinct and defined as:
    //    <offset-path> = <ray()> | <url> | <basic-shape>
    //
    // So, this expands out to a grammar of:
    //
    // <'offset-path'> = none | [ <ray()> | <url> | <basic-shape> || <coord-box> ]
    //
    // which is almost the same as <'clip-path'> above, with the following differences:
    //
    // 1. <'clip-path'> does not support `ray()`.
    // 2. <'clip-path'> does not allow a `box` to be provided with `<url>`.
    // 3. <'clip-path'> specifies `<geometry-box>` rather than `<coord-box>`.
    //
    // https://drafts.fxtf.org/motion-1/#propdef-offset-path

    if (range.Peek().ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(range);
    }

    // FIXME: It should be possible to consume both a <url> and <coord-box>.
    if (auto url = ConsumeURL(range, state, {}))
    {
      return url;
    }

    RefPtr<CSSValue> shapeOrRay;
    RefPtr<CSSValue> box;

    auto ConsumeRay = [&]() -> bool
    {
      if (shapeOrRay)
      {
        return false;
      }

      shapeOrRay = ConsumeRayFunction(range, state);
      return !!shapeOrRay;
    };

    auto ConsumeShape = [&]() -> bool
    {
      if (shapeOrRay)
      {
        return false;
      }

      shapeOrRay = ConsumeBasicShape(range, state, PathParsingOption::RejectPathFillRule);
      return !!shapeOrRay;
    };

    auto ConsumeBox = [&]() -> bool
    {
      if (box)
      {
        return false;
      }

      // FIXME: The Motion Path spec calls for this to be a <coord-box>, not a <geometry-box>, the difference
      // being that the former does not contain "margin-box" as a valid term. However, the spec also has a few
      // examples using "margin-box", so there seems to be some abiguity to be resolved. See:
      // https://github.com/w3c/fxtf-drafts/issues/481.
      box = CSSPropertyParsing::ConsumeGeometryBox(range);
      return !!box;
    };

    while (!range.IsAtEnd())
    {
      if (ConsumeRay() || ConsumeShape() || ConsumeBox())
      {
        continue;
      }

      break;
    }

    bool hasShapeOrRay = !!shapeOrRay;

    CSSValueListBuilder list;
    if (shapeOrRay)
    {
      list.push_back(Krys::Move(shapeOrRay));
    }

    // Default value is border-box.
    if (box && (box->ValueId() != CSSValueId::BorderBox || !hasShapeOrRay))
    {
      list.push_back(Krys::Move(box));
    }

    if (list.empty())
    {
      return nullptr;
    }

    return CSSValueList::CreateSpaceSeparated(Krys::Move(list));
  }
}