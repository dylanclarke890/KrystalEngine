#include "Krystal.Booey/Style/Extractor.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Properties/ShorthandSerialiser.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeAncestorIterator.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/ExtractorGenerated.hpp"
#include "Krystal.Booey/Style/Interpolation.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/ScopeRuleSets.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  namespace
  {
    enum class ForcedLayout : uint8
    {
      No,
      Yes,
      ParentDocument
    };

    using PhysicalDirection = BoxSide;
    using FlowRelativeDirection = LogicalBoxSide;

    KRYS_NODISCARD dom::Element *StyleElementForNode(dom::Node *node) noexcept
    {
      if (!node)
      {
        return nullptr;
      }
      if (auto *element = DynamicDowncast<dom::Element>(*node))
      {
        return element;
      }
      return ComposedTreeAncestors(*node).First();
    }

    KRYS_NODISCARD bool HasValidStyleForProperty(dom::Element &element, css::PropertyId) noexcept
    {
      if (element.StyleValidity() != Validity::Valid)
      {
        return false;
      }
      if (element.NodeDocument().HasPendingFullStyleRebuild())
      {
        return false;
      }
      if (!element.NodeDocument().ChildNeedsStyleRecalc())
      {
        return true;
      }

      // FIXME: Animations. A property with a running keyframe effect doesn't have a valid style.

      auto isQueryContainer = [&](dom::Element &element)
      {
        const auto *style = element.GetRenderStyle();
        return style && style->GetContainerType() != ContainerType::Normal;
      };

      if (isQueryContainer(element))
      {
        return false;
      }

      const auto *currentElement = &element;
      for (auto &ancestor : ComposedTreeAncestors(element))
      {
        if (ancestor.StyleValidity() != Validity::Valid)
        {
          return false;
        }

        if (isQueryContainer(ancestor))
        {
          return false;
        }

        if (ancestor.DirectChildNeedsStyleRecalc() && currentElement->StyleIsAffectedByPreviousSibling())
        {
          return false;
        }

        currentElement = &ancestor;
      }

      return true;
    }

    KRYS_NODISCARD const RenderStyle *
      ComputeRenderStyleForProperty(dom::Element &element,
                                    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                                    css::PropertyId propertyID, UniquePtr<RenderStyle> &ownedStyle) noexcept
    {
      if (style::interpolation::IsAccelerated(propertyID, element.NodeDocument().GetSettings()))
      {
        style::Styleable styleable(element, pseudoElementIdentifier);
        if (styleable.Renderer() && styleable.IsRunningAcceleratedAnimationOfProperty(propertyID))
        {
          ownedStyle = styleable.Renderer()->AnimatedStyle();
          return ownedStyle.get();
        }
      }

      return element.ComputedStyle(pseudoElementIdentifier);
    }

    KRYS_NODISCARD bool IsLayoutDependent(css::PropertyId propertyID, const RenderStyle *style,
                                          const RenderElement *renderer) noexcept
    {
      auto isNonReplacedInline = [](auto &renderer)
      {
        return renderer.IsInline() && !renderer.IsBlockLevelReplacedOrAtomicInline();
      };

      auto formattingContextRootStyle = [](auto &renderer) -> const RenderStyle &
      {
        const RenderElement *ancestorToUse =
          (renderer.IsFlexItem() || renderer.IsGridItem()) ? renderer.Parent() : renderer.ContainingBlock();
        if (ancestorToUse)
        {
          return ancestorToUse->Style();
        }
        krys_debug_assert_msg(false, "a box has a parent or a containing block");
        return renderer.Style();
      };

      auto mapLogicalToPhysicalPaddingProperty = [&](auto direction, auto &renderer) -> css::PropertyId
      {
        switch (MapSideLogicalToPhysical(formattingContextRootStyle(renderer).GetWritingMode(), direction))
        {
          case PhysicalDirection::Top:    return css::PropertyId::PaddingTop;
          case PhysicalDirection::Right:  return css::PropertyId::PaddingRight;
          case PhysicalDirection::Bottom: return css::PropertyId::PaddingBottom;
          case PhysicalDirection::Left:   return css::PropertyId::PaddingLeft;
          default:
            krys_debug_assert_msg(false, "a flow-relative side maps to a physical side");
            return css::PropertyId::Invalid;
        }
      };

      auto paddingIsLayoutDependent = [&](auto lengthGetter) -> bool
      {
        return renderer && style && renderer->IsRenderBox() && !(style->*lengthGetter)().IsFixed();
      };

      switch (propertyID)
      {
        case css::PropertyId::Top:
        case css::PropertyId::Bottom:
        case css::PropertyId::Left:
        case css::PropertyId::Right:
        case css::PropertyId::InsetBlockStart:
        case css::PropertyId::InsetBlockEnd:
        case css::PropertyId::InsetInlineStart:
        case css::PropertyId::InsetInlineEnd:   return renderer && style && renderer->IsRenderBox();
        case css::PropertyId::Width:
        case css::PropertyId::Height:
        case css::PropertyId::InlineSize:
        case css::PropertyId::BlockSize:
          return renderer && !renderer->IsRenderOrLegacyRenderSVGModelObject()
                 && !isNonReplacedInline(*renderer);
        case css::PropertyId::Margin:
        case css::PropertyId::MarginBlock:
        case css::PropertyId::MarginBlockStart:
        case css::PropertyId::MarginBlockEnd:
        case css::PropertyId::MarginInline:
        case css::PropertyId::MarginInlineStart:
        case css::PropertyId::MarginInlineEnd:
        case css::PropertyId::MarginTop:
        case css::PropertyId::MarginRight:
        case css::PropertyId::MarginBottom:
        case css::PropertyId::MarginLeft:        return renderer && renderer->IsRenderBox();
        case css::PropertyId::PerspectiveOrigin:
        case css::PropertyId::TransformOrigin:
        case css::PropertyId::Transform:
        case css::PropertyId::Filter:         // Why are filters layout-dependent?
        case css::PropertyId::BackdropFilter: // Why are backdrop-filters layout-dependent?
          return true;
        case css::PropertyId::Padding:
          return IsLayoutDependent(css::PropertyId::PaddingBlock, style, renderer)
                 || IsLayoutDependent(css::PropertyId::PaddingInline, style, renderer);
        case css::PropertyId::PaddingBlock:
          return IsLayoutDependent(css::PropertyId::PaddingBlockStart, style, renderer)
                 || IsLayoutDependent(css::PropertyId::PaddingBlockEnd, style, renderer);
        case css::PropertyId::PaddingInline:
          return IsLayoutDependent(css::PropertyId::PaddingInlineStart, style, renderer)
                 || IsLayoutDependent(css::PropertyId::PaddingInlineEnd, style, renderer);
        case css::PropertyId::PaddingBlockStart:
          if (const auto *renderBox = DynamicDowncast<RenderBox>(renderer))
          {
            return IsLayoutDependent(
              mapLogicalToPhysicalPaddingProperty(FlowRelativeDirection::BlockStart, *renderBox), style,
              renderBox);
          }
          return false;
        case css::PropertyId::PaddingBlockEnd:
          if (const auto *renderBox = DynamicDowncast<RenderBox>(renderer))
          {
            return IsLayoutDependent(
              mapLogicalToPhysicalPaddingProperty(FlowRelativeDirection::BlockEnd, *renderBox), style,
              renderBox);
          }
          return false;
        case css::PropertyId::PaddingInlineStart:
          if (const auto *renderBox = DynamicDowncast<RenderBox>(renderer))
          {
            return IsLayoutDependent(
              mapLogicalToPhysicalPaddingProperty(FlowRelativeDirection::InlineStart, *renderBox), style,
              renderBox);
          }
          return false;
        case css::PropertyId::PaddingInlineEnd:
          if (const auto *renderBox = DynamicDowncast<RenderBox>(renderer))
          {
            return IsLayoutDependent(
              mapLogicalToPhysicalPaddingProperty(FlowRelativeDirection::InlineEnd, *renderBox), style,
              renderBox);
          }
          return false;
        case css::PropertyId::PaddingTop:          return paddingIsLayoutDependent(&RenderStyle::PaddingTop);
        case css::PropertyId::PaddingRight:        return paddingIsLayoutDependent(&RenderStyle::PaddingRight);
        case css::PropertyId::PaddingBottom:       return paddingIsLayoutDependent(&RenderStyle::PaddingBottom);
        case css::PropertyId::PaddingLeft:         return paddingIsLayoutDependent(&RenderStyle::PaddingLeft);
        case css::PropertyId::GridTemplateColumns:
        case css::PropertyId::GridTemplateRows:
        case css::PropertyId::GridTemplate:
        case css::PropertyId::Grid:                return renderer && renderer->IsRenderGrid();
        default:                                   return false;
      }
    }
  }

  Extractor::Extractor(dom::Node *node, bool allowVisitedStyle,
                       const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
      : Extractor(StyleElementForNode(node), allowVisitedStyle, pseudoElementIdentifier)
  {
  }

  Extractor::Extractor(dom::Node *node, bool allowVisitedStyle) noexcept
      : Extractor(node, allowVisitedStyle, null)
  {
  }

  Extractor::Extractor(dom::Element *element, bool allowVisitedStyle,
                       const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
      : _element(ShareRefPtr(element)), _pseudoElementIdentifier(pseudoElementIdentifier),
        _allowVisitedStyle(allowVisitedStyle)
  {
  }

  Extractor::Extractor(dom::Element *element, bool allowVisitedStyle) noexcept
      : Extractor(element, allowVisitedStyle, null)
  {
  }

  RefPtr<css::PrimitiveValue> Extractor::GetFontSizeCSSValuePreferringKeyword() const noexcept
  {
    RefPtr element = _element;
    if (!element)
    {
      return nullptr;
    }

    element->NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    const auto *style = element->ComputedStyle(_pseudoElementIdentifier);
    if (!style)
    {
      return nullptr;
    }

    if (auto sizeIdentifier = style->GetFontDescription().KeywordSizeAsIdentifier();
        sizeIdentifier != css::ValueId::Invalid)
    {
      return css::PrimitiveValue::Create(sizeIdentifier);
    }

    return css::PrimitiveValue::Create(
      AdjustFloatForAbsoluteZoom(style->GetFontDescription().ComputedSize(), *style), css::UnitType::px);
  }

  bool Extractor::UseFixedFontDefaultSize() const noexcept
  {
    RefPtr element = _element;
    if (!element)
    {
      return false;
    }

    const auto *style = element->ComputedStyle(_pseudoElementIdentifier);
    if (!style)
    {
      return false;
    }

    return style->GetFontDescription().UseFixedDefaultSize();
  }

  const RenderElement *Extractor::ComputeRenderer() const noexcept
  {
    RefPtr element = _element;
    if (!element)
    {
      return nullptr;
    }
    if (_pseudoElementIdentifier)
    {
      return Styleable(*element, _pseudoElementIdentifier).Renderer();
    }
    if (element->HasDisplayContents())
    {
      return nullptr;
    }
    return element->Renderer();
  }

  bool Extractor::UpdateStyleIfNeededForProperty(dom::Element &element, css::PropertyId propertyID) noexcept
  {
    Ref document = ShareRef(element.NodeDocument());

    document->StyleScope().FlushPendingUpdate();

    auto hasValidStyle = [&]
    {
      auto shorthand = css::ShorthandForProperty(propertyID);
      if (shorthand.Size())
      {
        for (auto longhand : shorthand)
        {
          if (!HasValidStyleForProperty(element, longhand))
          {
            return false;
          }
        }
        return true;
      }
      return HasValidStyleForProperty(element, propertyID);
    }();

    if (hasValidStyle)
    {
      return false;
    }

    document->UpdateStyleIfNeeded();
    return true;
  }

  const RenderStyle *
    Extractor::ComputeStyleForCustomProperty(UniquePtr<RenderStyle> &ownedStyle) const noexcept
  {
    RefPtr element = _element;
    if (!element)
    {
      return nullptr;
    }

    UpdateStyleIfNeededForProperty(*element, css::PropertyId::Custom);

    const auto *style =
      ComputeRenderStyleForProperty(*element, _pseudoElementIdentifier, css::PropertyId::Custom, ownedStyle);
    if (!style)
    {
      return nullptr;
    }

    // FIXME: Frames. Upstream updates the owner element's document's layout when the document has a style
    // with viewport units.

    return style;
  }

  RefPtr<css::Value> Extractor::CustomPropertyValue(const css::CSSOMStringAtom &propertyName) const noexcept
  {
    UniquePtr<RenderStyle> ownedStyle;
    const auto *style = ComputeStyleForCustomProperty(ownedStyle);
    if (!style)
    {
      return nullptr;
    }

    const auto *value = style->CustomPropertyValue(propertyName);
    if (!value)
    {
      return nullptr;
    }

    return value->PropertyValue(css::ValuePool::MainThreadPool(), *style);
  }

  css::CSSOMString
    Extractor::CustomPropertyValueSerialisation(const css::CSSOMStringAtom &propertyName,
                                                const css::SerialisationContext &context) const noexcept
  {
    UniquePtr<RenderStyle> ownedStyle;
    if (const auto *style = ComputeStyleForCustomProperty(ownedStyle))
    {
      return CustomPropertyValueSerialisationInStyle(*style, propertyName, context);
    }
    return {};
  }

  css::CSSOMString Extractor::CustomPropertyValueSerialisationInStyle(
    const RenderStyle &style, const css::CSSOMStringAtom &propertyName,
    const css::SerialisationContext &context) const noexcept
  {
    if (const auto *value = style.CustomPropertyValue(propertyName))
    {
      return value->PropertyValueSerialisation(context, style);
    }
    return {};
  }

  const RenderStyle *Extractor::ComputeStyle(css::PropertyId propertyID, UpdateLayout updateLayout,
                                             UniquePtr<RenderStyle> &ownedStyle) const noexcept
  {
    RefPtr element = _element;
    if (!element)
    {
      return nullptr;
    }

    if (!css::IsExposed(propertyID, element->NodeDocument().CssParserContext().PropertySettings))
    {
      // Exit quickly, and avoid us ever having to update layout in this case.
      return nullptr;
    }

    const RenderStyle *style = nullptr;
    auto forcedLayout = ForcedLayout::No;

    if (updateLayout == UpdateLayout::Yes)
    {
      Ref document = ShareRef(element->NodeDocument());

      UpdateStyleIfNeededForProperty(*element, propertyID);
      const auto *renderer = ComputeRenderer();
      if (propertyID == css::PropertyId::Display && !renderer)
      {
        auto *svgElement = DynamicDowncast<svg::SVGElement>(*element);
        if (svgElement && !svgElement->IsValid())
        {
          return nullptr;
        }
      }

      style = ComputeRenderStyleForProperty(*element, _pseudoElementIdentifier, propertyID, ownedStyle);

      forcedLayout = [&]
      {
        // FIXME: Some of these cases could be narrowed down or optimized better.
        if (IsLayoutDependent(propertyID, style, renderer))
        {
          return ForcedLayout::Yes;
        }
        // FIXME: Why?
        if (element->IsInShadowTree())
        {
          return ForcedLayout::Yes;
        }
        // FIXME: Frames. A document in a frame forces its layout when its rule sets have viewport-dependent
        // media queries or container queries, and its parent document's when it has viewport units.
        return ForcedLayout::No;
      }();

      if (forcedLayout == ForcedLayout::Yes)
      {
        document->UpdateLayoutIgnorePendingStylesheets(
          {dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
           dom::LayoutOptions::TreatContentVisibilityAutoAsVisible},
          element.get());
      }
    }

    if (updateLayout == UpdateLayout::No || forcedLayout != ForcedLayout::No)
    {
      style = ComputeRenderStyleForProperty(*element, _pseudoElementIdentifier, propertyID, ownedStyle);
    }

    return style;
  }

  RefPtr<css::Value> Extractor::PropertyValue(css::PropertyId propertyID, UpdateLayout updateLayout,
                                              ExtractorState::PropertyValueType valueType) const noexcept
  {
    UniquePtr<RenderStyle> ownedStyle;
    const auto *style = ComputeStyle(propertyID, updateLayout, ownedStyle);
    if (!style)
    {
      return nullptr;
    }

    return PropertyValueInStyle(
      *style, propertyID, css::ValuePool::MainThreadPool(),
      valueType == ExtractorState::PropertyValueType::Resolved ? ComputeRenderer() : nullptr, valueType);
  }

  css::CSSOMString
    Extractor::PropertyValueSerialisation(css::PropertyId propertyID,
                                          const css::SerialisationContext &context, UpdateLayout updateLayout,
                                          ExtractorState::PropertyValueType valueType) const noexcept
  {
    UniquePtr<RenderStyle> ownedStyle;
    const auto *style = ComputeStyle(propertyID, updateLayout, ownedStyle);
    if (!style)
    {
      return {};
    }

    auto canUseShorthandSerialiserForPropertyValue = [&]()
    {
      switch (propertyID)
      {
        case css::PropertyId::Gap:
        case css::PropertyId::GridArea:
        case css::PropertyId::GridColumn:
        case css::PropertyId::GridRow:
        case css::PropertyId::GridTemplate: return true;
        default:                            return false;
      }
    };
    if (css::IsShorthand(propertyID) && canUseShorthandSerialiserForPropertyValue())
    {
      return css::SerialiseShorthandValue(context, *this, propertyID);
    }

    return PropertyValueSerialisationInStyle(
      *style, propertyID, context, css::ValuePool::MainThreadPool(),
      valueType == ExtractorState::PropertyValueType::Resolved ? ComputeRenderer() : nullptr, valueType);
  }

  RefPtr<css::Value>
    Extractor::PropertyValueInStyle(const RenderStyle &style, css::PropertyId propertyID,
                                    css::ValuePool &pool, const RenderElement *renderer,
                                    ExtractorState::PropertyValueType valueType) const noexcept
  {
    krys_debug_assert(
      css::IsExposed(propertyID, _element->NodeDocument().CssParserContext().PropertySettings));

    ExtractorState state {
      .ValueType = valueType,
      .Style = style,
      .Element = Ref<dom::Element>(_element),
      .PseudoElementIdentifier = _pseudoElementIdentifier,
      .Renderer = renderer,
      .AllowVisitedStyle = _allowVisitedStyle,
      .Pool = pool,
    };
    return ExtractorGenerated::ExtractValue(state, propertyID);
  }

  css::CSSOMString
    Extractor::PropertyValueSerialisationInStyle(const RenderStyle &style, css::PropertyId propertyID,
                                                 const css::SerialisationContext &context,
                                                 css::ValuePool &pool, const RenderElement *renderer,
                                                 ExtractorState::PropertyValueType valueType) const noexcept
  {
    krys_debug_assert(
      css::IsExposed(propertyID, _element->NodeDocument().CssParserContext().PropertySettings));

    StringBuilder builder;

    ExtractorState state {
      .ValueType = valueType,
      .Style = style,
      .Element = Ref<dom::Element>(_element),
      .PseudoElementIdentifier = _pseudoElementIdentifier,
      .Renderer = renderer,
      .AllowVisitedStyle = _allowVisitedStyle,
      .Pool = pool,
    };
    ExtractorGenerated::ExtractValueSerialisation(state, builder, context, propertyID);

    return krys::move(builder).ToString();
  }

  bool Extractor::PropertyMatches(css::PropertyId propertyID, const css::Value *value) const noexcept
  {
    if (!_element)
    {
      return false;
    }
    if (propertyID == css::PropertyId::FontSize)
    {
      if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
      {
        _element->NodeDocument().UpdateLayoutIgnorePendingStylesheets();
        if (const auto *style = _element->ComputedStyle(_pseudoElementIdentifier))
        {
          if (css::ValueId sizeIdentifier = style->GetFontDescription().KeywordSizeAsIdentifier();
              sizeIdentifier != css::ValueId::Invalid)
          {
            if (primitiveValue->IsValueId() && primitiveValue->ValueId() == sizeIdentifier)
            {
              return true;
            }
          }
        }
      }
    }
    RefPtr<css::Value> computedValue = PropertyValue(propertyID);
    return computedValue && value && computedValue->Equals(*value);
  }

  Ref<css::MutableStyleProperties>
    Extractor::CopyProperties(Span<const css::PropertyId> properties) const noexcept
  {
    List<css::Property> list;
    for (auto property : properties)
    {
      if (auto value = PropertyValue(property))
      {
        list.emplace_back(property, Ref<css::Value>(krys::move(value)));
      }
    }
    return css::MutableStyleProperties::Create(krys::move(list));
  }

  Ref<css::MutableStyleProperties> Extractor::CopyProperties() const noexcept
  {
    List<css::Property> list;
    for (auto property : css::AllLonghandProperties())
    {
      if (auto value = PropertyValue(property))
      {
        list.emplace_back(property, Ref<css::Value>(krys::move(value)));
      }
    }
    return css::MutableStyleProperties::Create(krys::move(list));
  }
}
