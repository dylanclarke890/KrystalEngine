#pragma once

#include "Krystal.Booey/HTML/HTMLDivElement.hpp"

namespace krys::boo::html
{
  class HTMLInputElement;

  /// @brief A range's thumb (the `::-webkit-slider-thumb` part), which the user drags along the track.
  class SliderThumbElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SliderThumbElement);

  private:
    bool _inDragMode {false};

  public:
    explicit SliderThumbElement(dom::Document &) noexcept;

    KRYS_NODISCARD static Ref<SliderThumbElement> Create(dom::Document &) noexcept;

    void SetPositionFromValue() noexcept;

    void DragFrom(const LayoutPoint &) noexcept;
    KRYS_NODISCARD HTMLInputElement *HostInput() const noexcept;
    void SetPositionFromPoint(const LayoutPoint &) noexcept;

    void HostDisabledStateChanged() noexcept;

    KRYS_NODISCARD bool IsSliderThumbElement() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept final;
    KRYS_NODISCARD bool MatchesReadWritePseudoClass() const noexcept final;
    void DefaultEventHandler(dom::Event &) noexcept final;

  private:
    // FIXME: Releasing the capture when the renderer is detached (willDetachRenderers), once there's a hook.
    KRYS_NODISCARD Maybe<style::UnadjustedStyle> ResolveCustomStyle(const style::ResolutionContext &,
                                                                    const RenderStyle *) noexcept final;

    void StartDragging() noexcept;
    void StopDragging() noexcept;
  };

  /// @brief The container of a range's track (the `::-webkit-slider-container` part), which lays the track
  /// and thumb out.
  class SliderContainerElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SliderContainerElement);

  public:
    explicit SliderContainerElement(dom::Document &) noexcept;

    KRYS_NODISCARD static Ref<SliderContainerElement> Create(dom::Document &) noexcept;

    KRYS_NODISCARD bool IsSliderContainerElement() const noexcept final
    {
      return true;
    }

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::SliderThumbElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &element) noexcept
  {
    return element.IsSliderThumbElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsHTMLElement() && IsType(Downcast<krys::boo::html::HTMLElement>(node));
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::SliderContainerElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &element) noexcept
  {
    return element.IsSliderContainerElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsHTMLElement() && IsType(Downcast<krys::boo::html::HTMLElement>(node));
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
