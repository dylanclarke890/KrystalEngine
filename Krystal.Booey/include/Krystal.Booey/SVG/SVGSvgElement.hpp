#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/SVG/SVGFitToViewBox.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGPoint.hpp"
#include "Krystal.Booey/SVG/SVGZoomAndPan.hpp"

namespace krys::boo::css
{
  struct DOMMatrix2DInit;
}

namespace krys::boo
{
  class LocalFrame;
}

namespace krys::boo::svg
{
  class SVGAngle;
  class SVGLength;
  class SVGMatrix;
  class SVGNumber;
  class SVGRect;
  class SVGTransform;
  class SVGViewElement;
  class SVGViewSpec;

  /// @brief The `<svg>` element: a viewport, the root of an SVG document fragment when it's outermost.
  /// @see https://svgwg.org/svg2-draft/struct.html#InterfaceSVGSVGElement
  class SVGSvgElement final : public SVGGraphicsElement, public SVGFitToViewBox, public SVGZoomAndPan
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGSvgElement);

  private:
    Ref<SVGPoint> _currentTranslate {SVGPoint::Create()};

    Ref<SVGAnimatedLength> _x;
    Ref<SVGAnimatedLength> _y;
    Ref<SVGAnimatedLength> _width;
    Ref<SVGAnimatedLength> _height;

    bool _useCurrentView {false};
    RefPtr<SVGViewSpec> _viewSpec;
    dom::DOMString _currentViewFragmentIdentifier;
    WeakPtr<SVGViewElement> _currentViewElement;

  public:
    explicit SVGSvgElement(dom::Document &document) noexcept;
    ~SVGSvgElement() noexcept override;

#pragma region DOM

    /// @see https://svgwg.org/svg2-draft/struct.html#__svg__SVGSVGElement__currentScale
    KRYS_NODISCARD float CurrentScale() const noexcept;
    void SetCurrentScale(float scale) noexcept;

    /// @see https://svgwg.org/svg2-draft/struct.html#__svg__SVGSVGElement__currentTranslate
    KRYS_NODISCARD SVGPoint &CurrentTranslate() noexcept
    {
      return *_currentTranslate;
    }

    KRYS_NODISCARD FloatPoint CurrentTranslateValue() const noexcept
    {
      return _currentTranslate->Value();
    }

    /// @see https://svgwg.org/svg2-draft/struct.html#__svg__SVGSVGElement__getIntersectionList
    KRYS_NODISCARD Ref<dom::NodeList> GetIntersectionList(SVGRect &, SVGElement *referenceElement) noexcept;
    /// @see https://svgwg.org/svg2-draft/struct.html#__svg__SVGSVGElement__getEnclosureList
    KRYS_NODISCARD Ref<dom::NodeList> GetEnclosureList(SVGRect &, SVGElement *referenceElement) noexcept;
    /// @see https://svgwg.org/svg2-draft/struct.html#__svg__SVGSVGElement__checkIntersection
    KRYS_NODISCARD static bool CheckIntersection(Ref<SVGElement> &&, SVGRect &) noexcept;
    /// @see https://svgwg.org/svg2-draft/struct.html#__svg__SVGSVGElement__checkEnclosure
    KRYS_NODISCARD static bool CheckEnclosure(Ref<SVGElement> &&, SVGRect &) noexcept;

    /// @see https://svgwg.org/svg2-draft/struct.html#__svg__SVGSVGElement__deselectAll
    void DeselectAll() noexcept;

    KRYS_NODISCARD static Ref<SVGNumber> CreateSVGNumber() noexcept;
    KRYS_NODISCARD static Ref<SVGLength> CreateSVGLength() noexcept;
    KRYS_NODISCARD static Ref<SVGAngle> CreateSVGAngle() noexcept;
    KRYS_NODISCARD static Ref<SVGPoint> CreateSVGPoint() noexcept;
    KRYS_NODISCARD static Ref<SVGMatrix> CreateSVGMatrix() noexcept;
    KRYS_NODISCARD static Ref<SVGRect> CreateSVGRect() noexcept;
    KRYS_NODISCARD static Ref<SVGTransform> CreateSVGTransform() noexcept;
    KRYS_NODISCARD static Ref<SVGTransform>
      CreateSVGTransformFromMatrix(css::DOMMatrix2DInit &&matrixInit) noexcept;

    /// @brief The first element in the subtree of the `<svg>` element whose id is @p id.
    /// @see https://svgwg.org/svg2-draft/struct.html#__svg__SVGSVGElement__getElementById
    KRYS_NODISCARD RefPtr<dom::Element> GetElementById(dom::DOMStringView id) noexcept;

    /// @see https://svgwg.org/svg2-draft/struct.html#__svg__SVGSVGElement__suspendRedraw
    KRYS_NODISCARD uint32 SuspendRedraw(uint32) noexcept
    {
      return 1;
    }

    void UnsuspendRedraw(uint32) noexcept
    {
    }

    void UnsuspendRedrawAll() noexcept
    {
    }

    void ForceRedraw() noexcept
    {
    }

#pragma endregion

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGSvgElement, SVGGraphicsElement, SVGFitToViewBox>;

    /// @brief Whether the `<svg>` is shown with the view a fragment identifier gave it.
    KRYS_NODISCARD bool UseCurrentView() const noexcept
    {
      return _useCurrentView;
    }
    /// @brief The view a fragment identifier gives the `<svg>`, created when first asked for.
    SVGViewSpec &CurrentView() noexcept;

    bool ScrollToFragment(dom::DOMStringView fragmentIdentifier) noexcept;
    void ResetScrollAnchor() noexcept;

    void SetCurrentTranslate(const FloatPoint &translation) noexcept; // Used to pan.
    void UpdateCurrentTranslate() noexcept;

    KRYS_NODISCARD bool HasIntrinsicWidth() const noexcept;
    KRYS_NODISCARD bool HasIntrinsicHeight() const noexcept;
    KRYS_NODISCARD float IntrinsicWidth() const noexcept;
    KRYS_NODISCARD float IntrinsicHeight() const noexcept;

    KRYS_NODISCARD FloatSize CurrentViewportSizeExcludingZoom() const noexcept;
    KRYS_NODISCARD FloatRect CurrentViewBoxRect() const noexcept;

    KRYS_NODISCARD AffineTransform ViewBoxToViewTransform(float viewWidth, float viewHeight) const noexcept;
    KRYS_NODISCARD bool HasTransformRelatedAttributes() const noexcept final;

    void InheritViewAttributes(const SVGViewElement &) noexcept;

    KRYS_NODISCARD const SVGLengthValue &X() const noexcept
    {
      return _x->CurrentValue();
    }

    KRYS_NODISCARD const SVGLengthValue &Y() const noexcept
    {
      return _y->CurrentValue();
    }

    KRYS_NODISCARD const SVGLengthValue &Width() const noexcept
    {
      return _width->CurrentValue();
    }

    KRYS_NODISCARD const SVGLengthValue &Height() const noexcept
    {
      return _height->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedLength &XAnimated() noexcept
    {
      return *_x;
    }

    KRYS_NODISCARD SVGAnimatedLength &YAnimated() noexcept
    {
      return *_y;
    }

    KRYS_NODISCARD SVGAnimatedLength &WidthAnimated() noexcept
    {
      return *_width;
    }

    KRYS_NODISCARD SVGAnimatedLength &HeightAnimated() noexcept
    {
      return *_height;
    }

    KRYS_NODISCARD bool IsReplaced(const RenderStyle * = nullptr) const noexcept final;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept override;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept override;

  private:
    KRYS_NODISCARD AffineTransform LocalCoordinateSpaceTransform(CTMScope) const noexcept override;
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;
    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept override;
    KRYS_NODISCARD bool IsValid() const noexcept override;

    KRYS_NODISCARD LocalFrame *FrameForCurrentScale() const noexcept;

    KRYS_NODISCARD Ref<dom::NodeList>
      CollectIntersectionOrEnclosureList(SVGRect &, SVGElement *,
                                         bool (*checkFunction)(SVGElement &, SVGRect &)) noexcept;

    KRYS_NODISCARD RefPtr<SVGViewElement>
      FindViewAnchor(dom::DOMStringView fragmentIdentifier) const noexcept;
    KRYS_NODISCARD SVGSvgElement *FindRootAnchor(const SVGViewElement *) const noexcept;
    KRYS_NODISCARD SVGSvgElement *FindRootAnchor(dom::DOMStringView) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGSvgElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGSvgElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
