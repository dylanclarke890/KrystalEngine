#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+SettersInlines.hpp"

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/PathTraversalState.hpp"
#include "Krystal.Booey/Graphics/Transforms/ScaleTransformOperation.hpp"
#include "Krystal.Booey/Rendering/MotionPath.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Rendering/Style/SVGRenderStyle.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyleBase+ConstructionInlines.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Text/TextTransform.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/ScaleTransformFunction.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <algorithm>

namespace krys::boo::style
{
  void ComputedStyle::InheritFrom(const ComputedStyle &inheritParent) noexcept
  {
    _rareInheritedData = inheritParent._rareInheritedData;
    _inheritedData = inheritParent._inheritedData;
    _inheritedFlags = inheritParent._inheritedFlags;

    if (_svgStyle != inheritParent._svgStyle)
    {
      _svgStyle.Access().InheritFrom(inheritParent._svgStyle.Get());
    }
  }

  void ComputedStyle::InheritIgnoringCustomPropertiesFrom(const ComputedStyle &inheritParent) noexcept
  {
    auto oldCustomProperties = _rareInheritedData->CustomProperties;
    InheritFrom(inheritParent);
    if (oldCustomProperties != _rareInheritedData->CustomProperties)
    {
      _rareInheritedData.Access().CustomProperties = oldCustomProperties;
    }
  }

  void ComputedStyle::InheritUnicodeBidiFrom(const ComputedStyle &inheritParent) noexcept
  {
    _nonInheritedFlags.UnicodeBidi = inheritParent._nonInheritedFlags.UnicodeBidi;
  }

  void ComputedStyle::FastPathInheritFrom(const ComputedStyle &inheritParent) noexcept
  {
    krys_debug_assert(!DisallowsFastPathInheritance());

    // FIXME: Use this mechanism for other properties too, like variables.
    _inheritedFlags.Visibility = inheritParent._inheritedFlags.Visibility;
    _inheritedFlags.HasExplicitlySetColor = inheritParent._inheritedFlags.HasExplicitlySetColor;

    if (_inheritedData.Ptr() != inheritParent._inheritedData.Ptr())
    {
      if (_inheritedData->NonFastPathInheritedEqual(*inheritParent._inheritedData))
      {
        _inheritedData = inheritParent._inheritedData;
        return;
      }
      _inheritedData.Access().FastPathInheritFrom(*inheritParent._inheritedData);
    }
  }

  void ComputedStyle::CopyNonInheritedFrom(const ComputedStyle &other) noexcept
  {
    _nonInheritedData = other._nonInheritedData;
    _nonInheritedFlags.CopyNonInheritedFrom(other._nonInheritedFlags);

    if (_svgStyle != other._svgStyle)
    {
      _svgStyle.Access().CopyNonInheritedFrom(other._svgStyle.Get());
    }

    krys_debug_assert(GetZoom() == InitialZoom());
  }

  void ComputedStyle::CopyContentFrom(const ComputedStyle &other) noexcept
  {
    if (!other._nonInheritedData->MiscData->Content.IsData())
    {
      return;
    }
    _nonInheritedData.Access().MiscData.Access().Content = other._nonInheritedData->MiscData->Content;
  }

  void ComputedStyle::CopyPseudoElementBitsFrom(const ComputedStyle &other) noexcept
  {
    _nonInheritedFlags.PseudoBits = other._nonInheritedFlags.PseudoBits;
  }

  bool ComputedStyle::operator==(const ComputedStyle &other) const noexcept
  {
    // compare everything except the pseudoStyle pointer
    return _inheritedFlags == other._inheritedFlags && _nonInheritedFlags == other._nonInheritedFlags
           && _nonInheritedData == other._nonInheritedData && _rareInheritedData == other._rareInheritedData
           && _inheritedData == other._inheritedData && _svgStyle == other._svgStyle;
  }

  bool ComputedStyle::InheritedEqual(const ComputedStyle &other) const noexcept
  {
    return _inheritedFlags == other._inheritedFlags && _inheritedData == other._inheritedData
           && (_svgStyle.Ptr() == other._svgStyle.Ptr() || _svgStyle->InheritedEqual(other._svgStyle))
           && _rareInheritedData == other._rareInheritedData;
  }

  bool ComputedStyle::NonInheritedEqual(const ComputedStyle &other) const noexcept
  {
    return _nonInheritedFlags == other._nonInheritedFlags && _nonInheritedData == other._nonInheritedData
           && (_svgStyle.Ptr() == other._svgStyle.Ptr() || _svgStyle->NonInheritedEqual(other._svgStyle));
  }

  bool ComputedStyle::FastPathInheritedEqual(const ComputedStyle &other) const noexcept
  {
    if (_inheritedFlags.Visibility != other._inheritedFlags.Visibility)
    {
      return false;
    }
    if (_inheritedFlags.HasExplicitlySetColor != other._inheritedFlags.HasExplicitlySetColor)
    {
      return false;
    }
    if (_inheritedData.Ptr() == other._inheritedData.Ptr())
    {
      return true;
    }
    return _inheritedData->FastPathInheritedEqual(*other._inheritedData);
  }

  bool ComputedStyle::NonFastPathInheritedEqual(const ComputedStyle &other) const noexcept
  {
    auto withoutFastPathFlags = [](auto flags)
    {
      flags.Visibility = {};
      flags.HasExplicitlySetColor = {};
      return flags;
    };
    if (withoutFastPathFlags(_inheritedFlags) != withoutFastPathFlags(other._inheritedFlags))
    {
      return false;
    }
    if (_inheritedData.Ptr() != other._inheritedData.Ptr()
        && !_inheritedData->NonFastPathInheritedEqual(*other._inheritedData))
    {
      return false;
    }
    if (_rareInheritedData != other._rareInheritedData)
    {
      return false;
    }
    if (_svgStyle.Ptr() != other._svgStyle.Ptr() && !_svgStyle->InheritedEqual(other._svgStyle))
    {
      return false;
    }
    return true;
  }

  bool
    ComputedStyle::DescendantAffectingNonInheritedPropertiesEqual(const ComputedStyle &other) const noexcept
  {
    if (_nonInheritedData.Ptr() == other._nonInheritedData.Ptr()
        || _nonInheritedData->MiscData.Ptr() == other._nonInheritedData->MiscData.Ptr())
    {
      return true;
    }

    if (_nonInheritedData->MiscData->AlignItems != other._nonInheritedData->MiscData->AlignItems)
    {
      return false;
    }

    if (_nonInheritedData->MiscData->JustifyItems != other._nonInheritedData->MiscData->JustifyItems)
    {
      return false;
    }

    if (_nonInheritedData->MiscData->UsedAppearance != other._nonInheritedData->MiscData->UsedAppearance)
    {
      return false;
    }

    return true;
  }

  bool ComputedStyle::BorderAndBackgroundEqual(const ComputedStyle &other) const noexcept
  {
    return Border() == other.Border() && GetBackgroundLayers() == other.GetBackgroundLayers()
           && BackgroundColor() == other.BackgroundColor();
  }

  float ComputedStyle::ComputedLineHeight() const noexcept
  {
    return ComputeLineHeight(GetLineHeight());
  }

  float ComputedStyle::ComputeLineHeight(const LineHeight &lineHeight) const noexcept
  {
    return krys::SwitchOn(
      lineHeight, [&](const css::keywords::Normal &) -> float
      { return MetricsOfPrimaryFont().LineSpacing(); }, [&](const LineHeight::Fixed &fixed) -> float
      { return Evaluate<LayoutUnit>(fixed, UsedZoomForLength()).ToFloat(); },
      [&](const LineHeight::Percentage &percentage) -> float
      { return Evaluate<LayoutUnit>(percentage, LayoutUnit {ComputedFontSize()}).ToFloat(); },
      [&](const LineHeight::Calc &calc) -> float
      { return Evaluate<LayoutUnit>(calc, LayoutUnit {ComputedFontSize()}, UsedZoomForLength()).ToFloat(); });
  }

  void ComputedStyle::SetPageScaleTransform(float scale) noexcept
  {
    if (scale == 1)
    {
      return;
    }

    SetTransform(style::Transform {style::TransformFunction {
      style::ScaleTransformFunction::Create(scale, scale, style::TransformFunctionType::Scale)}});
    SetTransformOriginX(0_css_px);
    SetTransformOriginY(0_css_px);
  }

  bool ComputedStyle::ScrollSnapDataEquivalent(const ComputedStyle &other) const noexcept
  {
    if (_nonInheritedData.Ptr() == other._nonInheritedData.Ptr()
        || _nonInheritedData->RareData.Ptr() == other._nonInheritedData->RareData.Ptr())
    {
      return true;
    }

    return _nonInheritedData->RareData->ScrollMargin == other._nonInheritedData->RareData->ScrollMargin
           && _nonInheritedData->RareData->ScrollSnapAlign
                == other._nonInheritedData->RareData->ScrollSnapAlign
           && _nonInheritedData->RareData->ScrollSnapStop == other._nonInheritedData->RareData->ScrollSnapStop
           && _nonInheritedData->RareData->ScrollSnapAlign
                == other._nonInheritedData->RareData->ScrollSnapAlign;
  }

  // MARK: - Style adjustment utilities

  void ComputedStyle::AdjustAnimations() noexcept
  {
    if (GetAnimations().IsInitial())
    {
      return;
    }

    EnsureAnimations().PrepareForUse();
  }

  void ComputedStyle::AdjustTransitions() noexcept
  {
    if (GetTransitions().IsInitial())
    {
      return;
    }

    EnsureTransitions().PrepareForUse();
  }

  void ComputedStyle::AdjustBackgroundLayers() noexcept
  {
    if (GetBackgroundLayers().IsInitial())
    {
      return;
    }

    EnsureBackgroundLayers().PrepareForUse();
  }

  void ComputedStyle::AdjustMaskLayers() noexcept
  {
    if (GetMaskLayers().IsInitial())
    {
      return;
    }

    EnsureMaskLayers().PrepareForUse();
  }

  void ComputedStyle::AdjustScrollTimelines() noexcept
  {
    auto &names = ScrollTimelineNames();
    if (names.IsNone() && GetScrollTimelines().isEmpty())
    {
      return;
    }

    auto &axes = ScrollTimelineAxes();
    auto numberOfAxes = axes.size();
    krys_debug_assert(numberOfAxes > 0);

    auto timelines = css::CommaSeparatedFixedList<Ref<ScrollTimeline>>::createWithSizeFromGenerator(
      names.size(),
      [&](auto i) { return ScrollTimeline::Create(names[i].Value.Value, axes[i % numberOfAxes]); });
    _nonInheritedData.Access().RareData.Access().ScrollTimelines =
      ScrollTimelines {krys::move(timelines.Value)};
  }

  void ComputedStyle::AdjustViewTimelines() noexcept
  {
    auto &names = ViewTimelineNames();
    if (names.IsNone() && GetViewTimelines().isEmpty())
    {
      return;
    }

    auto &axes = ViewTimelineAxes();
    auto numberOfAxes = axes.size();
    krys_debug_assert(numberOfAxes > 0);

    auto &insets = GetViewTimelineInsets();
    auto numberOfInsets = insets.size();
    krys_debug_assert(numberOfInsets > 0);

    auto timelines = css::CommaSeparatedFixedList<Ref<ViewTimeline>>::createWithSizeFromGenerator(
      names.size(),
      [&](auto i)
      {
        return ViewTimeline::Create(names[i].Value.Value, axes[i % numberOfAxes], insets[i % numberOfInsets]);
      });
    _nonInheritedData.Access().RareData.Access().ViewTimelines = ViewTimelines {krys::move(timelines.Value)};
  }
}
