#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

#define RENDER_STYLE_PROPERTIES_SETTERS_INLINES_INCLUDE_TRAP 1
#include "Krystal.Booey/Rendering/Style/RenderStyleProperties+SettersInlines.hpp"
#undef RENDER_STYLE_PROPERTIES_SETTERS_INLINES_INCLUDE_TRAP

namespace krys::boo
{
  // MARK: - Initialization

  inline void RenderStyle::InheritFrom(const RenderStyle &other) noexcept
  {
    return _computedStyle.InheritFrom(other._computedStyle);
  }

  inline void RenderStyle::InheritIgnoringCustomPropertiesFrom(const RenderStyle &other) noexcept
  {
    return _computedStyle.InheritIgnoringCustomPropertiesFrom(other._computedStyle);
  }

  inline void RenderStyle::InheritUnicodeBidiFrom(const RenderStyle &other) noexcept
  {
    return _computedStyle.InheritUnicodeBidiFrom(other._computedStyle);
  }

  inline void RenderStyle::InheritColumnPropertiesFrom(const RenderStyle &other) noexcept
  {
    return _computedStyle.InheritColumnPropertiesFrom(other._computedStyle);
  }

  inline void RenderStyle::FastPathInheritFrom(const RenderStyle &other) noexcept
  {
    return _computedStyle.FastPathInheritFrom(other._computedStyle);
  }

  inline void RenderStyle::CopyNonInheritedFrom(const RenderStyle &other) noexcept
  {
    return _computedStyle.CopyNonInheritedFrom(other._computedStyle);
  }

  inline void RenderStyle::CopyContentFrom(const RenderStyle &other) noexcept
  {
    return _computedStyle.CopyContentFrom(other._computedStyle);
  }

  inline void RenderStyle::CopyPseudoElementBitsFrom(const RenderStyle &other) noexcept
  {
    return _computedStyle.CopyPseudoElementBitsFrom(other._computedStyle);
  }

  // MARK: - Style adjustment utilities

  inline void RenderStyle::SetPageScaleTransform(float scale) noexcept
  {
    _computedStyle.SetPageScaleTransform(scale);
  }

  inline void RenderStyle::AdjustAnimations() noexcept
  {
    _computedStyle.AdjustAnimations();
  }

  inline void RenderStyle::AdjustTransitions() noexcept
  {
    _computedStyle.AdjustTransitions();
  }

  inline void RenderStyle::AdjustBackgroundLayers() noexcept
  {
    _computedStyle.AdjustBackgroundLayers();
  }

  inline void RenderStyle::AdjustMaskLayers() noexcept
  {
    _computedStyle.AdjustMaskLayers();
  }

  inline void RenderStyle::AdjustScrollTimelines() noexcept
  {
    _computedStyle.AdjustScrollTimelines();
  }

  inline void RenderStyle::AdjustViewTimelines() noexcept
  {
    _computedStyle.AdjustViewTimelines();
  }

  inline void RenderStyle::AddToTextDecorationLineInEffect(style::TextDecorationLine value) noexcept
  {
    _computedStyle.AddToTextDecorationLineInEffect(value);
  }

  inline void RenderStyle::ContainIntrinsicWidthAddAuto() noexcept
  {
    _computedStyle.ContainIntrinsicWidthAddAuto();
  }

  inline void RenderStyle::ContainIntrinsicHeightAddAuto() noexcept
  {
    _computedStyle.ContainIntrinsicHeightAddAuto();
  }

  inline void RenderStyle::SetGridAutoFlowDirection(style::GridAutoFlow::Direction direction) noexcept
  {
    _computedStyle.SetGridAutoFlowDirection(direction);
  }

  inline void RenderStyle::ResetBorderBottom() noexcept
  {
    _computedStyle.ResetBorderBottom();
  }

  inline void RenderStyle::ResetBorderLeft() noexcept
  {
    _computedStyle.ResetBorderLeft();
  }

  inline void RenderStyle::ResetBorderRight() noexcept
  {
    _computedStyle.ResetBorderRight();
  }

  inline void RenderStyle::ResetBorderTop() noexcept
  {
    _computedStyle.ResetBorderTop();
  }

  inline void RenderStyle::ResetMargin() noexcept
  {
    _computedStyle.ResetMargin();
  }

  inline void RenderStyle::ResetPadding() noexcept
  {
    _computedStyle.ResetPadding();
  }

  inline void RenderStyle::ResetBorder() noexcept
  {
    _computedStyle.ResetBorder();
  }

  inline void RenderStyle::ResetBorderExceptRadius() noexcept
  {
    _computedStyle.ResetBorderExceptRadius();
  }

  inline void RenderStyle::ResetBorderRadius() noexcept
  {
    _computedStyle.ResetBorderRadius();
  }
}
