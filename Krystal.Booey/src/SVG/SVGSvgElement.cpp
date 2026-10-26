#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/CSS/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGAngle.hpp"
#include "Krystal.Booey/SVG/SVGLength.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGMatrix.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGNumber.hpp"
#include "Krystal.Booey/SVG/SVGRect.hpp"
#include "Krystal.Booey/SVG/SVGTransform.hpp"
#include <cmath>

namespace krys::boo::svg
{
  SVGSvgElement::SVGSvgElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Svg),
        SVGFitToViewBox(this), _x(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _y(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _width(SVGAnimatedLength::Create(this, SVGLengthMode::Width, u8"100%")),
        _height(SVGAnimatedLength::Create(this, SVGLengthMode::Height, u8"100%"))
  {
    // FIXME: Register for the document's suspension callbacks (to pause and resume animations), and create
    // the time container, with SMIL (B11i).
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGSvgElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGSvgElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGSvgElement::_width>(SVGNames::WidthAttr());
      PropertyRegistry::RegisterProperty<&SVGSvgElement::_height>(SVGNames::HeightAttr());
    }
  }

  SVGSvgElement::~SVGSvgElement() noexcept = default;

  LocalFrame *SVGSvgElement::FrameForCurrentScale() const noexcept
  {
    // The behavior of currentScale() is undefined when we're dealing with non-standalone SVG documents.
    // If the document is embedded, the scaling is handled by the host renderer.
    if (!IsConnected() || !IsOutermostSVGSVGElement() || NodeDocument().DocumentElement().get() != this)
    {
      return nullptr;
    }
    return NodeDocument().Frame();
  }

  float SVGSvgElement::CurrentScale() const noexcept
  {
    // When asking from inside an embedded SVG document, a scale value of 1 seems reasonable, as it doesn't
    // know anything about the parent scale.
    auto *frame = FrameForCurrentScale();
    return frame ? frame->PageZoomFactor() : 1;
  }

  void SVGSvgElement::SetCurrentScale(float scale) noexcept
  {
    krys_debug_assert(std::isfinite(scale));
    if (auto *frame = FrameForCurrentScale())
    {
      frame->SetPageZoomFactor(scale);
    }
  }

  void SVGSvgElement::SetCurrentTranslate(const FloatPoint &translation) noexcept
  {
    if (_currentTranslate->Value() == translation)
    {
      return;
    }
    _currentTranslate->SetValue(translation);
    UpdateCurrentTranslate();
  }

  void SVGSvgElement::UpdateCurrentTranslate() noexcept
  {
    if (!Renderer())
    {
      return;
    }

    // FIXME: Update the renderer, and repaint the view when this is the document element
    // (updateSVGRendererForElementChange), once there are SVG renderers (B11b).
  }

  void SVGSvgElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                       Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::XAttr())
    {
      _x->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::YAttr())
    {
      _y->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    else if (name == SVGNames::WidthAttr())
    {
      _width->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                           SVGLengthNegativeValuesMode::Forbid, u8"100%"));
    }
    else if (name == SVGNames::HeightAttr())
    {
      _height->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                            SVGLengthNegativeValuesMode::Forbid, u8"100%"));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGFitToViewBox::ParseAttribute(name, value);
    SVGZoomAndPan::ParseAttribute(*this, name, value);
    SVGGraphicsElement::AttributeChanged(name, oldValue, value);
  }

  void SVGSvgElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      InstanceInvalidationGuard guard(*this);
      SetPresentationalHintStyleIsDirty();

      // FIXME: Lay out the view of an SVG document embedded through a frame when its width or height changes,
      // and update the renderer (updateSVGRendererForElementChange), once there are SVG renderers (B11b).
      return;
    }

    if (SVGFitToViewBox::IsKnownAttribute(attributeName))
    {
      // FIXME: Mark the renderer's transform for an update, lay out the root if its intrinsic size changed
      // and update the renderer (setNeedsTransformUpdate, setNeedsLayoutIfNeededAfterIntrinsicSizeChange,
      // updateSVGRendererForElementChange), once there are SVG renderers (B11b).
      return;
    }

    SVGGraphicsElement::SVGAttributeChanged(attributeName);
  }

  void SVGSvgElement::DeselectAll() noexcept
  {
    if (NodeDocument().Frame())
    {
      NodeDocument().Selection().Clear();
    }
  }

  Ref<SVGNumber> SVGSvgElement::CreateSVGNumber() noexcept
  {
    return SVGNumber::Create();
  }

  Ref<SVGLength> SVGSvgElement::CreateSVGLength() noexcept
  {
    return SVGLength::Create();
  }

  Ref<SVGAngle> SVGSvgElement::CreateSVGAngle() noexcept
  {
    return SVGAngle::Create();
  }

  Ref<SVGPoint> SVGSvgElement::CreateSVGPoint() noexcept
  {
    return SVGPoint::Create();
  }

  Ref<SVGMatrix> SVGSvgElement::CreateSVGMatrix() noexcept
  {
    return SVGMatrix::Create();
  }

  Ref<SVGRect> SVGSvgElement::CreateSVGRect() noexcept
  {
    return SVGRect::Create();
  }

  Ref<SVGTransform> SVGSvgElement::CreateSVGTransform() noexcept
  {
    return SVGTransform::Create(SVGTransformValue::SVG_TRANSFORM_MATRIX);
  }

  Ref<SVGTransform> SVGSvgElement::CreateSVGTransformFromMatrix(css::DOMMatrix2DInit &&matrixInit) noexcept
  {
    AffineTransform transform;
    if (matrixInit.A)
    {
      transform.SetA(*matrixInit.A);
    }
    if (matrixInit.B)
    {
      transform.SetB(*matrixInit.B);
    }
    if (matrixInit.C)
    {
      transform.SetC(*matrixInit.C);
    }
    if (matrixInit.D)
    {
      transform.SetD(*matrixInit.D);
    }
    if (matrixInit.E)
    {
      transform.SetE(*matrixInit.E);
    }
    if (matrixInit.F)
    {
      transform.SetF(*matrixInit.F);
    }
    return SVGTransform::Create(transform);
  }

  AffineTransform SVGSvgElement::LocalCoordinateSpaceTransform(CTMScope mode) const noexcept
  {
    AffineTransform viewBoxTransform;
    if (!HasEmptyViewBox())
    {
      FloatSize size = CurrentViewportSizeExcludingZoom();
      viewBoxTransform = ViewBoxToViewTransform(size.Width(), size.Height());
    }

    AffineTransform transform;
    if (!IsOutermostSVGSVGElement())
    {
      SVGLengthContext lengthContext(this);
      transform.Translate(X().Value(lengthContext), Y().Value(lengthContext));
    }
    else if (mode == CTMScope::ScreenScope)
    {
      // FIXME: Translate to the renderer's place in its CSS parent coordinate space (mapping through the
      // root's localToBorderBoxTransform, excluding zoom, less the view box transform's translation and the
      // view's scroll position), once there are SVG renderers (B11b).
    }

    return transform.Multiply(viewBoxTransform);
  }

  bool SVGSvgElement::IsReplaced(const RenderStyle *) const noexcept
  {
    return IsOutermostSVGSVGElement();
  }

  bool SVGSvgElement::SelfHasRelativeLengths() const noexcept
  {
    return X().IsRelative() || Y().IsRelative() || Width().IsRelative() || Height().IsRelative()
           || HasAttributeNS(dom::DOMStringAtom::Null(), SVGNames::ViewBoxAttr().LocalName());
  }

  bool SVGSvgElement::HasTransformRelatedAttributes() const noexcept
  {
    if (SVGGraphicsElement::HasTransformRelatedAttributes())
    {
      return true;
    }

    // 'x' / 'y' / 'viewBox' lead to a non-identity supplementalLayerTransform in RenderSVGViewportContainer
    auto noNamespace = dom::DOMStringAtom::Null();
    return (HasAttributeNS(noNamespace, SVGNames::XAttr().LocalName())
            || HasAttributeNS(noNamespace, SVGNames::YAttr().LocalName()))
           || (HasAttributeNS(noNamespace, SVGNames::ViewBoxAttr().LocalName()) && !HasEmptyViewBox());
  }

  FloatRect SVGSvgElement::CurrentViewBoxRect() const noexcept
  {
    // FIXME: The view box of the current view (SVGViewSpec), when a fragment identifier picks one, with
    // <view> (B11b).

    auto viewBox = ViewBox();
    if (!viewBox.IsEmpty())
    {
      return viewBox;
    }

    // FIXME: If no viewBox is specified but non-relative width/height values, then we should always
    // synthesize a viewBox if we're embedded through a SVGImage, once there are SVG images (B11d).
    return {};
  }

  FloatSize SVGSvgElement::CurrentViewportSizeExcludingZoom() const noexcept
  {
    FloatSize viewportSize;

    // FIXME: The root renderer's content box size excluding zoom, or the viewport container's viewport size,
    // once there are SVG renderers (B11b).

    if (!viewportSize.IsEmpty())
    {
      return viewportSize;
    }

    if (!(HasIntrinsicWidth() && HasIntrinsicHeight()))
    {
      return {};
    }

    return {IntrinsicWidth(), IntrinsicHeight()};
  }

  bool SVGSvgElement::HasIntrinsicWidth() const noexcept
  {
    return Width().LengthType() != SVGLengthType::Percentage;
  }

  bool SVGSvgElement::HasIntrinsicHeight() const noexcept
  {
    return Height().LengthType() != SVGLengthType::Percentage;
  }

  float SVGSvgElement::IntrinsicWidth() const noexcept
  {
    if (Width().LengthType() == SVGLengthType::Percentage)
    {
      return 0;
    }

    SVGLengthContext lengthContext(this);
    return Width().Value(lengthContext);
  }

  float SVGSvgElement::IntrinsicHeight() const noexcept
  {
    if (Height().LengthType() == SVGLengthType::Percentage)
    {
      return 0;
    }

    SVGLengthContext lengthContext(this);
    return Height().Value(lengthContext);
  }

  AffineTransform SVGSvgElement::ViewBoxToViewTransform(float viewWidth, float viewHeight) const noexcept
  {
    // FIXME: The current view's aspect ratio and transform (SVGViewSpec), with <view> (B11b).
    return SVGFitToViewBox::ViewBoxToViewTransform(CurrentViewBoxRect(), PreserveAspectRatio(), viewWidth,
                                                   viewHeight);
  }

  // getElementById on SVGSVGElement is restricted to only the child subtree defined by the <svg> element.
  // See http://www.w3.org/TR/SVG11/struct.html#InterfaceSVGSVGElement
  RefPtr<dom::Element> SVGSvgElement::GetElementById(dom::DOMStringView id) noexcept
  {
    if (id.empty())
    {
      return nullptr;
    }

    return dom::mixins::NonElementParentNode::GetElementById(*this, id);
  }

  bool SVGSvgElement::IsValid() const noexcept
  {
    return SVGTests::IsValid();
  }
}
