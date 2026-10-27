#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/CSS/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Internals/NodeList/StaticNodeList.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGViewportContainer.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGAngle.hpp"
#include "Krystal.Booey/SVG/SVGLength.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGMatrix.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGNumber.hpp"
#include "Krystal.Booey/SVG/SVGRect.hpp"
#include "Krystal.Booey/SVG/SVGTransform.hpp"
#include "Krystal.Booey/SVG/SVGTransformList.hpp"
#include "Krystal.Booey/SVG/SVGViewElement.hpp"
#include "Krystal.Booey/SVG/SVGViewSpec.hpp"
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

  SVGSvgElement::~SVGSvgElement() noexcept
  {
    if (auto viewSpec = _viewSpec)
    {
      viewSpec->ResetContextElement();
    }
    // FIXME: Unregister from the document's suspension callbacks and remove the time container, with SMIL
    // (B11i).
  }

  SVGViewSpec &SVGSvgElement::CurrentView() noexcept
  {
    if (!_viewSpec)
    {
      _viewSpec = SVGViewSpec::Create(*this);
    }
    return *_viewSpec;
  }

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

    UpdateSVGRendererForElementChange();
    if (ParentNode() == &NodeDocument() && NodeDocument().GetRenderView())
    {
      NodeDocument().GetRenderView()->Repaint();
    }
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

      UpdateSVGRendererForElementChange();
      return;
    }

    if (SVGFitToViewBox::IsKnownAttribute(attributeName))
    {
      if (auto *renderer = Renderer())
      {
        renderer->SetNeedsTransformUpdate();
        if (auto *svgRoot = DynamicDowncast<LegacyRenderSVGRoot>(*renderer))
        {
          svgRoot->SetNeedsLayoutIfNeededAfterIntrinsicSizeChange();
        }
      }

      UpdateSVGRendererForElementChange();
      return;
    }

    SVGGraphicsElement::SVGAttributeChanged(attributeName);
  }

  bool SVGSvgElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    if (!IsValid())
    {
      return false;
    }
    // FIXME: We should respect display: none on the documentElement svg element
    // but many things in FrameView and SVGImage depend on the LegacyRenderSVGRoot when
    // they should instead depend on the RenderView.
    // https://bugs.webkit.org/show_bug.cgi?id=103493
    if (NodeDocument().DocumentElement().get() == this)
    {
      return true;
    }
    return dom::StyledElement::RendererIsNeeded(style);
  }

  RenderPtr<RenderElement> SVGSvgElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    if (IsOutermostSVGSVGElement())
    {
      return CreateRenderer<LegacyRenderSVGRoot>(*this, krys::move(style));
    }

    return CreateRenderer<LegacyRenderSVGViewportContainer>(*this, krys::move(style));
  }

  namespace
  {
    constexpr EnumSet<dom::LayoutOptions> SvgQueryLayoutOptions {
      dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
      dom::LayoutOptions::TreatContentVisibilityAutoAsVisible};

    bool CheckIntersectionWithoutUpdatingLayout(SVGElement &element, SVGRect &rect) noexcept
    {
      return LegacyRenderSVGModelObject::CheckIntersection(element.Renderer(), rect.Value());
    }

    bool CheckEnclosureWithoutUpdatingLayout(SVGElement &element, SVGRect &rect) noexcept
    {
      return LegacyRenderSVGModelObject::CheckEnclosure(element.Renderer(), rect.Value());
    }
  }

  Ref<dom::NodeList> SVGSvgElement::GetIntersectionList(SVGRect &rect, SVGElement *referenceElement) noexcept
  {
    (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(SvgQueryLayoutOptions, this);
    return CollectIntersectionOrEnclosureList(rect, referenceElement, CheckIntersectionWithoutUpdatingLayout);
  }

  Ref<dom::NodeList> SVGSvgElement::GetEnclosureList(SVGRect &rect, SVGElement *referenceElement) noexcept
  {
    (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets(SvgQueryLayoutOptions, this);
    return CollectIntersectionOrEnclosureList(rect, referenceElement, CheckEnclosureWithoutUpdatingLayout);
  }

  bool SVGSvgElement::CheckIntersection(Ref<SVGElement> &&element, SVGRect &rect) noexcept
  {
    (void)element->NodeDocument().UpdateLayoutIgnorePendingStylesheets(SvgQueryLayoutOptions, element.get());
    return CheckIntersectionWithoutUpdatingLayout(*element, rect);
  }

  bool SVGSvgElement::CheckEnclosure(Ref<SVGElement> &&element, SVGRect &rect) noexcept
  {
    (void)element->NodeDocument().UpdateLayoutIgnorePendingStylesheets(SvgQueryLayoutOptions, element.get());
    return CheckEnclosureWithoutUpdatingLayout(*element, rect);
  }

  Ref<dom::NodeList>
    SVGSvgElement::CollectIntersectionOrEnclosureList(SVGRect &rect, SVGElement *referenceElement,
                                                      bool (*checkFunction)(SVGElement &, SVGRect &)) noexcept
  {
    dom::SmallNodeList elements;
    dom::ContainerNode &root =
      referenceElement ? static_cast<dom::ContainerNode &>(*referenceElement) : *this;
    for (auto *node = root.FirstChild(); node; node = dom::TreeQueries::Next(*node, &root))
    {
      auto *element = DynamicDowncast<SVGElement>(*node);
      if (element && checkFunction(*element, rect))
      {
        elements.push_back(ShareRef(*element));
      }
    }
    return AdoptRef(*new dom::StaticNodeList(krys::move(elements)));
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
      if (auto *renderer = Renderer())
      {
        FloatPoint location;
        float zoomFactor = 1;

        // At the SVG/HTML boundary (aka LegacyRenderSVGRoot), we apply the localToBorderBoxTransform
        // to map an element from SVG viewport coordinates to CSS box coordinates.
        // LegacyRenderSVGRoot's localToAbsolute method expects CSS box coordinates.
        // We also need to adjust for the zoom level factored into CSS coordinates (bug #96361).
        if (auto *legacyRenderSVGRoot = DynamicDowncast<LegacyRenderSVGRoot>(*renderer))
        {
          location = legacyRenderSVGRoot->LocalToBorderBoxTransform().MapPoint(location);
          zoomFactor = 1 / renderer->Style().UsedZoom();
        }

        // Translate in our CSS parent coordinate space
        // FIXME: This doesn't work correctly with CSS transforms.
        location = renderer->LocalToAbsolute(location, MapCoordinatesMode::UseTransforms);
        location.Scale(zoomFactor);

        // Be careful here! localToBorderBoxTransform() included the x/y offset coming from the
        // viewBoxToViewTransform(), so we have to subtract it here (original cause of bug #27183)
        transform.Translate(location.X() - viewBoxTransform.E(), location.Y() - viewBoxTransform.F());

        // Respect scroll offset.
        if (auto *view = NodeDocument().View())
        {
          LayoutPoint scrollPosition = view->GetScrollPosition();
          scrollPosition.Scale(zoomFactor);
          transform.Translate(-scrollPosition);
        }
      }
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
    if (_useCurrentView)
    {
      if (auto viewSpec = _viewSpec)
      {
        return viewSpec->ViewBox();
      }
      return {};
    }

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

    if (Renderer())
    {
      if (auto *svgRoot = DynamicDowncast<LegacyRenderSVGRoot>(Renderer()))
      {
        viewportSize = svgRoot->ContentBoxRect().Size() / svgRoot->Style().UsedZoom();
      }
      else if (auto *svgViewportContainer = DynamicDowncast<LegacyRenderSVGViewportContainer>(Renderer()))
      {
        viewportSize = svgViewportContainer->Viewport().Size();
      }
      else
      {
        krys_debug_assert(false);
        return {};
      }
    }

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
    if (!_useCurrentView || !_viewSpec)
    {
      return SVGFitToViewBox::ViewBoxToViewTransform(CurrentViewBoxRect(), PreserveAspectRatio(), viewWidth,
                                                     viewHeight);
    }

    auto viewSpec = _viewSpec;
    AffineTransform transform = SVGFitToViewBox::ViewBoxToViewTransform(
      CurrentViewBoxRect(), viewSpec->PreserveAspectRatio(), viewWidth, viewHeight);
    transform *= viewSpec->Transform()->Concatenate();
    return transform;
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

  bool SVGSvgElement::ScrollToFragment(dom::DOMStringView fragmentIdentifier) noexcept
  {
    auto *renderer = Renderer();

    auto view = _viewSpec;
    if (view)
    {
      view->Reset();
    }

    bool hadUseCurrentView = _useCurrentView;
    _useCurrentView = false;

    auto invalidateView = [](RenderElement &renderer)
    {
      LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidation(renderer);
    };

    if (fragmentIdentifier.starts_with(u8"svgView("))
    {
      if (!view)
      {
        view = ShareRefPtr(&CurrentView()); // Create the SVGViewSpec.
      }
      if (view->ParseViewSpec(fragmentIdentifier))
      {
        _useCurrentView = true;
      }
      else
      {
        view->Reset();
      }
      if (renderer && (hadUseCurrentView || _useCurrentView))
      {
        invalidateView(*renderer);
      }
      return _useCurrentView;
    }

    // Spec: If the SVG fragment identifier addresses a "view" element within an SVG document (e.g.,
    // MyDrawing.svg#MyView) then the root 'svg' element is displayed in the SVG viewport. Any view
    // specification attributes included on the given "view" element override the corresponding view
    // specification attributes on the root 'svg' element.
    if (auto viewElement = FindViewAnchor(fragmentIdentifier))
    {
      if (auto *rootElement = FindRootAnchor(viewElement.get()))
      {
        if (auto *currentViewElement = rootElement->_currentViewElement.get())
        {
          krys_debug_assert(currentViewElement->TargetElement() == rootElement);

          // If the viewElement has changed, remove the link from the SVGViewElement to the previously
          // selected SVGSVGElement.
          if (currentViewElement != viewElement.get())
          {
            currentViewElement->ResetTargetElement();
          }
        }

        if (rootElement->_currentViewElement.get() != viewElement.get())
        {
          rootElement->_currentViewElement = WeakPtr<SVGViewElement>::WithRef(*viewElement);
          viewElement->SetTargetElement(*rootElement);
        }

        rootElement->InheritViewAttributes(*viewElement);
        if (auto *rootRenderer = rootElement->Renderer())
        {
          invalidateView(*rootRenderer);
        }
        _currentViewFragmentIdentifier = dom::DOMString(fragmentIdentifier);
        return true;
      }
    }

    // FIXME: We need to decide which <svg> to focus on, and zoom to it.
    // FIXME: We need to actually "highlight" the viewTarget(s).
    return false;
  }

  void SVGSvgElement::ResetScrollAnchor() noexcept
  {
    if (!_useCurrentView && _currentViewFragmentIdentifier.empty())
    {
      return;
    }

    if (auto viewSpec = _viewSpec)
    {
      viewSpec->Reset();
    }

    if (!_currentViewFragmentIdentifier.empty())
    {
      if (auto *rootElement = FindRootAnchor(_currentViewFragmentIdentifier))
      {
        auto &view = rootElement->CurrentView();
        view.SetViewBox(ViewBox());
        view.SetPreserveAspectRatio(PreserveAspectRatio());
        view.SetZoomAndPan(ZoomAndPan());
        _currentViewFragmentIdentifier = {};
      }
    }

    _useCurrentView = false;

    auto *renderer = Renderer();
    if (!renderer)
    {
      return;
    }

    LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidation(*renderer);
  }

  void SVGSvgElement::InheritViewAttributes(const SVGViewElement &viewElement) noexcept
  {
    auto &view = CurrentView();
    _useCurrentView = true;

    auto noNamespace = dom::DOMStringAtom::Null();
    if (viewElement.HasAttributeNS(noNamespace, SVGNames::ViewBoxAttr().LocalName()))
    {
      view.SetViewBox(viewElement.ViewBox());
    }
    else
    {
      view.SetViewBox(ViewBox());
    }

    if (viewElement.HasAttributeNS(noNamespace, SVGNames::PreserveAspectRatioAttr().LocalName()))
    {
      view.SetPreserveAspectRatio(viewElement.PreserveAspectRatio());
    }
    else
    {
      view.SetPreserveAspectRatio(PreserveAspectRatio());
    }

    if (viewElement.HasAttributeNS(noNamespace, SVGNames::ZoomAndPanAttr().LocalName()))
    {
      view.SetZoomAndPan(viewElement.ZoomAndPan());
    }
    else
    {
      view.SetZoomAndPan(ZoomAndPan());
    }
  }

  RefPtr<SVGViewElement> SVGSvgElement::FindViewAnchor(dom::DOMStringView fragmentIdentifier) const noexcept
  {
    auto *anchor = NodeDocument().FindAnchor(fragmentIdentifier);
    auto *svgAnchor = anchor ? DynamicDowncast<SVGElement>(*anchor) : nullptr;
    return svgAnchor ? ShareRefPtr(DynamicDowncast<SVGViewElement>(*svgAnchor)) : nullptr;
  }

  SVGSvgElement *SVGSvgElement::FindRootAnchor(const SVGViewElement *viewElement) const noexcept
  {
    if (!viewElement)
    {
      return nullptr;
    }

    auto documentElement = viewElement->NodeDocument().DocumentElement();
    auto *svgElement = documentElement ? DynamicDowncast<SVGElement>(*documentElement) : nullptr;
    return svgElement ? DynamicDowncast<SVGSvgElement>(*svgElement) : nullptr;
  }

  SVGSvgElement *SVGSvgElement::FindRootAnchor(dom::DOMStringView fragmentIdentifier) const noexcept
  {
    if (auto viewElement = FindViewAnchor(fragmentIdentifier))
    {
      return FindRootAnchor(viewElement.get());
    }
    return nullptr;
  }
}
