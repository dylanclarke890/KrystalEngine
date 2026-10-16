#pragma once

#include "Krystal.Booey/CSS/FontSelectionValueInlines.hpp"
#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthandFunctions.hpp"
#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/Values/FontValue.hpp"
#include "Krystal.Booey/CSS/Values/GridAutoRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/GridIntegerRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/ExtractorGenerated.hpp"
#include "Krystal.Booey/Style/ExtractorState.hpp"
#include "Krystal.Booey/Style/OrderedNamedLinesCollector.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/SeamRenderers.hpp"
#include "Krystal.Booey/Style/TransformResolver.hpp"
#include "Krystal.Booey/Style/Values/Primitives/CoordinatedValueListValue.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <algorithm>

namespace krys::boo::style
{
  // Custom handling of computed value extraction.
  class ExtractorCustom
  {
  public:
#define KRYS_DECLARE_EXTRACTOR_CUSTOM(name)                                                                  \
  KRYS_NODISCARD static Ref<css::Value> Extract##name(ExtractorState &) noexcept;                            \
  static void Extract##name##Serialisation(ExtractorState &, StringBuilder &,                                \
                                           const css::SerialisationContext &) noexcept

#define KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(name)                                                         \
  KRYS_NODISCARD static RefPtr<css::Value> Extract##name(ExtractorState &) noexcept;                         \
  static void Extract##name##Serialisation(ExtractorState &, StringBuilder &,                                \
                                           const css::SerialisationContext &) noexcept

    KRYS_DECLARE_EXTRACTOR_CUSTOM(Direction);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(WritingMode);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Float);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Content);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(LetterSpacing);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(WordSpacing);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(LineHeight);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(FontFamily);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(FontSize);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Top);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Right);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Bottom);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Left);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(MarginTop);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(MarginRight);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(MarginBottom);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(MarginLeft);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(PaddingTop);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(PaddingRight);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(PaddingBottom);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(PaddingLeft);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Height);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Width);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(MaxHeight);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(MaxWidth);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(MinHeight);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(MinWidth);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(CounterIncrement);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(CounterReset);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(CounterSet);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Transform);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(BorderImageWidth);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Translate);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Scale);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Rotate);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(GridAutoFlow);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(GridTemplateColumns);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(GridTemplateRows);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(AnimationDuration);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Widows);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Orphans);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(WebkitTextCombine);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(WebkitRubyPosition);
    KRYS_DECLARE_EXTRACTOR_CUSTOM(Color);

    // MARK: Shorthands

    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(AnimationShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(AnimationRangeShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(BackgroundShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(BackgroundPositionShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(BlockStepShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(BorderShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(BorderBlockShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(BorderImageShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(BorderInlineShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(BorderRadiusShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(ColumnsShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(ContainerShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(FlexFlowShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(FontShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(FontSynthesisShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(FontVariantShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(LineClampShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(MaskShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(MaskBorderShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(MaskPositionShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(OffsetShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(OverscrollBehaviorShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(PageBreakAfterShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(PageBreakBeforeShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(PageBreakInsideShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(PerspectiveOriginShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(PositionTryShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(ScrollTimelineShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(TextBoxShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(TextDecorationSkipShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(TextDecorationShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(TextWrapShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(TransformOriginShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(TransitionShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(ViewTimelineShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(WhiteSpaceShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(WebkitMaskPositionShorthand);
    KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL(MarkerShorthand);

#undef KRYS_DECLARE_EXTRACTOR_CUSTOM
#undef KRYS_DECLARE_EXTRACTOR_CUSTOM_OPTIONAL
  };

  // MARK: - Shared Adaptor

  // Shared adaptors are used by adaptors to further adapt a value that has been partially extracted from a
  // RenderStyle. Like adaptors, they use a provided functor to allow them to be used for both css::Value
  // creation and serialisation.

  template <css::PropertyId propertyID>
  struct InsetEdgeSharedAdaptor
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, const InsetEdge &value, F &&functor) const noexcept
    {
      // If the element is not displayed; return the "computed value".
      const auto *box = DynamicDowncast<RenderBox>(state.Renderer);
      if (!box)
      {
        return functor(value);
      }

      const auto *containingBlock = DynamicDowncast<RenderBoxModelObject>(box->Container());

      // Resolve a "computed value" percentage if the element is positioned.
      if (containingBlock && value.IsPercentOrCalculated() && box->IsPositioned())
      {
        constexpr bool isVerticalProperty =
          (propertyID == css::PropertyId::Top || propertyID == css::PropertyId::Bottom);

        LayoutUnit containingBlockSize;
        if (box->IsStickilyPositioned())
        {
          auto &enclosingClippingBox = box->EnclosingClippingBoxForStickyPosition();
          if (isVerticalProperty == enclosingClippingBox.IsHorizontalWritingMode())
          {
            containingBlockSize = enclosingClippingBox.ContentBoxLogicalHeight();
          }
          else
          {
            containingBlockSize = enclosingClippingBox.ContentBoxLogicalWidth();
          }
        }
        else
        {
          if (box->IsOutOfFlowPositioned())
          {
            if (isVerticalProperty)
            {
              containingBlockSize =
                box->ContainingBlockRangeSizeForPositioned(*containingBlock, BoxAxis::Vertical);
            }
            else
            {
              containingBlockSize =
                box->ContainingBlockRangeSizeForPositioned(*containingBlock, BoxAxis::Horizontal);
            }
          }
          else
          {
            if (isVerticalProperty == containingBlock->IsHorizontalWritingMode())
            {
              containingBlockSize = box->ContainingBlockLogicalHeightForContent(
                AvailableLogicalHeightType::ExcludeMarginBorderPadding);
            }
            else
            {
              containingBlockSize = box->ContainingBlockLogicalWidthForContent();
            }
          }
        }
        return functor(Length<> {
          Evaluate<LayoutUnit>(value, containingBlockSize, containingBlock->Style().UsedZoomForLength())});
      }

      // Return a "computed value" length.
      if (!value.IsAuto())
      {
        return functor(value);
      }

      auto insetUsedStyleRelative = [&](const RenderBox &box) -> LayoutUnit
      {
        // For relatively positioned boxes, the inset is with respect to the top edges
        // of the box itself. This ties together top/bottom and left/right to be
        // opposites of each other.
        //
        // See http://www.w3.org/TR/CSS2/visuren.html#relative-positioning
        //
        // Specifically;
        //   Since boxes are not split or stretched as a result of 'left' or
        //   'right', the used values are always: left = -right.
        // and
        //   Since boxes are not split or stretched as a result of 'top' or
        //   'bottom', the used values are always: top = -bottom.

        if constexpr (propertyID == css::PropertyId::Top)
        {
          return box.RelativePositionOffset().Height();
        }
        else if constexpr (propertyID == css::PropertyId::Right)
        {
          return -(box.RelativePositionOffset().Width());
        }
        else if constexpr (propertyID == css::PropertyId::Bottom)
        {
          return -(box.RelativePositionOffset().Height());
        }
        else if constexpr (propertyID == css::PropertyId::Left)
        {
          return box.RelativePositionOffset().Width();
        }
      };

      // The property won't be over-constrained if its computed value is "auto", so the "used value" can be
      // returned.
      if (box->IsRelativelyPositioned())
      {
        return functor(Length<> {insetUsedStyleRelative(*box)});
      }

      auto insetUsedStyleOutOfFlowPositioned = [&](auto &container, auto &box) -> LayoutUnit
      {
        // For out-of-flow positioned boxes, the inset is how far an box's margin
        // edge is inset below the edge of the box's containing block.
        // See http://www.w3.org/TR/CSS2/visuren.html#position-props
        //
        // Margins are included in offsetTop/offsetLeft so we need to remove them here.
        auto paddingBoxWidth = [&]() -> LayoutUnit
        {
          if (const auto *renderBlock = DynamicDowncast<RenderBlock>(&container))
          {
            return renderBlock->PaddingBoxWidth();
          }
          if (const auto *inlineBox = DynamicDowncast<RenderInline>(&container))
          {
            return inlineBox->InnerPaddingBoxWidth();
          }
          krys_debug_assert_msg(false, "an out-of-flow box's container is a block or an inline box");
          return {};
        };
        auto paddingBoxHeight = [&]() -> LayoutUnit
        {
          if (const auto *renderBlock = DynamicDowncast<RenderBlock>(&container))
          {
            return renderBlock->PaddingBoxHeight();
          }
          if (const auto *inlineBox = DynamicDowncast<RenderInline>(&container))
          {
            return inlineBox->InnerPaddingBoxHeight();
          }
          krys_debug_assert_msg(false, "an out-of-flow box's container is a block or an inline box");
          return {};
        };
        if constexpr (propertyID == css::PropertyId::Top)
        {
          return box.OffsetTop() - box.MarginTop();
        }
        else if constexpr (propertyID == css::PropertyId::Right)
        {
          return paddingBoxWidth() - (box.OffsetLeft() + box.OffsetWidth()) - box.MarginRight();
        }
        else if constexpr (propertyID == css::PropertyId::Bottom)
        {
          return paddingBoxHeight() - (box.OffsetTop() + box.OffsetHeight()) - box.MarginBottom();
        }
        else if constexpr (propertyID == css::PropertyId::Left)
        {
          return box.OffsetLeft() - box.MarginLeft();
        }
      };

      if (containingBlock && box->IsOutOfFlowPositioned())
      {
        return functor(Length<> {insetUsedStyleOutOfFlowPositioned(*containingBlock, *box)});
      }

      return functor(css::keywords::Auto {});
    }
  };

  template <css::PropertyId propertyID>
  struct MarginEdgeSharedAdaptor
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, const MarginEdge &value, F &&functor) const noexcept
    {
      auto rendererCanHaveTrimmedMargin = [](const RenderBox &renderer)
      {
        auto marginTrimSide = []() -> MarginTrimSide
        {
          if constexpr (propertyID == css::PropertyId::MarginTop)
          {
            return MarginTrimSide::BlockStart;
          }
          else if constexpr (propertyID == css::PropertyId::MarginRight)
          {
            return MarginTrimSide::InlineEnd;
          }
          else if constexpr (propertyID == css::PropertyId::MarginBottom)
          {
            return MarginTrimSide::BlockEnd;
          }
          else if constexpr (propertyID == css::PropertyId::MarginLeft)
          {
            return MarginTrimSide::InlineStart;
          }
        };

        // A renderer will have a specific margin marked as trimmed by setting its rare data bit if:
        // 1.) The layout system the box is in has this logic (setting the rare data bit for this
        // specific margin) implemented
        // 2.) The block container/flexbox/grid has this margin specified in its margin-trim style
        // If marginTrimSide is empty we will check if any of the supported margins are in the style
        if (renderer.IsFlexItem() || renderer.IsGridItem())
        {
          return renderer.Parent()->Style().GetMarginTrim().Contains(marginTrimSide());
        }

        // Even though margin-trim is not inherited, it is possible for nested block level boxes
        // to get placed at the block-start of an containing block ancestor which does have margin-trim.
        // In this case it is not enough to simply check the immediate containing block of the child. It is
        // also probably too expensive to perform an arbitrary walk up the tree to check for the existence
        // of an ancestor containing block with the property, so we will just return true and let
        // the rest of the logic in RenderBox::hasTrimmedMargin to determine if the rare data bit
        // were set at some point during layout
        if (renderer.IsBlockLevelBox())
        {
          const auto *containingBlock = renderer.ContainingBlock();
          return containingBlock && containingBlock->IsHorizontalWritingMode();
        }
        return false;
      };

      auto toMarginTrimSide = [](const RenderBox &renderer) -> MarginTrimSide
      {
        auto formattingContextRootStyle = [](const RenderBox &renderer) -> const RenderStyle &
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

        auto boxSide = []() -> BoxSide
        {
          if constexpr (propertyID == css::PropertyId::MarginTop)
          {
            return BoxSide::Top;
          }
          else if constexpr (propertyID == css::PropertyId::MarginRight)
          {
            return BoxSide::Right;
          }
          else if constexpr (propertyID == css::PropertyId::MarginBottom)
          {
            return BoxSide::Bottom;
          }
          else if constexpr (propertyID == css::PropertyId::MarginLeft)
          {
            return BoxSide::Left;
          }
        };

        switch (MapSidePhysicalToLogical(formattingContextRootStyle(renderer).GetWritingMode(), boxSide()))
        {
          case LogicalBoxSide::BlockStart:  return MarginTrimSide::BlockStart;
          case LogicalBoxSide::BlockEnd:    return MarginTrimSide::BlockEnd;
          case LogicalBoxSide::InlineStart: return MarginTrimSide::InlineStart;
          case LogicalBoxSide::InlineEnd:   return MarginTrimSide::InlineEnd;
          default:
            krys_debug_assert_msg(false, "a physical side maps to a flow-relative side");
            return MarginTrimSide::BlockStart;
        }
      };

      auto usedValue = [](auto &box)
      {
        if constexpr (propertyID == css::PropertyId::MarginTop)
        {
          return Length<> {box.MarginTop()};
        }
        else if constexpr (propertyID == css::PropertyId::MarginRight)
        {
          return Length<> {box.MarginRight()};
        }
        else if constexpr (propertyID == css::PropertyId::MarginBottom)
        {
          return Length<> {box.MarginBottom()};
        }
        else if constexpr (propertyID == css::PropertyId::MarginLeft)
        {
          return Length<> {box.MarginLeft()};
        }
      };

      const auto *box = DynamicDowncast<RenderBox>(state.Renderer);
      if (!box)
      {
        return functor(value);
      }

      if constexpr (propertyID == css::PropertyId::MarginRight)
      {
        if (rendererCanHaveTrimmedMargin(*box) && box->HasTrimmedMargin(toMarginTrimSide(*box)))
        {
          return functor(usedValue(*box));
        }

        if (value.IsFixed())
        {
          return functor(value);
        }

        if (value.IsPercentOrCalculated())
        {
          // RenderBox gives a marginRight() that is the distance between the right-edge of the child box
          // and the right-edge of the containing box, when display == DisplayType::Block. Let's calculate the
          // absolute value of the specified margin-right % instead of relying on RenderBox's marginRight()
          // value.
          return functor(Length<> {EvaluateMinimum<float>(value, box->ContainingBlockLogicalWidthForContent(),
                                                          state.Style.UsedZoomForLength())});
        }
      }

      return functor(usedValue(*box));
    }
  };

  template <css::PropertyId propertyID>
  struct PaddingEdgeSharedAdaptor
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, const PaddingEdge &value, F &&functor) const noexcept
    {
      const auto *renderBox = DynamicDowncast<RenderBox>(state.Renderer);
      if (!renderBox || value.IsFixed())
      {
        return functor(value);
      }

      if constexpr (propertyID == css::PropertyId::PaddingTop)
      {
        return functor(Length<> {renderBox->ComputedCSSPaddingTop()});
      }
      else if constexpr (propertyID == css::PropertyId::PaddingRight)
      {
        return functor(Length<> {renderBox->ComputedCSSPaddingRight()});
      }
      else if constexpr (propertyID == css::PropertyId::PaddingBottom)
      {
        return functor(Length<> {renderBox->ComputedCSSPaddingBottom()});
      }
      else if constexpr (propertyID == css::PropertyId::PaddingLeft)
      {
        return functor(Length<> {renderBox->ComputedCSSPaddingLeft()});
      }
    }
  };

  template <css::PropertyId propertyID>
  struct PreferredSizeSharedAdaptor
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, const PreferredSize &value,
                                 F &&functor) const noexcept
    {
      auto sizingBox = [](auto &renderer) -> LayoutRect
      {
        const auto *box = DynamicDowncast<RenderBox>(&renderer);
        if (!box)
        {
          return LayoutRect();
        }
        return box->Style().GetBoxSizing() == BoxSizing::BorderBox ? box->BorderBoxRect()
                                                                   : box->ComputedCSSContentBoxRect();
      };

      auto isNonReplacedInline = [](auto &renderer)
      {
        return renderer.IsInline() && !renderer.IsBlockLevelReplacedOrAtomicInline();
      };

      if (state.Renderer && !state.Renderer->IsRenderOrLegacyRenderSVGModelObject())
      {
        // According to http://www.w3.org/TR/CSS2/visudet.html#the-height-property,
        // the "height" property does not apply for non-replaced inline elements.
        if (!isNonReplacedInline(*state.Renderer))
        {
          if constexpr (propertyID == css::PropertyId::Height)
          {
            return functor(Length<> {sizingBox(*state.Renderer).Height()});
          }
          else if constexpr (propertyID == css::PropertyId::Width)
          {
            return functor(Length<> {sizingBox(*state.Renderer).Width()});
          }
        }
      }
      return functor(value);
    }
  };

  template <css::PropertyId>
  struct MaximumSizeSharedAdaptor
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &, const MaximumSize &value, F &&functor) const noexcept
    {
      if (value.IsNone())
      {
        return functor(css::keywords::None {});
      }
      return functor(value);
    }
  };

  template <css::PropertyId>
  struct MinimumSizeSharedAdaptor
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, const MinimumSize &value, F &&functor) const noexcept
    {
      auto isFlexOrGridItem = [](auto renderer)
      {
        const auto *box = DynamicDowncast<RenderBox>(renderer);
        return box && (box->IsFlexItem() || box->IsGridItem());
      };

      if (value.IsAuto())
      {
        if (isFlexOrGridItem(state.Renderer))
        {
          return functor(css::keywords::Auto {});
        }
        return functor(Length<> {0});
      }
      return functor(value);
    }
  };

  template <css::PropertyId>
  struct PageBreakSharedAdaptor
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &, BreakBetween value, F &&functor) const noexcept
    {
      switch (value)
      {
        case BreakBetween::Page:
        case BreakBetween::LeftPage:
        case BreakBetween::RightPage:
        case BreakBetween::RectoPage:
        case BreakBetween::VersoPage:
          // CSS 2.1 allows us to map these to always.
          return functor(css::keywords::Always {});
        case BreakBetween::Avoid:
        case BreakBetween::AvoidPage:   return functor(css::keywords::Avoid {});
        case BreakBetween::AvoidColumn:
        case BreakBetween::Column:
        case BreakBetween::Auto:        return functor(css::keywords::Auto {});
      }
      krys_unreachable();
    }

    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &, BreakInside value, F &&functor) const noexcept
    {
      switch (value)
      {
        case BreakInside::Avoid:
        case BreakInside::AvoidPage:   return functor(css::keywords::Avoid {});
        case BreakInside::AvoidColumn:
        case BreakInside::Auto:        return functor(css::keywords::Auto {});
      }
      krys_unreachable();
    }
  };

  // MARK: - Adaptors

  // Adaptors are used to implement the logic for extracting a value from a RenderStyle and performing some
  // operation of the CSS value equivalent. This allows the same code to be used for CSS creation and
  // serialisation.

  template <css::PropertyId>
  struct PropertyExtractorAdaptor;

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::Direction>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      if (state.Element.get() == state.Element->NodeDocument().DocumentElement()
          && !state.Style.HasExplicitlySetDirection())
      {
        return functor(ComputedStyle::InitialDirection());
      }
      return functor(state.Style.GetWritingMode().ComputedTextDirection());
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::WritingMode>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      if (state.Element.get() == state.Element->NodeDocument().DocumentElement()
          && !state.Style.HasExplicitlySetWritingMode())
      {
        return functor(ComputedStyle::InitialWritingMode());
      }
      return functor(state.Style.GetWritingMode().ComputedWritingMode());
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::Float>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      if (state.Style.HasOutOfFlowPosition())
      {
        return functor(css::keywords::None {});
      }
      return functor(state.Style.Floating());
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::Content>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      if (state.Style.HasUsedContentNone())
      {
        return functor(css::keywords::None {});
      }
      return functor(state.Style.GetContent());
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::LetterSpacing>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      // "For legacy reasons, a computed letter-spacing of zero yields a
      //  resolved value (getComputedStyle() return value) of `normal`."
      // https://www.w3.org/TR/css-text-4/#letter-spacing-property

      auto &spacing = state.Style.ComputedLetterSpacing();
      if (auto fixedSpacing = spacing.TryFixed(); fixedSpacing && fixedSpacing->IsZero())
      {
        return functor(css::keywords::Normal {});
      }
      return functor(spacing);
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::WordSpacing>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      return functor(state.Style.ComputedWordSpacing());
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::LineHeight>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      return krys::SwitchOn(
        state.Style.GetLineHeight(), [&](const css::keywords::Normal &keyword) { return functor(keyword); },
        [&](const LineHeight::Fixed &fixed) { return functor(fixed); },
        [&](const LineHeight::Percentage &percentage)
        {
          // CSSValueConversion<LineHeight> will convert a percentage value to a fixed value,
          // and a number value to a percentage value. To be able to roundtrip a number value, we thus
          // look for a percent value and convert it back to a number.
          if (state.ValueType == ExtractorState::PropertyValueType::Computed)
          {
            return functor(Number<css::NonNegative> {percentage.Value / 100});
          }

          // This is imperfect, because it doesn't include the zoom factor and the real computation
          // for how high to be in pixels does include things like minimum font size and the zoom factor.
          // On the other hand, since font-size doesn't include the zoom factor, we really can't do
          // that here either.
          return functor(Length<css::NonNegative> {percentage.Value
                                                   * state.Style.GetFontDescription().ComputedSize() / 100});
        },
        [&](const LineHeight::Calc &calc)
        {
          // FIXME: We pass 1.0f here to get the unzoomed value but it really is not clear why we are even
          // evaluating calc here. We should probably revisit this and figure out another way to do this.
          return functor(Length<css::NonNegative> {Evaluate<float>(calc, 0.0f, ZoomFactor {1.0f, 1.0f})});
        });
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::FontFamily>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      auto fontFamily = state.Style.FontFamily();
      if (fontFamily.size() == 1)
      {
        return functor(fontFamily.First());
      }
      return functor(fontFamily);
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::FontSize>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      return functor(Length<css::NonNegative> {state.Style.GetFontDescription().ComputedSize()});
    }
  };

#define KRYS_EXTRACTOR_SHARED_ADAPTOR(property, adaptor, getter)                                             \
  template <>                                                                                                \
  struct PropertyExtractorAdaptor<css::PropertyId::property>                                                 \
  {                                                                                                          \
    template <typename F>                                                                                    \
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept                          \
    {                                                                                                        \
      return adaptor<css::PropertyId::property> {}.ComputedValue(state, state.Style.getter(),                \
                                                                 std::forward<F>(functor));                  \
    }                                                                                                        \
  }

  KRYS_EXTRACTOR_SHARED_ADAPTOR(Top, InsetEdgeSharedAdaptor, Top);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(Right, InsetEdgeSharedAdaptor, Right);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(Bottom, InsetEdgeSharedAdaptor, Bottom);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(Left, InsetEdgeSharedAdaptor, Left);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(MarginTop, MarginEdgeSharedAdaptor, MarginTop);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(MarginRight, MarginEdgeSharedAdaptor, MarginRight);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(MarginBottom, MarginEdgeSharedAdaptor, MarginBottom);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(MarginLeft, MarginEdgeSharedAdaptor, MarginLeft);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(PaddingTop, PaddingEdgeSharedAdaptor, PaddingTop);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(PaddingRight, PaddingEdgeSharedAdaptor, PaddingRight);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(PaddingBottom, PaddingEdgeSharedAdaptor, PaddingBottom);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(PaddingLeft, PaddingEdgeSharedAdaptor, PaddingLeft);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(Height, PreferredSizeSharedAdaptor, Height);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(Width, PreferredSizeSharedAdaptor, Width);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(MaxHeight, MaximumSizeSharedAdaptor, MaxHeight);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(MaxWidth, MaximumSizeSharedAdaptor, MaxWidth);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(MinHeight, MinimumSizeSharedAdaptor, MinHeight);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(MinWidth, MinimumSizeSharedAdaptor, MinWidth);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(PageBreakAfter, PageBreakSharedAdaptor, BreakAfter);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(PageBreakBefore, PageBreakSharedAdaptor, BreakBefore);
  KRYS_EXTRACTOR_SHARED_ADAPTOR(PageBreakInside, PageBreakSharedAdaptor, GetBreakInside);

#undef KRYS_EXTRACTOR_SHARED_ADAPTOR

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::GridAutoFlow>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      // FIXME: Adjust this once CSSWG clarifies exactly how the initial value should compute.
      // For now, this gives the most backwards-compatible behavior.
      auto gridFlow = state.Style.GetGridAutoFlow();
      switch (gridFlow.GetDirection())
      {
        case GridAutoFlow::Direction::Column:
          switch (gridFlow.GetPacking())
          {
            case GridAutoFlow::Packing::Dense:
              return functor(css::SpaceSeparatedTuple {css::keywords::Column {}, css::keywords::Dense {}});
            case GridAutoFlow::Packing::Sparse: return functor(css::keywords::Column {});
          }
          [[fallthrough]];
        case GridAutoFlow::Direction::Row:
          switch (gridFlow.GetPacking())
          {
            case GridAutoFlow::Packing::Dense:
              if (!state.Style.GridTemplateRows().IsNone() && state.Style.GridTemplateColumns().IsNone()
                  && (state.Style.Display() == DisplayType::GridLanes
                      || state.Style.Display() == DisplayType::InlineGridLanes))
              {
                return functor(css::SpaceSeparatedTuple {css::keywords::Row {}, css::keywords::Dense {}});
              }
              return functor(css::keywords::Dense {});
            case GridAutoFlow::Packing::Sparse: return functor(css::keywords::Row {});
          }
          [[fallthrough]];
        default:
          krys_debug_assert(state.Style.Display() != DisplayType::GridLanes
                            && state.Style.Display() != DisplayType::InlineGridLanes
                            && state.Style.Display() != DisplayType::Grid
                            && state.Style.Display() != DisplayType::InlineGrid);
          switch (gridFlow.GetPacking())
          {
            case GridAutoFlow::Packing::Dense:  return functor(css::keywords::Dense {});
            case GridAutoFlow::Packing::Sparse: return functor(css::keywords::Normal {});
          }
      }
      krys_unreachable();
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::Rotate>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      if (Is<RenderInline>(state.Renderer))
      {
        return functor(css::keywords::None {});
      }
      return functor(state.Style.GetRotate());
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::Scale>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      if (Is<RenderInline>(state.Renderer))
      {
        return functor(css::keywords::None {});
      }
      return functor(state.Style.GetScale());
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::Translate>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      if (Is<RenderInline>(state.Renderer))
      {
        return functor(css::keywords::None {});
      }
      return functor(state.Style.GetTranslate());
    }
  };

  // FIXME: if 'auto' value is removed then this can likely also be removed.
  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::Widows>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      return functor(state.Style.GetWidows().TryValue().value_or(2));
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::Orphans>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      return functor(state.Style.GetOrphans().TryValue().value_or(2));
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::WebkitTextCombine>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      auto textCombine = state.Style.GetTextCombine();
      if (textCombine == TextCombine::All)
      {
        return functor(css::keywords::Horizontal {});
      }
      return functor(textCombine);
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::WebkitRubyPosition>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      switch (state.Style.GetRubyPosition())
      {
        case RubyPosition::Over:                 return functor(css::keywords::Before {});
        case RubyPosition::Under:                return functor(css::keywords::After {});
        case RubyPosition::InterCharacter:
        case RubyPosition::LegacyInterCharacter: return functor(css::keywords::InterCharacter {});
      }
      krys_unreachable();
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::BlockStep>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      auto blockStepSize = state.Style.GetBlockStepSize();
      bool hasBlockStepSize = blockStepSize != ComputedStyle::InitialBlockStepSize();
      auto blockStepSizeValue = [&]() -> Maybe<BlockStepSize>
      {
        return hasBlockStepSize ? Maybe<BlockStepSize>(blockStepSize) : null;
      };

      auto blockStepInsert = state.Style.GetBlockStepInsert();
      bool hasBlockStepInsert = blockStepInsert != ComputedStyle::InitialBlockStepInsert();
      auto blockStepInsertValue = [&]() -> Maybe<BlockStepInsert>
      {
        return hasBlockStepInsert ? Maybe<BlockStepInsert>(blockStepInsert) : null;
      };

      auto blockStepAlign = state.Style.GetBlockStepAlign();
      bool hasBlockStepAlign = blockStepAlign != ComputedStyle::InitialBlockStepAlign();
      auto blockStepAlignValue = [&]() -> Maybe<BlockStepAlign>
      {
        return hasBlockStepAlign ? Maybe<BlockStepAlign>(blockStepAlign) : null;
      };

      auto blockStepRound = state.Style.GetBlockStepRound();
      bool hasBlockStepRound = blockStepRound != ComputedStyle::InitialBlockStepRound();
      auto blockStepRoundValue = [&]() -> Maybe<BlockStepRound>
      {
        return hasBlockStepRound ? Maybe<BlockStepRound>(blockStepRound) : null;
      };

      if (!hasBlockStepSize && !hasBlockStepInsert && !hasBlockStepAlign && !hasBlockStepRound)
      {
        return functor(css::keywords::None {});
      }

      return functor(css::SpaceSeparatedTuple {blockStepSizeValue(), blockStepInsertValue(),
                                               blockStepAlignValue(), blockStepRoundValue()});
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::FontSynthesis>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      auto &description = state.Style.GetFontDescription();

      bool hasWeight = description.HasAutoFontSynthesisWeight();
      auto weightValue = [&]() -> Maybe<css::keywords::Weight>
      {
        return hasWeight ? Maybe<css::keywords::Weight>(css::keywords::Weight {}) : null;
      };

      bool hasStyle = description.HasAutoFontSynthesisStyle();
      auto styleValue = [&]() -> Maybe<css::keywords::Style>
      {
        return hasStyle ? Maybe<css::keywords::Style>(css::keywords::Style {}) : null;
      };

      bool hasSmallCaps = description.HasAutoFontSynthesisSmallCaps();
      auto smallCapsValue = [&]() -> Maybe<css::keywords::SmallCaps>
      {
        return hasSmallCaps ? Maybe<css::keywords::SmallCaps>(css::keywords::SmallCaps {}) : null;
      };

      if (!hasWeight && !hasStyle && !hasSmallCaps)
      {
        return functor(css::keywords::None {});
      }
      return functor(css::SpaceSeparatedTuple {weightValue(), styleValue(), smallCapsValue()});
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::LineClamp>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      auto maxLines = state.Style.MaxLines().TryValue();
      if (!maxLines)
      {
        return functor(css::keywords::None {});
      }
      return functor(css::SpaceSeparatedTuple {*maxLines, state.Style.GetBlockEllipsis()});
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::MaskBorder>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      return functor(state.Style.GetMaskBorder());
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::OverscrollBehavior>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      return functor(std::max(state.Style.OverscrollBehaviorX(), state.Style.OverscrollBehaviorY()));
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::PerspectiveOrigin>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      if (state.Renderer)
      {
        auto box = state.Renderer->TransformReferenceBoxRect(state.Style);

        auto perspectiveOriginX =
          Length<> {Evaluate<float>(state.Style.GetPerspectiveOriginX(), box.Width(), ZoomNeeded {})};
        auto perspectiveOriginY =
          Length<> {Evaluate<float>(state.Style.GetPerspectiveOriginY(), box.Height(), ZoomNeeded {})};

        return functor(css::SpaceSeparatedTuple {perspectiveOriginX, perspectiveOriginY});
      }

      return functor(
        css::SpaceSeparatedTuple {state.Style.GetPerspectiveOriginX(), state.Style.GetPerspectiveOriginY()});
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::TextBox>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      auto textBoxTrim = state.Style.GetTextBoxTrim();
      auto textBoxEdge = state.Style.GetTextBoxEdge();

      auto hasDefaultTextBoxTrim = textBoxTrim == ComputedStyle::InitialTextBoxTrim();
      auto hasDefaultTextBoxEdge = textBoxEdge == ComputedStyle::InitialTextBoxEdge();

      if (hasDefaultTextBoxTrim && hasDefaultTextBoxEdge)
      {
        return functor(css::keywords::Normal {});
      }
      if (hasDefaultTextBoxEdge)
      {
        return functor(textBoxTrim);
      }
      if (textBoxTrim == TextBoxTrim::TrimBoth)
      {
        return functor(textBoxEdge);
      }

      return functor(css::SpaceSeparatedTuple {textBoxTrim, textBoxEdge});
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::TextDecoration>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      auto textDecorationLine = state.Style.GetTextDecorationLine();
      bool hasTextDecorationLine = textDecorationLine != ComputedStyle::InitialTextDecorationLine();
      auto textDecorationLineValue = [&]() -> Maybe<TextDecorationLine>
      {
        return hasTextDecorationLine ? Maybe<TextDecorationLine>(textDecorationLine) : null;
      };

      auto textDecorationThickness = state.Style.GetTextDecorationThickness();
      bool hasTextDecorationThickness =
        state.Style.GetTextDecorationThickness() != ComputedStyle::InitialTextDecorationThickness();
      auto textDecorationThicknessValue = [&]() -> Maybe<TextDecorationThickness>
      {
        return hasTextDecorationThickness ? Maybe<TextDecorationThickness>(textDecorationThickness) : null;
      };

      auto textDecorationStyle = state.Style.GetTextDecorationStyle();
      bool hasTextDecorationStyle =
        state.Style.GetTextDecorationStyle() != ComputedStyle::InitialTextDecorationStyle();
      auto textDecorationStyleValue = [&]() -> Maybe<TextDecorationStyle>
      {
        return hasTextDecorationStyle ? Maybe<TextDecorationStyle>(textDecorationStyle) : null;
      };

      auto textDecorationColor = state.Style.TextDecorationColor();
      bool hasTextDecorationColor = !textDecorationColor.IsCurrentColor();
      auto textDecorationColorValue = [&]() -> Maybe<style::Color>
      {
        return hasTextDecorationColor ? Maybe<style::Color>(textDecorationColor) : null;
      };

      if (!hasTextDecorationLine && !hasTextDecorationStyle && !hasTextDecorationColor
          && !hasTextDecorationThickness)
      {
        return functor(css::keywords::None {});
      }

      return functor(css::SpaceSeparatedTuple {textDecorationLineValue(), textDecorationThicknessValue(),
                                               textDecorationStyleValue(), textDecorationColorValue()});
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::TextWrap>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      auto textWrapMode = state.Style.GetTextWrapMode();
      auto textWrapStyle = state.Style.GetTextWrapStyle();

      // Omit default longhand values.
      if (textWrapStyle == ComputedStyle::InitialTextWrapStyle())
      {
        return functor(textWrapMode);
      }
      if (textWrapMode == ComputedStyle::InitialTextWrapMode())
      {
        return functor(textWrapStyle);
      }

      return functor(css::SpaceSeparatedTuple {textWrapMode, textWrapStyle});
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::TransformOrigin>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      if (state.Renderer)
      {
        auto box = state.Renderer->TransformReferenceBoxRect(state.Style);

        auto transformOriginX =
          Length<> {Evaluate<float>(state.Style.GetTransformOriginX(), box.Width(), ZoomNeeded {})};
        auto transformOriginY =
          Length<> {Evaluate<float>(state.Style.GetTransformOriginY(), box.Height(), ZoomNeeded {})};

        if (auto transformOriginZ = state.Style.GetTransformOriginZ(); !transformOriginZ.IsZero())
        {
          return functor(css::SpaceSeparatedTuple {transformOriginX, transformOriginY, transformOriginZ});
        }
        return functor(css::SpaceSeparatedTuple {transformOriginX, transformOriginY});
      }

      if (auto transformOriginZ = state.Style.GetTransformOriginZ(); !transformOriginZ.IsZero())
      {
        return functor(css::SpaceSeparatedTuple {state.Style.GetTransformOriginX(),
                                                 state.Style.GetTransformOriginY(), transformOriginZ});
      }
      return functor(
        css::SpaceSeparatedTuple {state.Style.GetTransformOriginX(), state.Style.GetTransformOriginY()});
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::WhiteSpace>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      auto whiteSpaceCollapse = state.Style.GetWhiteSpaceCollapse();
      auto textWrapMode = state.Style.GetTextWrapMode();

      // Convert to backwards-compatible keywords if possible.
      if (whiteSpaceCollapse == WhiteSpaceCollapse::Collapse && textWrapMode == TextWrapMode::Wrap)
      {
        return functor(css::keywords::Normal {});
      }
      if (whiteSpaceCollapse == WhiteSpaceCollapse::Preserve && textWrapMode == TextWrapMode::NoWrap)
      {
        return functor(css::keywords::Pre {});
      }
      if (whiteSpaceCollapse == WhiteSpaceCollapse::Preserve && textWrapMode == TextWrapMode::Wrap)
      {
        return functor(css::keywords::PreWrap {});
      }
      if (whiteSpaceCollapse == WhiteSpaceCollapse::PreserveBreaks && textWrapMode == TextWrapMode::Wrap)
      {
        return functor(css::keywords::PreLine {});
      }

      // Omit default longhand values.
      if (whiteSpaceCollapse == ComputedStyle::InitialWhiteSpaceCollapse())
      {
        return functor(textWrapMode);
      }
      if (textWrapMode == ComputedStyle::InitialTextWrapMode())
      {
        return functor(whiteSpaceCollapse);
      }

      return functor(css::SpaceSeparatedTuple {whiteSpaceCollapse, textWrapMode});
    }
  };

  template <>
  struct PropertyExtractorAdaptor<css::PropertyId::Color>
  {
    template <typename F>
    decltype(auto) ComputedValue(ExtractorState &state, F &&functor) const noexcept
    {
      return functor(style::Color {state.Style.GetColor()});
    }
  };

  // MARK: - Adaptor Invokers

  template <css::PropertyId propertyID>
  KRYS_NODISCARD Ref<css::Value> ExtractCSSValue(ExtractorState &state) noexcept
  {
    return PropertyExtractorAdaptor<propertyID> {}.ComputedValue(
      state, [&](auto &&value) -> Ref<css::Value> { return CreateCSSValue(state.Pool, state.Style, value); });
  }

  template <css::PropertyId propertyID>
  void ExtractSerialisation(ExtractorState &state, StringBuilder &builder,
                            const css::SerialisationContext &context) noexcept
  {
    PropertyExtractorAdaptor<propertyID> {}.ComputedValue(
      state, [&](auto &&value) { Serialise(builder, context, state.Style, value); });
  }

  // MARK: - Utilities

  template <css::PropertyId propertyID, typename TList, typename TMapper>
  KRYS_NODISCARD Ref<css::Value> ExtractCoordinatedValueListValue(ExtractorState &state, const TList &list,
                                                                  TMapper &&mapper) noexcept
  {
    using PropertyAccessor = CoordinatedValueListPropertyConstAccessor<propertyID>;

    css::ValueListBuilder resultListBuilder;

    if constexpr (TList::value_type::ComputedValueUsesUsedValues)
    {
      for (auto &value : list.UsedValues())
      {
        resultListBuilder.push_back(mapper(state, PropertyAccessor {value}.Get(), value, list));
      }
    }
    else
    {
      if (!list.IsInitial())
      {
        for (auto &value : list.ComputedValues())
        {
          if (!PropertyAccessor {value}.IsFilled())
          {
            resultListBuilder.push_back(mapper(state, PropertyAccessor {value}.Get(), value, list));
          }
        }
      }
      else
      {
        resultListBuilder.push_back(mapper(state, PropertyAccessor::Initial(), null, list));
      }
    }

    return css::ValueList::CreateCommaSeparated(krys::move(resultListBuilder));
  }

  template <css::PropertyId propertyID, typename TList, typename TMapper>
  void ExtractCoordinatedValueListSerialisation(ExtractorState &state, StringBuilder &builder,
                                                const css::SerialisationContext &context, const TList &list,
                                                TMapper &&mapper) noexcept
  {
    using PropertyAccessor = CoordinatedValueListPropertyConstAccessor<propertyID>;

    bool includeComma = false;

    if constexpr (TList::value_type::ComputedValueUsesUsedValues)
    {
      for (auto &value : list.UsedValues())
      {
        if (includeComma)
        {
          builder.Append(u8", ");
        }
        mapper(state, builder, context, PropertyAccessor {value}.Get(), value, list);
        includeComma = true;
      }
    }
    else
    {
      if (!list.IsInitial())
      {
        for (auto &value : list.ComputedValues())
        {
          if (!PropertyAccessor {value}.IsFilled())
          {
            if (includeComma)
            {
              builder.Append(u8", ");
            }
            mapper(state, builder, context, PropertyAccessor {value}.Get(), value, list);
            includeComma = true;
          }
        }
      }
      else
      {
        mapper(state, builder, context, PropertyAccessor::Initial(), null, list);
      }
    }
  }

  template <css::PropertyId propertyID>
  KRYS_NODISCARD Maybe<int32> CounterDirectiveValue(const CounterDirectives &directives) noexcept
  {
    if constexpr (propertyID == css::PropertyId::CounterIncrement)
    {
      return directives.IncrementValue;
    }
    else if constexpr (propertyID == css::PropertyId::CounterReset)
    {
      return directives.ResetValue;
    }
    else if constexpr (propertyID == css::PropertyId::CounterSet)
    {
      return directives.SetValue;
    }
  }

  template <css::PropertyId propertyID>
  KRYS_NODISCARD Ref<css::Value> ExtractCounterValue(ExtractorState &state) noexcept
  {
    auto &map = state.Style.GetCounterDirectives().Map;
    if (map.empty())
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
    }

    css::ValueListBuilder list;
    for (auto &[key, value] : map)
    {
      if (auto number = CounterDirectiveValue<propertyID>(value))
      {
        list.push_back(CreateCSSValue(state.Pool, state.Style, css::CustomIdentifier {key}));
        list.push_back(CreateCSSValue(state.Pool, state.Style, Integer<> {*number}));
      }
    }
    if (!list.empty())
    {
      return css::ValueList::CreateSpaceSeparated(krys::move(list));
    }
    return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
  }

  template <css::PropertyId propertyID>
  void ExtractCounterSerialisation(ExtractorState &state, StringBuilder &builder,
                                   const css::SerialisationContext &context) noexcept
  {
    auto &map = state.Style.GetCounterDirectives().Map;
    if (map.empty())
    {
      Serialise(builder, context, state.Style, css::keywords::None {});
      return;
    }

    bool listEmpty = true;

    for (auto &[key, value] : map)
    {
      if (auto number = CounterDirectiveValue<propertyID>(value))
      {
        if (!listEmpty)
        {
          builder.Append(u8' ');
        }

        Serialise(builder, context, state.Style, css::CustomIdentifier {key});
        builder.Append(u8' ');
        Serialise(builder, context, state.Style, Integer<> {*number});

        listEmpty = false;
      }
    }

    if (listEmpty)
    {
      Serialise(builder, context, state.Style, css::keywords::None {});
    }
  }

  template <GridTrackSizingDirection direction>
  KRYS_NODISCARD Ref<css::Value> ExtractGridTemplateValue(ExtractorState &state) noexcept
  {
    auto addValuesForNamedGridLinesAtIndex = [](auto &list, auto &collector, auto i, auto renderEmpty)
    {
      if (collector.IsEmpty() && !renderEmpty)
      {
        return;
      }

      List<css::CSSOMString> lineNames;
      collector.CollectLineNamesForIndex(lineNames, static_cast<uint32>(i));
      if (!lineNames.empty() || renderEmpty)
      {
        list.push_back(css::GridLineNamesValue::Create(lineNames));
      }
    };

    auto &tracks = state.Style.GridTemplateList(direction);

    const auto *renderGrid = DynamicDowncast<RenderGrid>(state.Renderer);

    auto &trackSizes = tracks.Sizes;
    auto &autoRepeatTrackSizes = tracks.AutoRepeatSizes;

    // Handle the 'none' case.
    bool trackListIsEmpty = trackSizes.empty() && autoRepeatTrackSizes.empty();
    if (renderGrid && trackListIsEmpty)
    {
      // For grids we should consider every listed track, whether implicitly or explicitly
      // created. Empty grids have a sole grid line per axis.
      auto &positions = renderGrid->Positions(direction);
      trackListIsEmpty = positions.size() == 1;
    }

    bool isSubgrid = tracks.Subgrid;

    if (trackListIsEmpty && !isSubgrid)
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
    }

    css::ValueListBuilder list;

    // If the element is a grid container, the resolved value is the used value,
    // specifying track sizes in pixels and expanding the repeat() notation.
    // If subgrid was specified, but the element isn't a subgrid (due to not having
    // an appropriate grid parent), then we fall back to using the specified value.
    if (renderGrid && (!isSubgrid || renderGrid->IsSubgrid(direction)))
    {
      if (isSubgrid)
      {
        list.push_back(CreateCSSValue(state.Pool, state.Style, css::keywords::Subgrid {}));

        OrderedNamedLinesCollectorInSubgridLayout collector(state, tracks, renderGrid->NumTracks(direction));
        for (int32 i = 0; i < collector.NamedGridLineCount(); i++)
        {
          addValuesForNamedGridLinesAtIndex(list, collector, i, true);
        }
        return css::ValueList::CreateSpaceSeparated(krys::move(list));
      }

      OrderedNamedLinesCollectorInGridLayout collector(state, tracks,
                                                       renderGrid->AutoRepeatCountForDirection(direction),
                                                       static_cast<uint32>(autoRepeatTrackSizes.size()));
      auto computedTrackSizes = renderGrid->TrackSizesForComputedStyle(direction);
      // Named grid line indices are relative to the explicit grid, but we are including all tracks.
      // So we need to subtract the number of leading implicit tracks in order to get the proper line index.
      int32 offset = -renderGrid->ExplicitGridStartForDirection(direction);

      int32 start = 0;
      auto end = static_cast<int32>(computedTrackSizes.size());
      krys_debug_assert(start <= end);
      for (int32 i = start; i < end; ++i)
      {
        if (i + offset >= 0)
        {
          addValuesForNamedGridLinesAtIndex(list, collector, i + offset, false);
        }
        list.push_back(CreateCSSValue(state.Pool, state.Style, Length<> {computedTrackSizes[i]}));
      }
      if (end + offset >= 0)
      {
        addValuesForNamedGridLinesAtIndex(list, collector, end + offset, false);
      }
      return css::ValueList::CreateSpaceSeparated(krys::move(list));
    }

    // Otherwise, the resolved value is the computed value, preserving repeat().
    auto &computedTracks = tracks.Tracks;

    auto repeatVisitor = [&](css::ValueListBuilder &list, const RepeatEntry &entry)
    {
      if (std::holds_alternative<List<css::CSSOMString>>(entry))
      {
        const auto &names = std::get<List<css::CSSOMString>>(entry);
        if (names.empty() && !isSubgrid)
        {
          return;
        }
        list.push_back(css::GridLineNamesValue::Create(names));
      }
      else
      {
        list.push_back(CreateCSSValue(state.Pool, state.Style, std::get<GridTrackSize>(entry)));
      }
    };

    for (auto &entry : computedTracks)
    {
      krys::SwitchOn(
        entry,
        [&](const GridTrackSize &size) { list.push_back(CreateCSSValue(state.Pool, state.Style, size)); },
        [&](const List<css::CSSOMString> &names)
        {
          // Subgrids don't have track sizes specified, so empty line names sets
          // need to be serialised, as they are meaningful placeholders.
          if (names.empty() && !isSubgrid)
          {
            return;
          }
          list.push_back(css::GridLineNamesValue::Create(names));
        },
        [&](const GridTrackEntryRepeat &repeat)
        {
          css::ValueListBuilder repeatedValues;
          for (auto &entry : repeat.Tracks)
          {
            repeatVisitor(repeatedValues, entry);
          }
          list.push_back(css::GridIntegerRepeatValue::Create(
            css::PrimitiveValue::CreateInteger(repeat.Repeats), krys::move(repeatedValues)));
        },
        [&](const GridTrackEntryAutoRepeat &repeat)
        {
          css::ValueListBuilder repeatedValues;
          for (auto &entry : repeat.Tracks)
          {
            repeatVisitor(repeatedValues, entry);
          }
          list.push_back(css::GridAutoRepeatValue::Create(
            repeat.Type == AutoRepeatType::Fill ? css::ValueId::AutoFill : css::ValueId::AutoFit,
            krys::move(repeatedValues)));
        },
        [&](const GridTrackEntrySubgrid &)
        { list.push_back(CreateCSSValue(state.Pool, state.Style, css::keywords::Subgrid {})); });
    }

    return css::ValueList::CreateSpaceSeparated(krys::move(list));
  }

  template <GridTrackSizingDirection direction>
  void ExtractGridTemplateSerialisation(ExtractorState &state, StringBuilder &builder,
                                        const css::SerialisationContext &context) noexcept
  {
    // FIXME: Do this more efficiently without creating and destroying a css::Value object.
    builder.Append(ExtractGridTemplateValue<direction>(state)->CssText(context));
  }

  // MARK: Shorthand Utilities

  /// @brief Appends each item to `builder`, separated by `separator`.
  template <typename TRange, typename TFunc>
  void AppendInterleaved(StringBuilder &builder, const TRange &range, TFunc &&func,
                         css::CSSOMStringView separator) noexcept
  {
    bool first = true;
    for (const auto &item : range)
    {
      if (!first)
      {
        builder.Append(separator);
      }
      first = false;
      func(builder, item);
    }
  }

  KRYS_NODISCARD inline bool CompareCSSValuePtr(const RefPtr<css::Value> &a,
                                                const RefPtr<css::Value> &b) noexcept
  {
    if (a && b)
    {
      return a->Equals(*b);
    }
    return !a && !b;
  }

  KRYS_NODISCARD inline Ref<css::Value>
    ExtractSingleShorthand(ExtractorState &state, const css::PropertyShorthand &shorthand) noexcept
  {
    krys_debug_assert(shorthand.Size() == 1);
    return Ref<css::Value>(ExtractorGenerated::ExtractValue(state, *shorthand.begin()));
  }

  inline void ExtractSingleShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                  const css::SerialisationContext &context,
                                                  const css::PropertyShorthand &shorthand) noexcept
  {
    krys_debug_assert(shorthand.Size() == 1);
    ExtractorGenerated::ExtractValueSerialisation(state, builder, context, *shorthand.begin());
  }

  KRYS_NODISCARD inline Ref<css::ValueList>
    ExtractStandardSpaceSeparatedShorthand(ExtractorState &state,
                                           const css::PropertyShorthand &shorthand) noexcept
  {
    css::ValueListBuilder list;
    for (auto longhand : shorthand)
    {
      list.push_back(Ref<css::Value>(ExtractorGenerated::ExtractValue(state, longhand)));
    }
    return css::ValueList::CreateSpaceSeparated(krys::move(list));
  }

  inline void
    ExtractStandardSpaceSeparatedShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                        const css::SerialisationContext &context,
                                                        const css::PropertyShorthand &shorthand) noexcept
  {
    AppendInterleaved(
      builder, shorthand, [&](auto &builder, const auto &longhand)
      { ExtractorGenerated::ExtractValueSerialisation(state, builder, context, longhand); }, u8" ");
  }

  KRYS_NODISCARD inline Ref<css::Value>
    ExtractStandardSlashSeparatedShorthand(ExtractorState &state,
                                           const css::PropertyShorthand &shorthand) noexcept
  {
    css::ValueListBuilder builder;
    for (auto longhand : shorthand)
    {
      builder.push_back(Ref<css::Value>(ExtractorGenerated::ExtractValue(state, longhand)));
    }
    return css::ValueList::CreateSlashSeparated(krys::move(builder));
  }

  inline void
    ExtractStandardSlashSeparatedShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                        const css::SerialisationContext &context,
                                                        const css::PropertyShorthand &shorthand) noexcept
  {
    AppendInterleaved(
      builder, shorthand, [&](auto &builder, const auto &longhand)
      { ExtractorGenerated::ExtractValueSerialisation(state, builder, context, longhand); }, u8" / ");
  }

  KRYS_NODISCARD inline RefPtr<css::Value>
    ExtractCoalescingPairShorthand(ExtractorState &state, const css::PropertyShorthand &shorthand) noexcept
  {
    // Assume the properties are in the usual order start, end.
    auto longhands = shorthand.Properties();
    auto startValue = ExtractorGenerated::ExtractValue(state, longhands[0]);
    auto endValue = ExtractorGenerated::ExtractValue(state, longhands[1]);

    // All 2 properties must be specified.
    if (!startValue || !endValue)
    {
      return nullptr;
    }

    return css::ValuePair::Create(Ref<css::Value>(krys::move(startValue)),
                                  Ref<css::Value>(krys::move(endValue)));
  }

  inline void ExtractCoalescingPairShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                          const css::SerialisationContext &context,
                                                          const css::PropertyShorthand &shorthand) noexcept
  {
    auto longhands = shorthand.Properties();

    auto offsetBeforeFirst = builder.Length();
    ExtractorGenerated::ExtractValueSerialisation(state, builder, context, longhands[0]);
    auto offsetAfterFirst = builder.Length();

    if (offsetBeforeFirst == offsetAfterFirst)
    {
      return;
    }

    builder.Append(u8' ');

    auto offsetBeforeSecond = builder.Length();
    ExtractorGenerated::ExtractValueSerialisation(state, builder, context, longhands[1]);
    auto offsetAfterSecond = builder.Length();

    if (offsetBeforeSecond == offsetAfterSecond)
    {
      builder.Shrink(offsetBeforeFirst);
      return;
    }

    css::CSSOMStringView stringView = builder;
    auto stringViewFirst = stringView.substr(offsetBeforeFirst, offsetAfterFirst - offsetBeforeFirst);
    auto stringViewSecond = stringView.substr(offsetBeforeSecond, offsetAfterSecond - offsetBeforeSecond);

    // If the two longhands serialised to the same value, shrink the builder to right after the first
    // longhand.
    if (stringViewFirst == stringViewSecond)
    {
      builder.Shrink(offsetAfterFirst);
    }
  }

  KRYS_NODISCARD inline RefPtr<css::Value>
    ExtractCoalescingQuadShorthand(ExtractorState &state, const css::PropertyShorthand &shorthand) noexcept
  {
    // Assume the properties are in the usual order top, right, bottom, left.
    auto longhands = shorthand.Properties();
    auto topValue = ExtractorGenerated::ExtractValue(state, longhands[0]);
    auto rightValue = ExtractorGenerated::ExtractValue(state, longhands[1]);
    auto bottomValue = ExtractorGenerated::ExtractValue(state, longhands[2]);
    auto leftValue = ExtractorGenerated::ExtractValue(state, longhands[3]);

    // All 4 properties must be specified.
    if (!topValue || !rightValue || !bottomValue || !leftValue)
    {
      return nullptr;
    }

    bool showLeft = !CompareCSSValuePtr(rightValue, leftValue);
    bool showBottom = !CompareCSSValuePtr(topValue, bottomValue) || showLeft;
    bool showRight = !CompareCSSValuePtr(topValue, rightValue) || showBottom;

    css::ValueListBuilder list;
    list.push_back(Ref<css::Value>(krys::move(topValue)));
    if (showRight)
    {
      list.push_back(Ref<css::Value>(krys::move(rightValue)));
    }
    if (showBottom)
    {
      list.push_back(Ref<css::Value>(krys::move(bottomValue)));
    }
    if (showLeft)
    {
      list.push_back(Ref<css::Value>(krys::move(leftValue)));
    }
    return css::ValueList::CreateSpaceSeparated(krys::move(list));
  }

  inline void ExtractCoalescingQuadShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                          const css::SerialisationContext &context,
                                                          const css::PropertyShorthand &shorthand) noexcept
  {
    auto longhands = shorthand.Properties();

    // All 4 properties must be specified.

    auto offsetBeforeTop = builder.Length();
    ExtractorGenerated::ExtractValueSerialisation(state, builder, context, longhands[0]);
    auto offsetAfterTop = builder.Length();
    if (offsetBeforeTop == offsetAfterTop)
    {
      return;
    }

    builder.Append(u8' ');

    auto offsetBeforeRight = builder.Length();
    ExtractorGenerated::ExtractValueSerialisation(state, builder, context, longhands[1]);
    auto offsetAfterRight = builder.Length();
    if (offsetBeforeRight == offsetAfterRight)
    {
      builder.Shrink(offsetBeforeTop);
      return;
    }

    builder.Append(u8' ');

    auto offsetBeforeBottom = builder.Length();
    ExtractorGenerated::ExtractValueSerialisation(state, builder, context, longhands[2]);
    auto offsetAfterBottom = builder.Length();
    if (offsetBeforeBottom == offsetAfterBottom)
    {
      builder.Shrink(offsetBeforeTop);
      return;
    }

    builder.Append(u8' ');

    auto offsetBeforeLeft = builder.Length();
    ExtractorGenerated::ExtractValueSerialisation(state, builder, context, longhands[3]);
    auto offsetAfterLeft = builder.Length();
    if (offsetBeforeLeft == offsetAfterLeft)
    {
      builder.Shrink(offsetBeforeTop);
      return;
    }

    css::CSSOMStringView stringView = builder;
    auto stringViewTop = stringView.substr(offsetBeforeTop, offsetAfterTop - offsetBeforeTop);
    auto stringViewRight = stringView.substr(offsetBeforeRight, offsetAfterRight - offsetBeforeRight);
    auto stringViewBottom = stringView.substr(offsetBeforeBottom, offsetAfterBottom - offsetBeforeBottom);
    auto stringViewLeft = stringView.substr(offsetBeforeLeft, offsetAfterLeft - offsetBeforeLeft);

    // Include everything.
    if (stringViewRight != stringViewLeft)
    {
      return;
    }

    // Shrink to include top, right and bottom.
    if (stringViewBottom != stringViewTop)
    {
      builder.Shrink(offsetAfterBottom);
      return;
    }

    // Shrink to include top and right.
    if (stringViewRight != stringViewTop)
    {
      builder.Shrink(offsetAfterRight);
      return;
    }

    // Shrink to just include top.
    builder.Shrink(offsetAfterTop);
  }

  KRYS_NODISCARD inline RefPtr<css::Value>
    ExtractBorderShorthand(ExtractorState &state, Span<const css::PropertyId> sections) noexcept
  {
    auto value = ExtractorGenerated::ExtractValue(state, sections[0]);
    for (auto &section : sections.subspan(1))
    {
      if (!CompareCSSValuePtr(value, ExtractorGenerated::ExtractValue(state, section)))
      {
        return nullptr;
      }
    }
    return value;
  }

  inline void ExtractBorderShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                  const css::SerialisationContext &context,
                                                  Span<const css::PropertyId> sections) noexcept
  {
    // FIXME: Do this more efficiently without creating and destroying a css::Value object.
    if (auto value = ExtractBorderShorthand(state, sections))
    {
      builder.Append(value->CssText(context));
    }
  }

  KRYS_NODISCARD inline Ref<css::Value> ExtractBorderRadiusShorthand(ExtractorState &state) noexcept
  {
    auto itemsEqual = [](const css::ValueListBuilder &a, const css::ValueListBuilder &b) -> bool
    {
      auto size = a.size();
      if (size != b.size())
      {
        return false;
      }
      for (size_t i = 0; i < size; ++i)
      {
        if (!a[i]->Equals(*b[i]))
        {
          return false;
        }
      }
      return true;
    };

    auto extractBorderRadiusCornerValues = [&](auto &state, const auto &radius)
    {
      auto x = CreateCSSValue(state.Pool, state.Style, radius.Width());
      auto y =
        radius.Width() == radius.Height() ? x : CreateCSSValue(state.Pool, state.Style, radius.Height());
      return Pair<Ref<css::Value>, Ref<css::Value>> {krys::move(x), krys::move(y)};
    };

    bool showHorizontalBottomLeft =
      state.Style.BorderTopRightRadius().Width() != state.Style.BorderBottomLeftRadius().Width();
    bool showHorizontalBottomRight =
      showHorizontalBottomLeft
      || (state.Style.BorderBottomRightRadius().Width() != state.Style.BorderTopLeftRadius().Width());
    bool showHorizontalTopRight =
      showHorizontalBottomRight
      || (state.Style.BorderTopRightRadius().Width() != state.Style.BorderTopLeftRadius().Width());

    bool showVerticalBottomLeft =
      state.Style.BorderTopRightRadius().Height() != state.Style.BorderBottomLeftRadius().Height();
    bool showVerticalBottomRight =
      showVerticalBottomLeft
      || (state.Style.BorderBottomRightRadius().Height() != state.Style.BorderTopLeftRadius().Height());
    bool showVerticalTopRight =
      showVerticalBottomRight
      || (state.Style.BorderTopRightRadius().Height() != state.Style.BorderTopLeftRadius().Height());

    auto [topLeftRadiusX, topLeftRadiusY] =
      extractBorderRadiusCornerValues(state, state.Style.BorderTopLeftRadius());
    auto [topRightRadiusX, topRightRadiusY] =
      extractBorderRadiusCornerValues(state, state.Style.BorderTopRightRadius());
    auto [bottomRightRadiusX, bottomRightRadiusY] =
      extractBorderRadiusCornerValues(state, state.Style.BorderBottomRightRadius());
    auto [bottomLeftRadiusX, bottomLeftRadiusY] =
      extractBorderRadiusCornerValues(state, state.Style.BorderBottomLeftRadius());

    css::ValueListBuilder horizontalRadii;
    horizontalRadii.push_back(krys::move(topLeftRadiusX));
    if (showHorizontalTopRight)
    {
      horizontalRadii.push_back(krys::move(topRightRadiusX));
    }
    if (showHorizontalBottomRight)
    {
      horizontalRadii.push_back(krys::move(bottomRightRadiusX));
    }
    if (showHorizontalBottomLeft)
    {
      horizontalRadii.push_back(krys::move(bottomLeftRadiusX));
    }

    css::ValueListBuilder verticalRadii;
    verticalRadii.push_back(krys::move(topLeftRadiusY));
    if (showVerticalTopRight)
    {
      verticalRadii.push_back(krys::move(topRightRadiusY));
    }
    if (showVerticalBottomRight)
    {
      verticalRadii.push_back(krys::move(bottomRightRadiusY));
    }
    if (showVerticalBottomLeft)
    {
      verticalRadii.push_back(krys::move(bottomLeftRadiusY));
    }

    if (itemsEqual(horizontalRadii, verticalRadii))
    {
      return css::ValueList::CreateSlashSeparated(
        css::ValueList::CreateSpaceSeparated(krys::move(horizontalRadii)));
    }
    return css::ValueList::CreateSlashSeparated(
      css::ValueList::CreateSpaceSeparated(krys::move(horizontalRadii)),
      css::ValueList::CreateSpaceSeparated(krys::move(verticalRadii)));
  }

  template <css::PropertyId property>
  KRYS_NODISCARD Ref<css::Value> ExtractFillLayerPropertyShorthand(
    ExtractorState &state, const css::PropertyShorthand &propertiesBeforeSlashSeparator,
    const css::PropertyShorthand &propertiesAfterSlashSeparator, css::PropertyId lastLayerProperty) noexcept
  {
    static_assert(property == css::PropertyId::Background || property == css::PropertyId::Mask);

    auto computeRenderStyle = [&]() -> const RenderStyle *
    {
      // FIXME: Animations. Read the renderer's animated style when it's composited and the property is
      // accelerated.
      return state.Element->ComputedStyle(state.PseudoElementIdentifier);
    };

    auto layerCount = [&]() -> size_t
    {
      // FIXME: Why does this not use state.Style?

      const auto *style = computeRenderStyle();
      if (!style)
      {
        return 0;
      }

      const auto &layers = [&]() -> const auto &
      {
        if constexpr (property == css::PropertyId::Mask)
        {
          return style->GetMaskLayers();
        }
        else
        {
          return style->GetBackgroundLayers();
        }
      }();

      if constexpr (property == css::PropertyId::Mask)
      {
        if (layers.UsedLength() == 1 && !layers.UsedFirst().HasImage())
        {
          return 0;
        }
      }

      return layers.UsedLength();
    }();
    if (!layerCount)
    {
      krys_debug_assert(property == css::PropertyId::Mask);
      return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
    }

    auto lastValue = lastLayerProperty != css::PropertyId::Invalid
                       ? ExtractorGenerated::ExtractValue(state, lastLayerProperty)
                       : nullptr;
    auto before = ExtractStandardSpaceSeparatedShorthand(state, propertiesBeforeSlashSeparator);
    auto after = ExtractStandardSpaceSeparatedShorthand(state, propertiesAfterSlashSeparator);

    // The computed properties are returned as lists of properties, with a list of layers in each.
    // We want to swap that around to have a list of layers, with a list of properties in each.

    auto layerItem = [&](const css::Value &value, size_t i) -> Ref<css::Value>
    {
      const auto &item =
        layerCount == 1 ? value : *Downcast<css::ValueList>(value).Item(static_cast<uint32>(i));
      return ShareRef(const_cast<css::Value &>(item));
    };

    css::ValueListBuilder layers;
    for (size_t i = 0; i < layerCount; i++)
    {
      css::ValueListBuilder beforeList;
      if (i == layerCount - 1 && lastValue)
      {
        beforeList.push_back(Ref<css::Value>(lastValue));
      }
      for (size_t j = 0; j < propertiesBeforeSlashSeparator.Size(); j++)
      {
        beforeList.push_back(layerItem(*before->Item(static_cast<uint32>(j)), i));
      }
      css::ValueListBuilder afterList;
      for (size_t j = 0; j < propertiesAfterSlashSeparator.Size(); j++)
      {
        afterList.push_back(layerItem(*after->Item(static_cast<uint32>(j)), i));
      }
      auto list =
        css::ValueList::CreateSlashSeparated(css::ValueList::CreateSpaceSeparated(krys::move(beforeList)),
                                             css::ValueList::CreateSpaceSeparated(krys::move(afterList)));
      if (layerCount == 1)
      {
        return list;
      }
      layers.push_back(krys::move(list));
    }
    return css::ValueList::CreateCommaSeparated(krys::move(layers));
  }

  template <css::PropertyId property>
  void ExtractFillLayerPropertyShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context,
    const css::PropertyShorthand &propertiesBeforeSlashSeparator,
    const css::PropertyShorthand &propertiesAfterSlashSeparator, css::PropertyId lastLayerProperty) noexcept
  {
    // FIXME: Do this more efficiently without creating and destroying a css::Value object.
    builder.Append(ExtractFillLayerPropertyShorthand<property>(
                     state, propertiesBeforeSlashSeparator, propertiesAfterSlashSeparator, lastLayerProperty)
                     ->CssText(context));
  }

  // MARK: - Custom Extractors

#define KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(name)                                                           \
  inline Ref<css::Value> ExtractorCustom::Extract##name(ExtractorState &state) noexcept                      \
  {                                                                                                          \
    return ExtractCSSValue<css::PropertyId::name>(state);                                                    \
  }                                                                                                          \
                                                                                                             \
  inline void ExtractorCustom::Extract##name##Serialisation(                                                 \
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept        \
  {                                                                                                          \
    ExtractSerialisation<css::PropertyId::name>(state, builder, context);                                    \
  }

  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Direction)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(WritingMode)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Float)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Content)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(LetterSpacing)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(WordSpacing)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(LineHeight)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(FontFamily)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(FontSize)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Top)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Right)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Bottom)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Left)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(MarginTop)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(MarginRight)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(MarginBottom)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(MarginLeft)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(PaddingTop)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(PaddingRight)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(PaddingBottom)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(PaddingLeft)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Height)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Width)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(MaxHeight)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(MaxWidth)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(MinHeight)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(MinWidth)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(GridAutoFlow)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Translate)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Scale)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Rotate)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Widows)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(Orphans)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(WebkitTextCombine)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED(WebkitRubyPosition)

#undef KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED

  inline Ref<css::Value> ExtractorCustom::ExtractCounterIncrement(ExtractorState &state) noexcept
  {
    return ExtractCounterValue<css::PropertyId::CounterIncrement>(state);
  }

  inline void
    ExtractorCustom::ExtractCounterIncrementSerialisation(ExtractorState &state, StringBuilder &builder,
                                                          const css::SerialisationContext &context) noexcept
  {
    ExtractCounterSerialisation<css::PropertyId::CounterIncrement>(state, builder, context);
  }

  inline Ref<css::Value> ExtractorCustom::ExtractCounterReset(ExtractorState &state) noexcept
  {
    return ExtractCounterValue<css::PropertyId::CounterReset>(state);
  }

  inline void
    ExtractorCustom::ExtractCounterResetSerialisation(ExtractorState &state, StringBuilder &builder,
                                                      const css::SerialisationContext &context) noexcept
  {
    ExtractCounterSerialisation<css::PropertyId::CounterReset>(state, builder, context);
  }

  inline Ref<css::Value> ExtractorCustom::ExtractCounterSet(ExtractorState &state) noexcept
  {
    return ExtractCounterValue<css::PropertyId::CounterSet>(state);
  }

  inline void
    ExtractorCustom::ExtractCounterSetSerialisation(ExtractorState &state, StringBuilder &builder,
                                                    const css::SerialisationContext &context) noexcept
  {
    ExtractCounterSerialisation<css::PropertyId::CounterSet>(state, builder, context);
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractBorderImageWidth(ExtractorState &state) noexcept
  {
    auto &borderImageWidth = state.Style.GetBorderImageWidth();
    if (borderImageWidth.OverridesBorderWidths())
    {
      return nullptr;
    }
    return CreateCSSValue(state.Pool, state.Style, borderImageWidth);
  }

  inline void
    ExtractorCustom::ExtractBorderImageWidthSerialisation(ExtractorState &state, StringBuilder &builder,
                                                          const css::SerialisationContext &context) noexcept
  {
    auto &borderImageWidth = state.Style.GetBorderImageWidth();
    if (borderImageWidth.OverridesBorderWidths())
    {
      return;
    }
    Serialise(builder, context, state.Style, borderImageWidth);
  }

  inline Ref<css::Value> ExtractorCustom::ExtractTransform(ExtractorState &state) noexcept
  {
    if (!state.Style.HasTransform())
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
    }

    if (state.Renderer)
    {
      // FIXME: The render tree. The transform operation data should come from the renderer (its motion path
      // data and whether it's an SVG renderer).
      return css::TransformListValue::Create(CreateCSSValue(
        state.Pool, state.Style,
        TransformResolver::ComputeTransform(
          state.Style, TransformOperationData(state.Renderer->TransformReferenceBoxRect(state.Style)), {})));
    }

    // https://w3c.github.io/csswg-drafts/css-transforms-1/#serialization-of-the-computed-value
    // If we don't have a renderer, then the value should be "none" if we're asking for the
    // resolved value (such as when calling getComputedStyle()).
    if (state.ValueType == ExtractorState::PropertyValueType::Resolved)
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
    }

    return CreateCSSValue(state.Pool, state.Style, state.Style.GetTransform());
  }

  inline void
    ExtractorCustom::ExtractTransformSerialisation(ExtractorState &state, StringBuilder &builder,
                                                   const css::SerialisationContext &context) noexcept
  {
    if (!state.Style.HasTransform())
    {
      Serialise(builder, context, state.Style, css::keywords::None {});
      return;
    }

    if (state.Renderer)
    {
      // FIXME: The render tree, as in `ExtractTransform`.
      Serialise(
        builder, context, state.Style,
        TransformResolver::ComputeTransform(
          state.Style, TransformOperationData(state.Renderer->TransformReferenceBoxRect(state.Style)), {}));
      return;
    }

    // https://w3c.github.io/csswg-drafts/css-transforms-1/#serialization-of-the-computed-value
    // If we don't have a renderer, then the value should be "none" if we're asking for the
    // resolved value (such as when calling getComputedStyle()).
    if (state.ValueType == ExtractorState::PropertyValueType::Resolved)
    {
      Serialise(builder, context, state.Style, css::keywords::None {});
      return;
    }

    Serialise(builder, context, state.Style, state.Style.GetTransform());
  }

  inline Ref<css::Value> ExtractorCustom::ExtractGridTemplateColumns(ExtractorState &state) noexcept
  {
    return ExtractGridTemplateValue<GridTrackSizingDirection::Columns>(state);
  }

  inline void ExtractorCustom::ExtractGridTemplateColumnsSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    ExtractGridTemplateSerialisation<GridTrackSizingDirection::Columns>(state, builder, context);
  }

  inline Ref<css::Value> ExtractorCustom::ExtractGridTemplateRows(ExtractorState &state) noexcept
  {
    return ExtractGridTemplateValue<GridTrackSizingDirection::Rows>(state);
  }

  inline void
    ExtractorCustom::ExtractGridTemplateRowsSerialisation(ExtractorState &state, StringBuilder &builder,
                                                          const css::SerialisationContext &context) noexcept
  {
    ExtractGridTemplateSerialisation<GridTrackSizingDirection::Rows>(state, builder, context);
  }

  KRYS_NODISCARD inline Ref<css::Value>
    ConvertSingleAnimationDuration(ExtractorState &state, const SingleAnimationDuration &duration,
                                   const Maybe<Animation> &animation,
                                   const Animations &animationList) noexcept
  {
    auto animationListHasMultipleExplicitTimelines = [&]
    {
      if (animationList.ComputedLength() <= 1)
      {
        return false;
      }
      auto explicitTimelines = 0;
      for (auto &animation : animationList.ComputedValues())
      {
        if (animation.IsTimelineSet())
        {
          ++explicitTimelines;
        }
        if (explicitTimelines > 1)
        {
          return true;
        }
      }
      return false;
    };

    auto animationHasExplicitNonAutoTimeline = [&]
    {
      if (!animation || !animation->IsTimelineSet())
      {
        return false;
      }
      return !animation->GetTimeline().IsAuto();
    };

    // https://drafts.csswg.org/css-animations-2/#animation-duration
    // For backwards-compatibility with Level 1, when the computed value of animation-timeline is auto
    // (i.e. only one list value, and that value being auto), the resolved value of auto for
    // animation-duration is 0s whenever its used value would also be 0s.
    if (duration.IsAuto()
        && (animationListHasMultipleExplicitTimelines() || animationHasExplicitNonAutoTimeline()))
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::Auto {});
    }
    return CreateCSSValue(state.Pool, state.Style,
                          duration.TryTime().value_or(SingleAnimationDuration::Time {0}));
  }

  inline Ref<css::Value> ExtractorCustom::ExtractAnimationDuration(ExtractorState &state) noexcept
  {
    auto mapper = [](auto &state, const auto &value, const Maybe<Animation> &animation,
                     const auto &animations) -> Ref<css::Value>
    {
      return ConvertSingleAnimationDuration(state, value, animation, animations);
    };
    return ExtractCoordinatedValueListValue<css::PropertyId::AnimationDuration>(
      state, state.Style.GetAnimations(), mapper);
  }

  inline void
    ExtractorCustom::ExtractAnimationDurationSerialisation(ExtractorState &state, StringBuilder &builder,
                                                           const css::SerialisationContext &context) noexcept
  {
    auto mapper = [&](auto &state, auto &builder, const auto &context, const auto &value,
                      const Maybe<Animation> &animation, const auto &animations)
    {
      // FIXME: Do this more efficiently without creating and destroying a css::Value object.
      builder.Append(ConvertSingleAnimationDuration(state, value, animation, animations)->CssText(context));
    };
    ExtractCoordinatedValueListSerialisation<css::PropertyId::AnimationDuration>(
      state, builder, context, state.Style.GetAnimations(), mapper);
  }

  inline Ref<css::Value> ExtractorCustom::ExtractColor(ExtractorState &state) noexcept
  {
    if (state.AllowVisitedStyle)
    {
      return state.Pool.CreateColorValue(state.Style.VisitedDependentColor(css::PropertyId::Color));
    }
    return ExtractCSSValue<css::PropertyId::Color>(state);
  }

  inline void ExtractorCustom::ExtractColorSerialisation(ExtractorState &state, StringBuilder &builder,
                                                         const css::SerialisationContext &context) noexcept
  {
    if (state.AllowVisitedStyle)
    {
      builder.Append(SerialisationForCSS(state.Style.VisitedDependentColor(css::PropertyId::Color)));
      return;
    }
    ExtractSerialisation<css::PropertyId::Color>(state, builder, context);
  }

  // MARK: - Shorthands

  KRYS_NODISCARD inline Ref<css::Value> ConvertSingleAnimation(ExtractorState &state,
                                                               const Animation &animation,
                                                               const Animations &animations) noexcept
  {
    static const EasingFunction initialTimingFunction(Animation::InitialTimingFunction());

    // If we have an animation-delay but no animation-duration set, we must serialise
    // the animation-duration because they're both <time> values and animation-delay
    // comes first.
    auto showsDelay = animation.GetDelay() != Animation::InitialDelay();
    auto showsDuration = showsDelay || animation.GetDuration() != Animation::InitialDuration();

    auto name = [&]() -> css::CSSOMStringView
    {
      if (auto keyframesName = animation.GetName().TryKeyframesName())
      {
        return keyframesName->Name.View();
      }
      return {};
    }();

    auto showsTimingFunction = [&]
    {
      if (animation.GetTimingFunction() != initialTimingFunction)
      {
        return true;
      }
      return name == u8"ease" || name == u8"ease-in" || name == u8"ease-in-out" || name == u8"ease-out"
             || name == u8"linear" || name == u8"step-end" || name == u8"step-start";
    };

    auto showsIterationCount = [&]
    {
      if (animation.GetIterationCount() != Animation::InitialIterationCount())
      {
        return true;
      }
      return name == u8"infinite";
    };

    auto showsDirection = [&]
    {
      if (animation.GetDirection() != Animation::InitialDirection())
      {
        return true;
      }
      return name == u8"normal" || name == u8"reverse" || name == u8"alternate"
             || name == u8"alternate-reverse";
    };

    auto showsFillMode = [&]
    {
      if (animation.GetFillMode() != Animation::InitialFillMode())
      {
        return true;
      }
      return name == u8"forwards" || name == u8"backwards" || name == u8"both";
    };

    auto showsPlaysState = [&]
    {
      if (animation.GetPlayState() != Animation::InitialPlayState())
      {
        return true;
      }
      return name == u8"running" || name == u8"paused";
    };

    css::ValueListBuilder list;
    if (showsDuration)
    {
      list.push_back(ConvertSingleAnimationDuration(state, animation.GetDuration(), animation, animations));
    }
    if (showsTimingFunction())
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, animation.GetTimingFunction()));
    }
    if (showsDelay)
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, animation.GetDelay()));
    }
    if (showsIterationCount())
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, animation.GetIterationCount()));
    }
    if (showsDirection())
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, animation.GetDirection()));
    }
    if (showsFillMode())
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, animation.GetFillMode()));
    }
    if (showsPlaysState())
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, animation.GetPlayState()));
    }
    if (animation.GetName() != Animation::InitialName())
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, animation.GetName()));
    }
    if (animation.GetTimeline() != Animation::InitialTimeline())
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, animation.GetTimeline()));
    }
    if (animation.GetCompositeOperation() != Animation::InitialCompositeOperation())
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, animation.GetCompositeOperation()));
    }
    if (list.empty())
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
    }
    return css::ValueList::CreateSpaceSeparated(krys::move(list));
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractAnimationShorthand(ExtractorState &state) noexcept
  {
    auto &animations = state.Style.GetAnimations();
    if (animations.IsInitial())
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
    }

    css::ValueListBuilder list;
    for (auto &animation : animations.ComputedValues())
    {
      // If any of the reset-only longhands are set, we cannot serialise this value.
      if (animation.IsTimelineSet() || animation.IsRangeStartSet() || animation.IsRangeEndSet())
      {
        list.clear();
        break;
      }
      list.push_back(ConvertSingleAnimation(state, animation, animations));
    }
    return css::ValueList::CreateCommaSeparated(krys::move(list));
  }

  inline void
    ExtractorCustom::ExtractAnimationShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                            const css::SerialisationContext &context) noexcept
  {
    auto &animations = state.Style.GetAnimations();
    if (animations.IsInitial())
    {
      Serialise(builder, context, state.Style, css::keywords::None {});
      return;
    }

    for (auto &animation : animations.ComputedValues())
    {
      // If any of the reset-only longhands are set, we cannot serialise this value.
      if (animation.IsTimelineSet() || animation.IsRangeStartSet() || animation.IsRangeEndSet())
      {
        return;
      }
    }

    AppendInterleaved(
      builder, animations.ComputedValues(),
      [&](auto &builder, const auto &animation)
      {
        // FIXME: Do this more efficiently without creating and destroying a css::Value object.
        builder.Append(ConvertSingleAnimation(state, animation, animations)->CssText(context));
      },
      u8", ");
  }

  KRYS_NODISCARD inline Ref<css::ValueList> ConvertAnimationRange(ExtractorState &state,
                                                                  const SingleAnimationRange &range) noexcept
  {
    css::ValueListBuilder list;

    auto createRangeValue = [&](auto &edge) -> Ref<css::ValueList>
    {
      Ref<css::Value> value = CreateCSSValue(state.Pool, state.Style, edge);
      if (auto *list = DynamicDowncast<css::ValueList>(value.get()))
      {
        return ShareRef(*list);
      }
      return css::ValueList::CreateSpaceSeparated(krys::move(value));
    };

    Ref<css::ValueList> startValue = createRangeValue(range.Start);
    Ref<css::ValueList> endValue = createRangeValue(range.End);
    bool endValueEqualsStart = startValue->Equals(*endValue);

    if (startValue->Length())
    {
      list.push_back(krys::move(startValue));
    }

    bool isNormal = range.End.IsNormal();
    bool isDefaultAndSameNameAsStart =
      range.Start.GetName() == range.End.GetName() && range.End.HasDefaultOffset();
    if (endValue->Length() && !endValueEqualsStart && !isNormal && !isDefaultAndSameNameAsStart)
    {
      list.push_back(krys::move(endValue));
    }

    return css::ValueList::CreateSpaceSeparated(krys::move(list));
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractAnimationRangeShorthand(ExtractorState &state) noexcept
  {
    auto mapper = [](auto &state, const auto &value, const Maybe<Animation> &,
                     const auto &) -> Ref<css::Value>
    {
      return ConvertAnimationRange(state, value);
    };
    return ExtractCoordinatedValueListValue<css::PropertyId::AnimationRange>(
      state, state.Style.GetAnimations(), mapper);
  }

  inline void ExtractorCustom::ExtractAnimationRangeShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    auto mapper = [&](auto &state, auto &builder, const auto &context, const auto &value,
                      const Maybe<Animation> &, const auto &)
    {
      // FIXME: Do this more efficiently without creating and destroying a css::Value object.
      builder.Append(ConvertAnimationRange(state, value)->CssText(context));
    };
    ExtractCoordinatedValueListSerialisation<css::PropertyId::AnimationRange>(
      state, builder, context, state.Style.GetAnimations(), mapper);
  }

  inline constexpr Array<css::PropertyId, 4> BackgroundPropertiesBeforeSlashSeparator {
    css::PropertyId::BackgroundImage, css::PropertyId::BackgroundRepeat,
    css::PropertyId::BackgroundAttachment, css::PropertyId::BackgroundPosition};
  inline constexpr Array<css::PropertyId, 3> BackgroundPropertiesAfterSlashSeparator {
    css::PropertyId::BackgroundSize, css::PropertyId::BackgroundOrigin, css::PropertyId::BackgroundClip};

  inline RefPtr<css::Value> ExtractorCustom::ExtractBackgroundShorthand(ExtractorState &state) noexcept
  {
    return ExtractFillLayerPropertyShorthand<css::PropertyId::Background>(
      state,
      css::PropertyShorthand(css::PropertyId::Background,
                             Span<const css::PropertyId, 4> {BackgroundPropertiesBeforeSlashSeparator}),
      css::PropertyShorthand(css::PropertyId::Background,
                             Span<const css::PropertyId, 3> {BackgroundPropertiesAfterSlashSeparator}),
      css::PropertyId::BackgroundColor);
  }

  inline void ExtractorCustom::ExtractBackgroundShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    ExtractFillLayerPropertyShorthandSerialisation<css::PropertyId::Background>(
      state, builder, context,
      css::PropertyShorthand(css::PropertyId::Background,
                             Span<const css::PropertyId, 4> {BackgroundPropertiesBeforeSlashSeparator}),
      css::PropertyShorthand(css::PropertyId::Background,
                             Span<const css::PropertyId, 3> {BackgroundPropertiesAfterSlashSeparator}),
      css::PropertyId::BackgroundColor);
  }

  inline RefPtr<css::Value>
    ExtractorCustom::ExtractBackgroundPositionShorthand(ExtractorState &state) noexcept
  {
    auto mapper = [](auto &state, const auto &value, const Maybe<BackgroundLayer> &,
                     const auto &) -> Ref<css::Value>
    {
      return CreateCSSValue(state.Pool, state.Style, value);
    };
    return ExtractCoordinatedValueListValue<css::PropertyId::BackgroundPosition>(
      state, state.Style.GetBackgroundLayers(), mapper);
  }

  inline void ExtractorCustom::ExtractBackgroundPositionShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    auto mapper = [](auto &state, auto &builder, const auto &context, const auto &value,
                     const Maybe<BackgroundLayer> &, const auto &)
    {
      Serialise(builder, context, state.Style, value);
    };
    ExtractCoordinatedValueListSerialisation<css::PropertyId::BackgroundPosition>(
      state, builder, context, state.Style.GetBackgroundLayers(), mapper);
  }

#define KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(name)                                                 \
  inline RefPtr<css::Value> ExtractorCustom::Extract##name##Shorthand(ExtractorState &state) noexcept        \
  {                                                                                                          \
    return ExtractCSSValue<css::PropertyId::name>(state);                                                    \
  }                                                                                                          \
                                                                                                             \
  inline void ExtractorCustom::Extract##name##ShorthandSerialisation(                                        \
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept        \
  {                                                                                                          \
    ExtractSerialisation<css::PropertyId::name>(state, builder, context);                                    \
  }

  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(BlockStep)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(FontSynthesis)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(LineClamp)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(MaskBorder)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(OverscrollBehavior)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(PageBreakAfter)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(PageBreakBefore)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(PageBreakInside)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(PerspectiveOrigin)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(TextBox)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(TextDecoration)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(TextWrap)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(TransformOrigin)
  KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND(WhiteSpace)

#undef KRYS_DEFINE_EXTRACTOR_CUSTOM_ADAPTED_SHORTHAND

  inline constexpr Array<css::PropertyId, 4> BorderSections {
    css::PropertyId::BorderTop, css::PropertyId::BorderRight, css::PropertyId::BorderBottom,
    css::PropertyId::BorderLeft};
  inline constexpr Array<css::PropertyId, 2> BorderBlockSections {css::PropertyId::BorderBlockStart,
                                                                  css::PropertyId::BorderBlockEnd};
  inline constexpr Array<css::PropertyId, 2> BorderInlineSections {css::PropertyId::BorderInlineStart,
                                                                   css::PropertyId::BorderInlineEnd};

  inline RefPtr<css::Value> ExtractorCustom::ExtractBorderShorthand(ExtractorState &state) noexcept
  {
    return style::ExtractBorderShorthand(state, Span<const css::PropertyId> {BorderSections});
  }

  inline void
    ExtractorCustom::ExtractBorderShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                         const css::SerialisationContext &context) noexcept
  {
    style::ExtractBorderShorthandSerialisation(state, builder, context,
                                               Span<const css::PropertyId> {BorderSections});
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractBorderBlockShorthand(ExtractorState &state) noexcept
  {
    return style::ExtractBorderShorthand(state, Span<const css::PropertyId> {BorderBlockSections});
  }

  inline void ExtractorCustom::ExtractBorderBlockShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    style::ExtractBorderShorthandSerialisation(state, builder, context,
                                               Span<const css::PropertyId> {BorderBlockSections});
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractBorderImageShorthand(ExtractorState &state) noexcept
  {
    auto &borderImage = state.Style.GetBorderImage();
    if (borderImage.Source.IsNone())
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
    }
    if (borderImage.Width.OverridesBorderWidths())
    {
      return nullptr;
    }
    return CreateCSSValue(state.Pool, state.Style, borderImage);
  }

  inline void ExtractorCustom::ExtractBorderImageShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    auto &borderImage = state.Style.GetBorderImage();
    if (borderImage.Source.IsNone())
    {
      Serialise(builder, context, state.Style, css::keywords::None {});
      return;
    }
    if (borderImage.Width.OverridesBorderWidths())
    {
      return;
    }
    Serialise(builder, context, state.Style, borderImage);
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractBorderInlineShorthand(ExtractorState &state) noexcept
  {
    return style::ExtractBorderShorthand(state, Span<const css::PropertyId> {BorderInlineSections});
  }

  inline void ExtractorCustom::ExtractBorderInlineShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    style::ExtractBorderShorthandSerialisation(state, builder, context,
                                               Span<const css::PropertyId> {BorderInlineSections});
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractBorderRadiusShorthand(ExtractorState &state) noexcept
  {
    return style::ExtractBorderRadiusShorthand(state);
  }

  inline void ExtractorCustom::ExtractBorderRadiusShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    // FIXME: Do this more efficiently without creating and destroying a css::Value object.
    builder.Append(ExtractBorderRadiusShorthand(state)->CssText(context));
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractColumnsShorthand(ExtractorState &state) noexcept
  {
    if (state.Style.GetColumnCount() == ComputedStyle::InitialColumnCount())
    {
      return CreateCSSValue(state.Pool, state.Style, state.Style.GetColumnWidth());
    }
    if (state.Style.GetColumnWidth() == ComputedStyle::InitialColumnWidth())
    {
      return CreateCSSValue(state.Pool, state.Style, state.Style.GetColumnCount());
    }
    return ExtractStandardSpaceSeparatedShorthand(state, css::ColumnsShorthand());
  }

  inline void
    ExtractorCustom::ExtractColumnsShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                          const css::SerialisationContext &context) noexcept
  {
    if (state.Style.GetColumnCount() == ComputedStyle::InitialColumnCount())
    {
      Serialise(builder, context, state.Style, state.Style.GetColumnWidth());
      return;
    }
    if (state.Style.GetColumnWidth() == ComputedStyle::InitialColumnWidth())
    {
      Serialise(builder, context, state.Style, state.Style.GetColumnCount());
      return;
    }
    ExtractStandardSpaceSeparatedShorthandSerialisation(state, builder, context, css::ColumnsShorthand());
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractContainerShorthand(ExtractorState &state) noexcept
  {
    auto name = [&]() -> Ref<css::Value>
    {
      if (state.Style.GetContainerNames().IsNone())
      {
        return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
      }
      return Ref<css::Value>(ExtractorGenerated::ExtractValue(state, css::PropertyId::ContainerName));
    }();

    if (state.Style.GetContainerType() == ContainerType::Normal)
    {
      return css::ValueList::CreateSlashSeparated(krys::move(name));
    }

    return css::ValueList::CreateSlashSeparated(
      krys::move(name),
      Ref<css::Value>(ExtractorGenerated::ExtractValue(state, css::PropertyId::ContainerType)));
  }

  inline void
    ExtractorCustom::ExtractContainerShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                            const css::SerialisationContext &context) noexcept
  {
    if (state.Style.GetContainerNames().IsNone())
    {
      Serialise(builder, context, state.Style, css::keywords::None {});
    }
    else
    {
      ExtractorGenerated::ExtractValueSerialisation(state, builder, context, css::PropertyId::ContainerName);
    }

    if (state.Style.GetContainerType() == ContainerType::Normal)
    {
      return;
    }

    builder.Append(u8" / ");
    ExtractorGenerated::ExtractValueSerialisation(state, builder, context, css::PropertyId::ContainerType);
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractFlexFlowShorthand(ExtractorState &state) noexcept
  {
    if (state.Style.GetFlexWrap() == ComputedStyle::InitialFlexWrap())
    {
      return CreateCSSValue(state.Pool, state.Style, state.Style.GetFlexDirection());
    }
    if (state.Style.GetFlexDirection() == ComputedStyle::InitialFlexDirection())
    {
      return CreateCSSValue(state.Pool, state.Style, state.Style.GetFlexWrap());
    }
    return ExtractStandardSpaceSeparatedShorthand(state, css::FlexFlowShorthand());
  }

  inline void
    ExtractorCustom::ExtractFlexFlowShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                           const css::SerialisationContext &context) noexcept
  {
    if (state.Style.GetFlexWrap() == ComputedStyle::InitialFlexWrap())
    {
      Serialise(builder, context, state.Style, state.Style.GetFlexDirection());
      return;
    }
    if (state.Style.GetFlexDirection() == ComputedStyle::InitialFlexDirection())
    {
      Serialise(builder, context, state.Style, state.Style.GetFlexWrap());
      return;
    }
    ExtractStandardSpaceSeparatedShorthandSerialisation(state, builder, context, css::FlexFlowShorthand());
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractFontShorthand(ExtractorState &state) noexcept
  {
    auto &description = state.Style.GetFontDescription();
    auto fontWidth = FontWidthKeyword(description.Width());
    auto fontStyle = FontStyleKeyword(description.FontStyleSlope(), description.GetFontStyleAxis());

    auto propertiesResetByShorthandAreExpressible = [&]
    {
      // The font shorthand can express "font-variant-caps: small-caps". Overwrite with "normal" so we can use
      // IsAllNormal to check that all the other settings are normal.
      auto variantSettingsOmittingExpressible = description.VariantSettings();
      if (variantSettingsOmittingExpressible.Caps == FontVariantCaps::Small)
      {
        variantSettingsOmittingExpressible.Caps = FontVariantCaps::Normal;
      }

      // When we add font-language-override, also add code to check for non-expressible values for it here.
      return variantSettingsOmittingExpressible.IsAllNormal() && fontWidth && fontStyle
             && description.GetFontSizeAdjust().IsNone() && description.GetKerning() == Kerning::Auto
             && description.FeatureSettings().isEmpty()
             && description.OpticalSizing() == FontOpticalSizing::Auto
             && description.VariationSettings().isEmpty();
    };

    auto computedFont = css::FontValue::Create();

    if (!propertiesResetByShorthandAreExpressible())
    {
      return computedFont;
    }

    auto size = CreateCSSValue(state.Pool, state.Style, Length<> {description.ComputedSize()});
    computedFont->Size = ShareRefPtr(DynamicDowncast<css::PrimitiveValue>(size.get()));

    auto lineHeight = ExtractorGenerated::ExtractValue(state, css::PropertyId::LineHeight);
    auto *computedLineHeight = DynamicDowncast<css::PrimitiveValue>(lineHeight.get());
    if (computedLineHeight && !css::IsValueId(*computedLineHeight, css::ValueId::Normal))
    {
      computedFont->LineHeight = ShareRefPtr(computedLineHeight);
    }

    if (description.VariantCaps() == FontVariantCaps::Small)
    {
      computedFont->Variant = css::PrimitiveValue::Create(css::ValueId::SmallCaps);
    }
    if (float weight = description.Weight(); weight != 400)
    {
      computedFont->Weight = css::PrimitiveValue::Create(weight);
    }
    if (*fontWidth != css::ValueId::Normal)
    {
      computedFont->Width = css::PrimitiveValue::Create(*fontWidth);
    }
    if (*fontStyle != css::ValueId::Normal)
    {
      computedFont->Style = css::PrimitiveValue::Create(*fontStyle);
    }

    computedFont->Family = CreateCSSValue(state.Pool, state.Style, state.Style.FontFamily());

    return computedFont;
  }

  inline void
    ExtractorCustom::ExtractFontShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                       const css::SerialisationContext &context) noexcept
  {
    // FIXME: Do this more efficiently without creating and destroying a css::Value object.
    builder.Append(ExtractFontShorthand(state)->CssText(context));
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractFontVariantShorthand(ExtractorState &state) noexcept
  {
    css::ValueListBuilder list;
    for (auto longhand : css::FontVariantShorthand())
    {
      auto value = ExtractorGenerated::ExtractValue(state, longhand);
      // We may not have a value if the longhand is disabled.
      if (!value || css::IsValueId(value, css::ValueId::Normal))
      {
        continue;
      }
      list.push_back(Ref<css::Value>(krys::move(value)));
    }
    if (list.empty())
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::Normal {});
    }
    return css::ValueList::CreateSpaceSeparated(krys::move(list));
  }

  inline void ExtractorCustom::ExtractFontVariantShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    // FIXME: Do this more efficiently without creating and destroying a css::Value object.
    builder.Append(ExtractFontVariantShorthand(state)->CssText(context));
  }

  inline constexpr Array<css::PropertyId, 2> MaskPropertiesBeforeSlashSeparator {
    css::PropertyId::MaskImage, css::PropertyId::MaskPosition};
  inline constexpr Array<css::PropertyId, 6> MaskPropertiesAfterSlashSeparator {
    css::PropertyId::MaskSize, css::PropertyId::MaskRepeat,    css::PropertyId::MaskOrigin,
    css::PropertyId::MaskClip, css::PropertyId::MaskComposite, css::PropertyId::MaskMode};

  inline RefPtr<css::Value> ExtractorCustom::ExtractMaskShorthand(ExtractorState &state) noexcept
  {
    return ExtractFillLayerPropertyShorthand<css::PropertyId::Mask>(
      state,
      css::PropertyShorthand(css::PropertyId::Mask,
                             Span<const css::PropertyId, 2> {MaskPropertiesBeforeSlashSeparator}),
      css::PropertyShorthand(css::PropertyId::Mask,
                             Span<const css::PropertyId, 6> {MaskPropertiesAfterSlashSeparator}),
      css::PropertyId::Invalid);
  }

  inline void
    ExtractorCustom::ExtractMaskShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                       const css::SerialisationContext &context) noexcept
  {
    ExtractFillLayerPropertyShorthandSerialisation<css::PropertyId::Mask>(
      state, builder, context,
      css::PropertyShorthand(css::PropertyId::Mask,
                             Span<const css::PropertyId, 2> {MaskPropertiesBeforeSlashSeparator}),
      css::PropertyShorthand(css::PropertyId::Mask,
                             Span<const css::PropertyId, 6> {MaskPropertiesAfterSlashSeparator}),
      css::PropertyId::Invalid);
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractMaskPositionShorthand(ExtractorState &state) noexcept
  {
    auto mapper = [](auto &state, const auto &value, const Maybe<MaskLayer> &,
                     const auto &) -> Ref<css::Value>
    {
      return CreateCSSValue(state.Pool, state.Style, value);
    };
    return ExtractCoordinatedValueListValue<css::PropertyId::MaskPosition>(state, state.Style.GetMaskLayers(),
                                                                           mapper);
  }

  inline void ExtractorCustom::ExtractMaskPositionShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    auto mapper = [](auto &state, auto &builder, const auto &context, const auto &value,
                     const Maybe<MaskLayer> &, const auto &)
    {
      Serialise(builder, context, state.Style, value);
    };
    ExtractCoordinatedValueListSerialisation<css::PropertyId::MaskPosition>(
      state, builder, context, state.Style.GetMaskLayers(), mapper);
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractOffsetShorthand(ExtractorState &state) noexcept
  {
    // [ <'offset-position'>? [ <'offset-path'> [ <'offset-distance'> || <'offset-rotate'> ]? ]? ]! [ /
    // <'offset-anchor'> ]?

    // The first four elements are serialised in a space separated css::ValueList.
    // This is then combined with offset-anchor in a slash separated css::ValueList.

    css::ValueListBuilder innerList;

    krys::SwitchOn(
      state.Style.GetOffsetPosition(), [&](const css::keywords::Auto &) {},
      [&](const css::keywords::Normal &) {}, [&](const Position &position)
      { innerList.push_back(CreateCSSValue(state.Pool, state.Style, position)); });

    bool nonInitialDistance = state.Style.GetOffsetDistance() != ComputedStyle::InitialOffsetDistance();
    bool nonInitialRotate = state.Style.GetOffsetRotate() != ComputedStyle::InitialOffsetRotate();

    if (state.Style.HasOffsetPath() || nonInitialDistance || nonInitialRotate)
    {
      innerList.push_back(CreateCSSValue(state.Pool, state.Style, state.Style.GetOffsetPath()));
    }

    if (nonInitialDistance)
    {
      innerList.push_back(CreateCSSValue(state.Pool, state.Style, state.Style.GetOffsetDistance()));
    }
    if (nonInitialRotate)
    {
      innerList.push_back(CreateCSSValue(state.Pool, state.Style, state.Style.GetOffsetRotate()));
    }

    auto inner = innerList.empty()
                   ? CreateCSSValue(state.Pool, state.Style, css::keywords::Auto {})
                   : Ref<css::Value> {css::ValueList::CreateSpaceSeparated(krys::move(innerList))};

    return krys::SwitchOn(
      state.Style.GetOffsetAnchor(), [&](const css::keywords::Auto &) -> Ref<css::Value> { return inner; },
      [&](const Position &position) -> Ref<css::Value>
      {
        return css::ValueList::CreateSlashSeparated(krys::move(inner),
                                                    CreateCSSValue(state.Pool, state.Style, position));
      });
  }

  inline void
    ExtractorCustom::ExtractOffsetShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                         const css::SerialisationContext &context) noexcept
  {
    // FIXME: Do this more efficiently without creating and destroying a css::Value object.
    builder.Append(ExtractOffsetShorthand(state)->CssText(context));
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractPositionTryShorthand(ExtractorState &state) noexcept
  {
    if (state.Style.GetPositionTryOrder() == ComputedStyle::InitialPositionTryOrder())
    {
      return ExtractorGenerated::ExtractValue(state, css::PropertyId::PositionTryFallbacks);
    }
    return ExtractStandardSpaceSeparatedShorthand(state, css::PositionTryShorthand());
  }

  inline void ExtractorCustom::ExtractPositionTryShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    if (state.Style.GetPositionTryOrder() == ComputedStyle::InitialPositionTryOrder())
    {
      ExtractorGenerated::ExtractValueSerialisation(state, builder, context,
                                                    css::PropertyId::PositionTryFallbacks);
      return;
    }
    ExtractStandardSpaceSeparatedShorthandSerialisation(state, builder, context, css::PositionTryShorthand());
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractScrollTimelineShorthand(ExtractorState &state) noexcept
  {
    auto &timelines = state.Style.GetScrollTimelines();
    if (timelines.isEmpty())
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
    }

    css::ValueListBuilder list;
    for (auto &timeline : timelines)
    {
      auto &name = timeline->GetName();
      auto axis = timeline->Axis();

      krys_debug_assert(!name.View().empty());
      auto nameCSSValue = CreateCSSValue(state.Pool, state.Style, css::CustomIdentifier {name});

      if (axis == ScrollAxis::Block)
      {
        list.push_back(krys::move(nameCSSValue));
      }
      else
      {
        list.push_back(
          css::ValuePair::CreateNonCoalescing(nameCSSValue, CreateCSSValue(state.Pool, state.Style, axis)));
      }
    }
    return css::ValueList::CreateCommaSeparated(krys::move(list));
  }

  inline void ExtractorCustom::ExtractScrollTimelineShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    auto &timelines = state.Style.GetScrollTimelines();
    if (timelines.isEmpty())
    {
      Serialise(builder, context, state.Style, css::keywords::None {});
      return;
    }

    AppendInterleaved(
      builder, timelines,
      [&](auto &builder, auto &timeline)
      {
        krys_debug_assert(!timeline->GetName().View().empty());

        Serialise(builder, context, state.Style, css::CustomIdentifier {timeline->GetName()});
        if (auto axis = timeline->Axis(); axis != ScrollAxis::Block)
        {
          builder.Append(u8' ');
          Serialise(builder, context, state.Style, axis);
        }
      },
      u8", ");
  }

  inline RefPtr<css::Value>
    ExtractorCustom::ExtractTextDecorationSkipShorthand(ExtractorState &state) noexcept
  {
    switch (state.Style.GetTextDecorationSkipInk())
    {
      case TextDecorationSkipInk::None:
        return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
      case TextDecorationSkipInk::Auto:
        return CreateCSSValue(state.Pool, state.Style, css::keywords::Auto {});
      case TextDecorationSkipInk::All: return nullptr;
    }
    krys_unreachable();
  }

  inline void ExtractorCustom::ExtractTextDecorationSkipShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    switch (state.Style.GetTextDecorationSkipInk())
    {
      case TextDecorationSkipInk::None:
        Serialise(builder, context, state.Style, css::keywords::None {});
        return;
      case TextDecorationSkipInk::Auto:
        Serialise(builder, context, state.Style, css::keywords::Auto {});
        return;
      case TextDecorationSkipInk::All: return;
    }
    krys_unreachable();
  }

  KRYS_NODISCARD inline Ref<css::Value> ConvertSingleTransition(ExtractorState &state,
                                                                const Transition &transition) noexcept
  {
    static const EasingFunction initialTimingFunction(Transition::InitialTimingFunction());

    // If we have a transition-delay but no transition-duration set, we must serialise
    // the transition-duration because they're both <time> values and transition-delay
    // comes first.
    auto showsDelay = transition.GetDelay() != Transition::InitialDelay();
    auto showsDuration = showsDelay || transition.GetDuration() != Transition::InitialDuration();

    css::ValueListBuilder list;
    if (transition.GetProperty() != Transition::InitialProperty())
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, transition.GetProperty()));
    }
    if (showsDuration)
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, transition.GetDuration()));
    }
    if (transition.GetTimingFunction() != initialTimingFunction)
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, transition.GetTimingFunction()));
    }
    if (showsDelay)
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, transition.GetDelay()));
    }
    if (transition.GetBehavior() != Transition::InitialBehavior())
    {
      list.push_back(CreateCSSValue(state.Pool, state.Style, transition.GetBehavior()));
    }
    if (list.empty())
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::All {});
    }
    return css::ValueList::CreateSpaceSeparated(krys::move(list));
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractTransitionShorthand(ExtractorState &state) noexcept
  {
    auto &transitions = state.Style.GetTransitions();
    if (transitions.IsInitial())
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::All {});
    }

    css::ValueListBuilder list;
    for (auto &transition : transitions.ComputedValues())
    {
      list.push_back(ConvertSingleTransition(state, transition));
    }
    krys_debug_assert(!list.empty());
    return css::ValueList::CreateCommaSeparated(krys::move(list));
  }

  inline void ExtractorCustom::ExtractTransitionShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    auto &transitions = state.Style.GetTransitions();
    if (transitions.IsInitial())
    {
      Serialise(builder, context, state.Style, css::keywords::All {});
      return;
    }

    AppendInterleaved(
      builder, transitions.ComputedValues(),
      [&](auto &builder, auto &transition)
      {
        // FIXME: Do this more efficiently without creating and destroying a css::Value object.
        builder.Append(ConvertSingleTransition(state, transition)->CssText(context));
      },
      u8", ");
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractViewTimelineShorthand(ExtractorState &state) noexcept
  {
    auto &timelines = state.Style.GetViewTimelines();
    if (timelines.isEmpty())
    {
      return CreateCSSValue(state.Pool, state.Style, css::keywords::None {});
    }

    css::ValueListBuilder list;
    for (auto &timeline : timelines)
    {
      auto &name = timeline->GetName();
      auto axis = timeline->Axis();
      auto &insets = timeline->Insets();

      auto hasDefaultAxis = axis == ScrollAxis::Block;
      auto hasDefaultInsets = insets.Start().IsAuto() && insets.End().IsAuto();

      krys_debug_assert(!name.View().empty());
      auto nameCSSValue = CreateCSSValue(state.Pool, state.Style, css::CustomIdentifier {name});

      if (hasDefaultAxis && hasDefaultInsets)
      {
        list.push_back(krys::move(nameCSSValue));
      }
      else if (hasDefaultAxis)
      {
        list.push_back(
          css::ValuePair::CreateNonCoalescing(nameCSSValue, CreateCSSValue(state.Pool, state.Style, insets)));
      }
      else if (hasDefaultInsets)
      {
        list.push_back(
          css::ValuePair::CreateNonCoalescing(nameCSSValue, CreateCSSValue(state.Pool, state.Style, axis)));
      }
      else
      {
        list.push_back(css::ValueList::CreateSpaceSeparated(krys::move(nameCSSValue),
                                                            CreateCSSValue(state.Pool, state.Style, axis),
                                                            CreateCSSValue(state.Pool, state.Style, insets)));
      }
    }
    return css::ValueList::CreateCommaSeparated(krys::move(list));
  }

  inline void ExtractorCustom::ExtractViewTimelineShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    // FIXME: Do this more efficiently without creating and destroying a css::Value object.
    builder.Append(ExtractViewTimelineShorthand(state)->CssText(context));
  }

  inline RefPtr<css::Value>
    ExtractorCustom::ExtractWebkitMaskPositionShorthand(ExtractorState &state) noexcept
  {
    return ExtractMaskPositionShorthand(state);
  }

  inline void ExtractorCustom::ExtractWebkitMaskPositionShorthandSerialisation(
    ExtractorState &state, StringBuilder &builder, const css::SerialisationContext &context) noexcept
  {
    ExtractMaskPositionShorthandSerialisation(state, builder, context);
  }

  inline void
    ExtractorCustom::ExtractMarkerShorthandSerialisation(ExtractorState &state, StringBuilder &builder,
                                                         const css::SerialisationContext &context) noexcept
  {
    auto markerStart = state.Style.MarkerStart();
    auto markerMid = state.Style.MarkerMid();
    auto markerEnd = state.Style.MarkerEnd();
    if (markerStart == markerMid && markerMid == markerEnd)
    {
      Serialise(builder, context, state.Style, markerStart);
    }
  }

  inline RefPtr<css::Value> ExtractorCustom::ExtractMarkerShorthand(ExtractorState &state) noexcept
  {
    auto markerStart = state.Style.MarkerStart();
    auto markerMid = state.Style.MarkerMid();
    auto markerEnd = state.Style.MarkerEnd();
    if (markerStart == markerMid && markerMid == markerEnd)
    {
      return CreateCSSValue(state.Pool, state.Style, markerStart);
    }
    return nullptr;
  }
}
