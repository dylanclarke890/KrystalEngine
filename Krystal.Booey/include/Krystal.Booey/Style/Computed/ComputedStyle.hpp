#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyleProperties.hpp"

namespace krys::boo::style
{
  class ComputedStyle final : public ComputedStyleProperties
  {
  public:
    void InheritFrom(const ComputedStyle &) noexcept;
    void InheritIgnoringCustomPropertiesFrom(const ComputedStyle &) noexcept;
    void InheritUnicodeBidiFrom(const ComputedStyle &) noexcept;
    inline void InheritColumnPropertiesFrom(const ComputedStyle &) noexcept;
    void FastPathInheritFrom(const ComputedStyle &) noexcept;
    void CopyNonInheritedFrom(const ComputedStyle &) noexcept;
    void CopyContentFrom(const ComputedStyle &) noexcept;
    void CopyPseudoElementBitsFrom(const ComputedStyle &) noexcept;

    // MARK: - Comparisons

    bool operator==(const ComputedStyle &) const noexcept;

    KRYS_NODISCARD bool InheritedEqual(const ComputedStyle &) const noexcept;
    KRYS_NODISCARD bool NonInheritedEqual(const ComputedStyle &) const noexcept;
    KRYS_NODISCARD bool FastPathInheritedEqual(const ComputedStyle &) const noexcept;
    KRYS_NODISCARD bool NonFastPathInheritedEqual(const ComputedStyle &) const noexcept;
    KRYS_NODISCARD bool DescendantAffectingNonInheritedPropertiesEqual(const ComputedStyle &) const noexcept;
    KRYS_NODISCARD bool BorderAndBackgroundEqual(const ComputedStyle &) const noexcept;
    KRYS_NODISCARD inline bool ContainerTypeAndNamesEqual(const ComputedStyle &) const noexcept;
    KRYS_NODISCARD inline bool ColumnSpanEqual(const ComputedStyle &) const noexcept;
    KRYS_NODISCARD inline bool ScrollPaddingEqual(const ComputedStyle &) const noexcept;
    KRYS_NODISCARD inline bool FontCascadeEqual(const ComputedStyle &) const noexcept;
    KRYS_NODISCARD bool ScrollSnapDataEquivalent(const ComputedStyle &) const noexcept;

    // MARK: - Style adjustment utilities

    void SetPageScaleTransform(float) noexcept;
    inline void AddToTextDecorationLineInEffect(TextDecorationLine) noexcept;
    inline void ContainIntrinsicWidthAddAuto() noexcept;
    inline void ContainIntrinsicHeightAddAuto() noexcept;
    inline void SetGridAutoFlowDirection(GridAutoFlow::Direction) noexcept;

    void AdjustAnimations() noexcept;
    void AdjustTransitions() noexcept;
    void AdjustBackgroundLayers() noexcept;
    void AdjustMaskLayers() noexcept;
    void AdjustScrollTimelines() noexcept;
    void AdjustViewTimelines() noexcept;

    inline void ResetBorder() noexcept;
    inline void ResetBorderExceptRadius() noexcept;
    inline void ResetBorderTop() noexcept;
    inline void ResetBorderRight() noexcept;
    inline void ResetBorderBottom() noexcept;
    inline void ResetBorderLeft() noexcept;
    inline void ResetBorderRadius() noexcept;
    inline void ResetMargin() noexcept;
    inline void ResetPadding() noexcept;

    // MARK: - Derived Values

    KRYS_NODISCARD float ComputedLineHeight() const noexcept;
    KRYS_NODISCARD float ComputeLineHeight(const LineHeight &) const noexcept;

    // MARK: - Non-property initial values.

    KRYS_NODISCARD static inline PageSize InitialPageSize() noexcept;
    KRYS_NODISCARD constexpr static ZIndex InitialUsedZIndex() noexcept;

  private:
    friend class boo::RenderStyleBase;

    ComputedStyle(ComputedStyle &&) noexcept;
    ComputedStyle &operator=(ComputedStyle &&) noexcept;
    ComputedStyle(CreateDefaultStyleTag) noexcept;
    ComputedStyle(const ComputedStyle &, CloneTag) noexcept;
    ComputedStyle(ComputedStyle &, ComputedStyle &&) noexcept;
  };
}
