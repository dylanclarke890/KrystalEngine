#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterRenderer.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterEffectGraph.hpp"
#include "Krystal.Booey/SVG/Graphics/Filters/SVGFilterPrimitiveGraph.hpp"
#include "Krystal.Booey/SVG/SVGFilterElement.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"
#include <numbers>

namespace krys::boo
{
  namespace
  {
    constexpr uint32 MaxCountChildNodes = 200;

    using FilterEffectGeometryMap = HashMap<Ref<FilterEffect>, FilterEffectGeometry>;

    struct BuiltFilterEffectGraph
    {
      SVGFilterEffectGraph Graph;
      FilterEffectGeometryMap EffectGeometryMap;
    };

    template <typename TCallback>
    void ForEachFilterPrimitive(svg::SVGFilterElement &filterElement, const TCallback &callback) noexcept
    {
      for (auto *child = filterElement.FirstChild(); child; child = child->NextSibling())
      {
        if (auto *effectElement = DynamicDowncast<svg::SVGFilterPrimitiveStandardAttributes>(*child))
        {
          if (!callback(*effectElement))
          {
            return;
          }
        }
      }
    }

    KRYS_NODISCARD Maybe<BuiltFilterEffectGraph>
      BuildFilterEffectGraph(svg::SVGElement *contextElement, svg::SVGFilterElement &filterElement,
                             const SVGFilterRenderer &filter,
                             const GraphicsContext &destinationContext) noexcept
    {
      if (filterElement.CountChildNodes() > MaxCountChildNodes)
      {
        return null;
      }

      BuiltFilterEffectGraph result {SVGFilterEffectGraph(SourceGraphic::Create(), SourceAlpha::Create()),
                                     {}};
      auto &graph = result.Graph;
      bool failed = false;

      ForEachFilterPrimitive(
        filterElement,
        [&](svg::SVGFilterPrimitiveStandardAttributes &effectElement)
        {
          auto inputNames = effectElement.FilterEffectInputsNames();
          auto inputs = graph.GetNamedNodes(inputNames);
          if (!inputs)
          {
            failed = true;
            return false;
          }

          auto effect = effectElement.GetFilterEffect(*inputs, destinationContext);
          if (!effect)
          {
            failed = true;
            return false;
          }

          if (auto flags = effectElement.EffectGeometryFlags(); !flags.IsEmpty())
          {
            auto effectBoundaries = svg::SVGLengthContext::ResolveRectangle(
              contextElement, effectElement, filter.PrimitiveUnits(), filter.ReferenceBox());
            result.EffectGeometryMap.insert_or_assign(effect, FilterEffectGeometry(effectBoundaries, flags));
          }

          if (effectElement.GetColorInterpolation() == ColorInterpolation::LinearRGB)
          {
            effect->SetOperatingColorSpace(DestinationColorSpace::LinearSRGB());
          }

          graph.AddNamedNode(dom::DOMStringAtom(effectElement.Result()), Ref<FilterEffect>(effect));
          graph.SetNodeInputs(*effect, krys::move(*inputs));
          return true;
        });

      if (failed)
      {
        return null;
      }

      return result;
    }

    KRYS_NODISCARD Maybe<SVGFilterPrimitiveGraph>
      BuildFilterPrimitiveGraph(svg::SVGFilterElement &filterElement) noexcept
    {
      auto countChildNodes = filterElement.CountChildNodes();
      if (!countChildNodes || countChildNodes > MaxCountChildNodes)
      {
        return null;
      }

      SVGFilterPrimitiveGraph graph;

      ForEachFilterPrimitive(
        filterElement,
        [&](svg::SVGFilterPrimitiveStandardAttributes &effectElement)
        {
          // We should not be strict about not finding the input primitives here because SourceGraphic and
          // SourceAlpha do not have primitives.
          auto inputNames = effectElement.FilterEffectInputsNames();
          auto inputs = graph.GetNamedNodes(inputNames).value_or(SVGFilterPrimitiveGraph::NodeVector());
          graph.AddNamedNode(dom::DOMStringAtom(effectElement.Result()), ShareRef(effectElement));
          graph.SetNodeInputs(effectElement, krys::move(inputs));
          return true;
        });

      return graph;
    }
  }

  RefPtr<SVGFilterRenderer> SVGFilterRenderer::Create(svg::SVGElement *contextElement,
                                                      svg::SVGFilterElement &filterElement,
                                                      const FilterGeometry &geometry,
                                                      EnumSet<FilterRenderingMode> preferredRenderingModes,
                                                      const GraphicsContext &destinationContext) noexcept
  {
    auto filter = AdoptRef(*new SVGFilterRenderer(geometry, filterElement.PrimitiveUnits()));

    auto result = BuildExpression(contextElement, filterElement, *filter, destinationContext);
    if (!result)
    {
      return nullptr;
    }

    krys_debug_assert(!result->Expression.empty());
    krys_debug_assert(!result->Effects.empty());
    filter->SetExpression(krys::move(result->Expression));
    filter->SetEffects(krys::move(result->Effects));

    filter->SetFilterRenderingModes(preferredRenderingModes);
    return filter;
  }

  Ref<SVGFilterRenderer>
    SVGFilterRenderer::Create(svg::SVGUnitTypes::SVGUnitType primitiveUnits, SVGFilterExpression &&expression,
                              FilterEffectVector &&effects, const FilterGeometry &geometry,
                              EnumSet<FilterRenderingMode> preferredRenderingModes) noexcept
  {
    auto filter =
      AdoptRef(*new SVGFilterRenderer(geometry, primitiveUnits, krys::move(expression), krys::move(effects)));
    // Setting filter rendering modes cannot be moved to the constructor because it ends up
    // calling supportedFilterRenderingModes() which is a virtual function.
    filter->SetFilterRenderingModes(preferredRenderingModes);
    return filter;
  }

  SVGFilterRenderer::SVGFilterRenderer(const FilterGeometry &geometry,
                                       svg::SVGUnitTypes::SVGUnitType primitiveUnits) noexcept
      : Filter(Type::SVGFilterRenderer, geometry), _primitiveUnits(primitiveUnits)
  {
  }

  SVGFilterRenderer::SVGFilterRenderer(const FilterGeometry &geometry,
                                       svg::SVGUnitTypes::SVGUnitType primitiveUnits,
                                       SVGFilterExpression &&expression,
                                       FilterEffectVector &&effects) noexcept
      : Filter(Type::SVGFilterRenderer, geometry), _primitiveUnits(primitiveUnits),
        _expression(krys::move(expression)), _effects(krys::move(effects))
  {
  }

  auto SVGFilterRenderer::BuildExpression(svg::SVGElement *contextElement,
                                          svg::SVGFilterElement &filterElement,
                                          const SVGFilterRenderer &filter,
                                          const GraphicsContext &destinationContext) noexcept
    -> Maybe<BuiltExpression>
  {
    auto result = BuildFilterEffectGraph(contextElement, filterElement, filter, destinationContext);
    if (!result)
    {
      return null;
    }

    auto &graph = result->Graph;
    auto &effectGeometryMap = result->EffectGeometryMap;

    auto effectGeometry = [&](FilterEffect &effect) -> Maybe<FilterEffectGeometry>
    {
      auto it = effectGeometryMap.find(ShareRef(effect));
      if (it != effectGeometryMap.end())
      {
        return it->second;
      }
      return null;
    };

    SVGFilterExpression expression;
    auto effects = graph.Nodes();

    bool success = graph.Visit(
      [&](FilterEffect &effect, uint32 level)
      {
        auto it = std::ranges::find_if(effects, [&](const auto &item) { return item.get() == &effect; });
        krys_debug_assert(it != effects.end());
        expression.push_back({static_cast<uint32>(it - effects.begin()), level, effectGeometry(effect)});
      });

    if (!success)
    {
      return null;
    }

    std::ranges::reverse(expression);
    expression.shrink_to_fit();
    return BuiltExpression {krys::move(expression), krys::move(effects)};
  }

  bool SVGFilterRenderer::IsIdentity(svg::SVGFilterElement &filterElement) noexcept
  {
    auto graph = BuildFilterPrimitiveGraph(filterElement);
    if (!graph)
    {
      return false;
    }

    bool isIdentity = true;
    graph->Visit(
      [&](svg::SVGFilterPrimitiveStandardAttributes &primitive, uint32)
      {
        if (!primitive.IsIdentity())
        {
          isIdentity = false;
        }
      });

    return isIdentity;
  }

  IntOutsets SVGFilterRenderer::CalculateOutsets(svg::SVGFilterElement &filterElement,
                                                 const FloatRect &targetBoundingBox) noexcept
  {
    auto graph = BuildFilterPrimitiveGraph(filterElement);
    if (!graph)
    {
      return {};
    }

    List<std::pair<IntOutsets, uint32>> outsetsStack;

    // Remove the outsets of the last level and return their maximum.
    auto lastLevelOutsets = [](auto &outsetsStack) -> IntOutsets
    {
      IntOutsets lastLevelOutsets;
      for (uint32 lastLevel = outsetsStack.back().second; lastLevel == outsetsStack.back().second;
           outsetsStack.pop_back())
      {
        lastLevelOutsets = max(lastLevelOutsets, outsetsStack.back().first);
      }
      return lastLevelOutsets;
    };

    bool result = graph->Visit(
      [&](svg::SVGFilterPrimitiveStandardAttributes &primitive, uint32 level)
      {
        auto primitiveOutsets = primitive.Outsets(targetBoundingBox, filterElement.PrimitiveUnits());
        uint32 lastLevel = outsetsStack.empty() ? 0 : outsetsStack.back().second;

        // Expand the last outsets of this level with the maximum of the outsets of its children.
        if (level < lastLevel)
        {
          auto childrenOutsets = lastLevelOutsets(outsetsStack);
          outsetsStack.back().first += childrenOutsets;
        }

        outsetsStack.emplace_back(primitiveOutsets, level);
      });

    if (!result)
    {
      return IntOutsets();
    }

    krys_debug_assert(!outsetsStack.empty());

    // Calculate the whole filter outsets by going back to the lastNode of the graph.
    while (outsetsStack.size() > 1)
    {
      auto childrenOutsets = lastLevelOutsets(outsetsStack);
      outsetsStack.back().first += childrenOutsets;
    }

    return outsetsStack.back().first;
  }

  FloatSize SVGFilterRenderer::CalculateResolvedSize(const FloatSize &size,
                                                     const FloatRect &targetBoundingBox,
                                                     svg::SVGUnitTypes::SVGUnitType primitiveUnits) noexcept
  {
    return primitiveUnits == svg::SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX
             ? size * targetBoundingBox.Size()
             : size;
  }

  FloatSize SVGFilterRenderer::ResolvedSize(const FloatSize &size) const noexcept
  {
    return CalculateResolvedSize(size, ReferenceBox(), _primitiveUnits);
  }

  FloatPoint3D SVGFilterRenderer::ResolvedPoint3D(const FloatPoint3D &point) const noexcept
  {
    if (_primitiveUnits != svg::SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
      return point;
    }

    auto referenceBox = ReferenceBox();
    FloatPoint3D resolvedPoint;
    resolvedPoint.SetX(referenceBox.X() + point.X() * referenceBox.Width());
    resolvedPoint.SetY(referenceBox.Y() + point.Y() * referenceBox.Height());

    // https://www.w3.org/TR/SVG/filters.html#fePointLightZAttribute and
    // https://www.w3.org/TR/SVG/coords.html#Units_viewport_percentage
    resolvedPoint.SetZ(point.Z()
                       * EuclidianDistance(referenceBox.MinXMinYCorner(), referenceBox.MaxXMaxYCorner())
                       / std::numbers::sqrt2_v<float>);

    return resolvedPoint;
  }

  EnumSet<FilterRenderingMode> SVGFilterRenderer::SupportedFilterRenderingModes(
    EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept
  {
    EnumSet<FilterRenderingMode> modes = AllFilterRenderingModes;

    for (const auto &effect : _effects)
    {
      modes = modes & effect->SupportedFilterRenderingModes(preferredFilterRenderingModes);
    }

    krys_debug_assert(!modes.IsEmpty());
    return modes;
  }

  FilterEffectVector SVGFilterRenderer::EffectsOfType(FilterFunction::Type filterType) const noexcept
  {
    FilterEffectVector effects;

    for (const auto &effect : _effects)
    {
      if (effect->FilterType() == filterType)
      {
        effects.push_back(effect);
      }
    }

    return effects;
  }

  FilterResults &SVGFilterRenderer::EnsureResults(const FilterResultsCreator &resultsCreator) noexcept
  {
    if (!_results)
    {
      _results = resultsCreator();
    }
    return *_results;
  }

  void SVGFilterRenderer::ClearEffectResult(FilterEffect &effect) noexcept
  {
    if (_results)
    {
      _results->ClearEffectResult(effect);
    }
  }

  void SVGFilterRenderer::MergeEffects(const FilterEffectVector &effects) noexcept
  {
    krys_debug_assert(_effects.size() == effects.size());

    for (size_t index = 0; index < _effects.size(); ++index)
    {
      if (ArePointingToEqualData(_effects[index], effects[index]))
      {
        continue;
      }

      ClearEffectResult(*_effects[index]);
      _effects[index] = effects[index];
    }
  }

  RefPtr<FilterImage> SVGFilterRenderer::Apply(const Filter &, FilterImage &sourceImage,
                                               FilterResults &results) noexcept
  {
    return Apply(&sourceImage, results);
  }

  RefPtr<FilterImage> SVGFilterRenderer::Apply(FilterImage *sourceImage, FilterResults &results) noexcept
  {
    krys_debug_assert(!_expression.empty());
    krys_debug_assert(FilterRenderingModes().Contains(FilterRenderingMode::Accelerated));
    krys_debug_assert(IsValidSVGFilterExpression(_expression, _effects));

    FilterImageVector stack;

    for (const auto &term : _expression)
    {
      auto &effect = _effects[term.Index];
      const auto &geometry = term.Geometry;

      if (effect->FilterType() == FilterEffect::Type::SourceGraphic)
      {
        if (auto *result = results.EffectResult(*effect))
        {
          stack.push_back(ShareRef(*result));
          continue;
        }

        if (!sourceImage)
        {
          return nullptr;
        }

        // Add sourceImage as an input to the SourceGraphic.
        stack.push_back(ShareRef(*sourceImage));
      }

      // Need to remove the inputs here in case the effect already has a result.
      auto inputs = effect->TakeImageInputs(stack);

      auto result = effect->Apply(*this, inputs, results, geometry);
      if (!result)
      {
        return nullptr;
      }

      stack.push_back(result);
    }

    krys_debug_assert(stack.size() == 1);
    return stack.back();
  }

  bool SVGFilterRenderer::IsValidSVGFilterExpression(const SVGFilterExpression &expression,
                                                     const FilterEffectVector &effects) noexcept
  {
    if (expression.empty() || effects.empty())
    {
      return false;
    }

    for (const auto &term : expression)
    {
      if (term.Index >= effects.size())
      {
        return false;
      }
    }

    return true;
  }

  FilterStyleVector SVGFilterRenderer::CreateFilterStyles(GraphicsContext &context, const Filter &,
                                                          const FilterStyle &sourceStyle) const noexcept
  {
    return CreateFilterStyles(context, sourceStyle);
  }

  FilterStyleVector SVGFilterRenderer::CreateFilterStyles(GraphicsContext &context,
                                                          const FilterStyle &sourceStyle) const noexcept
  {
    krys_debug_assert(!_expression.empty());
    krys_debug_assert(FilterRenderingModes().Contains(FilterRenderingMode::GraphicsContext));

    FilterStyleVector styles;
    FilterStyle lastStyle = sourceStyle;

    for (const auto &term : _expression)
    {
      const auto &effect = _effects[term.Index];
      const auto &geometry = term.Geometry;

      if (effect->FilterType() == FilterEffect::Type::SourceGraphic)
      {
        continue;
      }

      krys_debug_assert(effect->NumberOfImageInputs() == 1);
      auto style = effect->CreateFilterStyle(context, *this, lastStyle, geometry);

      lastStyle = style;
      styles.push_back(style);
    }

    return styles;
  }

  TextStream &SVGFilterRenderer::ExternalRepresentation(TextStream &ts,
                                                        FilterRepresentation representation) const noexcept
  {
    for (auto it = _expression.rbegin(), end = _expression.rend(); it != end; ++it)
    {
      const auto &term = *it;
      const auto &effect = _effects[term.Index];

      // SourceAlpha is a built-in effect. No need to say SourceGraphic is its input.
      if (effect->FilterType() == FilterEffect::Type::SourceAlpha)
      {
        ++it;
      }

      TextStream::IndentScope indentScope(ts, static_cast<int32>(term.Level));
      effect->ExternalRepresentation(ts, representation);
    }

    return ts;
  }
}
