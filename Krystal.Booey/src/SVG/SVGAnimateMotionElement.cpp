#include "Krystal.Booey/SVG/SVGAnimateMotionElement.hpp"
#include "Krystal.Booey/Graphics/PathTraversalState.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGPathData.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGMPathElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGPathElement.hpp"

namespace krys::boo::svg
{
  SVGAnimateMotionElement::SVGAnimateMotionElement(dom::Document &document) noexcept
      : SVGAnimationElement(document, SVGElementInterface::AnimateMotion)
  {
    SetCalcMode(CalcMode::Paced);
  }

  bool SVGAnimateMotionElement::HasValidAttributeType() const noexcept
  {
    RefPtr targetElement = ShareRefPtr(TargetElement());
    if (!targetElement)
    {
      return false;
    }

    // We don't have a special attribute name to verify the animation type. Check the element name instead.
    if (!Is<SVGGraphicsElement>(*targetElement))
    {
      return false;
    }
    // Spec: SVG 1.1 section 19.2.15
    // FIXME: svgTag is missing. Needs to be checked, if transforming <svg> could cause problems.
    switch (targetElement->Interface())
    {
      case SVGElementInterface::G:
      case SVGElementInterface::Defs:
      case SVGElementInterface::Use:
      case SVGElementInterface::Image:
      case SVGElementInterface::Switch:
      case SVGElementInterface::Path:
      case SVGElementInterface::Rect:
      case SVGElementInterface::Circle:
      case SVGElementInterface::Ellipse:
      case SVGElementInterface::Line:
      case SVGElementInterface::Polyline:
      case SVGElementInterface::Polygon:
      case SVGElementInterface::Text:
      case SVGElementInterface::ClipPath:
      case SVGElementInterface::Mask:
      case SVGElementInterface::A:
      case SVGElementInterface::ForeignObject:
      {
        return true;
      }
      default:
      {
        return false;
      }
    }
  }

  bool SVGAnimateMotionElement::HasValidAttributeName() const noexcept
  {
    // AnimateMotion does not use attributeName so it is always valid.
    return true;
  }

  void SVGAnimateMotionElement::AttributeChanged(const dom::QualifiedName &name,
                                                 Maybe<dom::DOMStringView> oldValue,
                                                 Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::PathAttr())
    {
      _path = BuildPathFromString(value.value_or(dom::DOMStringView {}));
      UpdateAnimationPath();
    }

    SVGAnimationElement::AttributeChanged(name, oldValue, value);
  }

  SVGAnimateMotionElement::RotateMode SVGAnimateMotionElement::GetRotateMode() const noexcept
  {
    auto rotate = AttributeValue(SVGNames::RotateAttr());
    if (rotate == dom::DOMStringView {u8"auto"})
    {
      return RotateAuto;
    }
    if (rotate == dom::DOMStringView {u8"auto-reverse"})
    {
      return RotateAutoReverse;
    }
    return RotateAngle;
  }

  void SVGAnimateMotionElement::UpdateAnimationPath() noexcept
  {
    _animationPath = Path();
    bool foundMPath = false;

    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      auto *mPath = DynamicDowncast<SVGMPathElement>(child);
      if (!mPath)
      {
        continue;
      }
      if (RefPtr pathElement = mPath->PathElement())
      {
        _animationPath = PathFromGraphicsElement(*pathElement);
        foundMPath = true;
        break;
      }
    }

    if (!foundMPath && AttributeValue(SVGNames::PathAttr()))
    {
      _animationPath = _path;
    }

    UpdateAnimationMode();
  }

  void SVGAnimateMotionElement::StartAnimation() noexcept
  {
    if (!HasValidAttributeType())
    {
      return;
    }
    RefPtr targetElement = ShareRefPtr(TargetElement());
    if (!targetElement)
    {
      return;
    }
    if (auto *transform = targetElement->EnsureSupplementalTransform())
    {
      transform->MakeIdentity();
    }
  }

  void SVGAnimateMotionElement::StopAnimation(SVGElement *targetElement) noexcept
  {
    if (!targetElement)
    {
      return;
    }
    if (auto *transform = targetElement->EnsureSupplementalTransform())
    {
      transform->MakeIdentity();
    }
    ApplyResultsToTarget();
  }

  bool SVGAnimateMotionElement::SetFromAndToValues(dom::DOMStringView fromString,
                                                   dom::DOMStringView toString) noexcept
  {
    _toPointAtEndOfDuration = {};
    _fromPoint = ParsePoint(fromString).value_or(FloatPoint {});
    _toPoint = ParsePoint(toString).value_or(FloatPoint {});
    return true;
  }

  bool SVGAnimateMotionElement::SetFromAndByValues(dom::DOMStringView fromString,
                                                   dom::DOMStringView byString) noexcept
  {
    _toPointAtEndOfDuration = {};
    if (GetAnimationMode() == AnimationMode::By && !IsAdditive())
    {
      return false;
    }
    _fromPoint = ParsePoint(fromString).value_or(FloatPoint {});
    auto byPoint = ParsePoint(byString).value_or(FloatPoint {});
    _toPoint = FloatPoint(_fromPoint.X() + byPoint.X(), _fromPoint.Y() + byPoint.Y());
    return true;
  }

  bool SVGAnimateMotionElement::SetToAtEndOfDurationValue(dom::DOMStringView toAtEndOfDurationString) noexcept
  {
    _toPointAtEndOfDuration = ParsePoint(toAtEndOfDurationString).value_or(FloatPoint {});
    return true;
  }

  void SVGAnimateMotionElement::BuildTransformForProgress(AffineTransform *transform,
                                                          float percentage) const noexcept
  {
    krys_debug_assert(!_animationPath.IsEmpty());

    float positionOnPath = _animationPath.Length() * percentage;
    auto traversalState = _animationPath.TraversalStateAtLength(positionOnPath);
    if (!traversalState.Success())
    {
      return;
    }

    FloatPoint position = traversalState.Current();
    transform->Translate(position);
  }

  void SVGAnimateMotionElement::CalculateAnimatedValue(float percentage, uint32 repeatCount) noexcept
  {
    RefPtr targetElement = ShareRefPtr(TargetElement());
    if (!targetElement)
    {
      return;
    }

    auto *transform = targetElement->EnsureSupplementalTransform();
    if (!transform)
    {
      return;
    }

    if (!IsAdditive())
    {
      transform->MakeIdentity();
    }

    if (GetAnimationMode() != AnimationMode::Path)
    {
      FloatPoint toPointAtEndOfDuration = _toPoint;
      if (IsAccumulated() && repeatCount && _toPointAtEndOfDuration)
      {
        toPointAtEndOfDuration = *_toPointAtEndOfDuration;
      }

      float animatedX = 0;
      AnimateAdditiveNumber(percentage, repeatCount, _fromPoint.X(), _toPoint.X(), toPointAtEndOfDuration.X(),
                            animatedX);

      float animatedY = 0;
      AnimateAdditiveNumber(percentage, repeatCount, _fromPoint.Y(), _toPoint.Y(), toPointAtEndOfDuration.Y(),
                            animatedY);

      transform->Translate(animatedX, animatedY);
      return;
    }

    BuildTransformForProgress(transform, percentage);

    // Handle accumulate="sum".
    if (IsAccumulated() && repeatCount)
    {
      for (uint32 i = 0; i < repeatCount; ++i)
      {
        BuildTransformForProgress(transform, 1);
      }
    }
    float positionOnPath = _animationPath.Length() * percentage;
    auto traversalState = _animationPath.TraversalStateAtLength(positionOnPath);

    // The 'angle' below is in 'degrees'.
    float angle = traversalState.NormalAngle();
    RotateMode rotateMode = GetRotateMode();
    if (rotateMode != RotateAuto && rotateMode != RotateAutoReverse)
    {
      return;
    }
    if (rotateMode == RotateAutoReverse)
    {
      angle += 180;
    }
    transform->Rotate(angle);
  }

  void SVGAnimateMotionElement::ApplyResultsToTarget() noexcept
  {
    // We accumulate to the target element transform list so there is not much to do here.
    RefPtr targetElement = ShareRefPtr(TargetElement());
    if (!targetElement)
    {
      return;
    }

    auto updateTargetElement = [](SVGElement &element)
    {
      if (auto *renderer = element.Renderer())
      {
        renderer->SetNeedsTransformUpdate();
      }
      element.UpdateSVGRendererForElementChange();
    };

    updateTargetElement(*targetElement);

    auto *targetSupplementalTransform = targetElement->EnsureSupplementalTransform();
    if (!targetSupplementalTransform)
    {
      return;
    }

    // ...except in case where we have additional instances in <use> trees.
    for (auto &instance : targetElement->Instances())
    {
      auto *transform = instance->EnsureSupplementalTransform();
      if (!transform || *transform == *targetSupplementalTransform)
      {
        continue;
      }
      *transform = *targetSupplementalTransform;
      updateTargetElement(*instance);
    }
  }

  Maybe<float> SVGAnimateMotionElement::CalculateDistance(dom::DOMStringView fromString,
                                                          dom::DOMStringView toString) noexcept
  {
    auto from = ParsePoint(fromString);
    if (!from)
    {
      return {};
    }
    auto to = ParsePoint(toString);
    if (!to)
    {
      return {};
    }
    return (*to - *from).DiagonalLength();
  }

  void SVGAnimateMotionElement::UpdateAnimationMode() noexcept
  {
    if (!_animationPath.IsEmpty())
    {
      SetAnimationMode(AnimationMode::Path);
    }
    else
    {
      SVGAnimationElement::UpdateAnimationMode();
    }
  }

  void SVGAnimateMotionElement::OnChildrenChanged() noexcept
  {
    SVGElement::OnChildrenChanged();
    // NOTE: The hook doesn't say what changed, so the path is rebuilt for any change (only removals can
    // change it: an inserted <mpath> updates the path itself once it's connected).
    UpdateAnimationPath();
  }
}
