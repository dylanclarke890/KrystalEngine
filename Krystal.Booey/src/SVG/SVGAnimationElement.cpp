#include "Krystal.Booey/SVG/SVGAnimationElement.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGAnimateElementBase.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace krys::boo::svg
{
  namespace
  {
    template <typename TPredicate>
    KRYS_NODISCARD dom::DOMStringView Trim(dom::DOMStringView string, TPredicate isWhitespace) noexcept
    {
      while (!string.empty() && isWhitespace(string.front()))
      {
        string.remove_prefix(1);
      }
      while (!string.empty() && isWhitespace(string.back()))
      {
        string.remove_suffix(1);
      }
      return string;
    }

    /// @brief Calls @p function with each part of @p string between semicolons.
    template <typename TFunction>
    void ForEachSemicolonSeparated(dom::DOMStringView string, TFunction function) noexcept
    {
      while (true)
      {
        auto separator = string.find(u8';');
        function(string.substr(0, separator));
        if (separator == dom::DOMStringView::npos)
        {
          return;
        }
        string.remove_prefix(separator + 1);
      }
    }

    KRYS_NODISCARD List<float> ParseKeyTimes(dom::DOMStringView value, bool verifyOrder) noexcept
    {
      List<float> result;
      bool failed = false;

      ForEachSemicolonSeparated(value,
                                [&](dom::DOMStringView keyTime)
                                {
                                  if (failed)
                                  {
                                    return;
                                  }

                                  keyTime = Trim(keyTime, &text::IsUnicodeCompatibleASCIIWhitespace<char8_t>);

                                  auto parsed = ToDouble(keyTime);
                                  auto time = static_cast<float>(parsed.value_or(0));
                                  if (!parsed || time < 0 || time > 1)
                                  {
                                    failed = true;
                                    return;
                                  }

                                  if (verifyOrder && (result.empty() ? time != 0 : time < result.back()))
                                  {
                                    failed = true;
                                    return;
                                  }

                                  result.push_back(time);
                                });

      if (failed)
      {
        return {};
      }

      if (verifyOrder && !result.empty() && !result.back())
      {
        krys_debug_assert(!std::accumulate(result.begin(), result.end(), 0.0f));
        return {};
      }

      return result;
    }

    KRYS_NODISCARD Maybe<List<UnitBezier>> ParseKeySplines(dom::DOMStringView string) noexcept
    {
      if (string.empty())
      {
        return {};
      }

      auto buffer = string;
      SkipOptionalSVGSpaces(buffer);

      List<UnitBezier> result;

      auto isInRange = [](Maybe<float> value)
      {
        return value && *value >= 0 && *value <= 1;
      };

      while (!buffer.empty())
      {
        auto posA = ParseNumber(buffer);
        if (!isInRange(posA))
        {
          return {};
        }

        auto posB = ParseNumber(buffer);
        if (!isInRange(posB))
        {
          return {};
        }

        auto posC = ParseNumber(buffer);
        if (!isInRange(posC))
        {
          return {};
        }

        auto posD = ParseNumber(buffer, SuffixSkippingPolicy::DontSkip);
        if (!isInRange(posD))
        {
          return {};
        }

        SkipOptionalSVGSpaces(buffer);

        if (!buffer.empty() && buffer.front() == u8';')
        {
          buffer.remove_prefix(1);
        }

        SkipOptionalSVGSpaces(buffer);

        result.emplace_back(*posA, *posB, *posC, *posD);
      }

      return result;
    }

    KRYS_NODISCARD double SolveEpsilon(double duration) noexcept
    {
      return 1 / (200 * duration);
    }
  }

  SVGAnimationElement::SVGAnimationElement(dom::Document &document, SVGElementInterface interface) noexcept
      : SVGSMILElement(document, CreateUnique<PropertyRegistry>(*this), interface), SVGTests(this)
  {
  }

  bool SVGAnimationElement::IsSupportedAttribute(const dom::QualifiedName &attrName) noexcept
  {
    static const Array supportedAttributes {
      &SVGNames::ValuesAttr(),
      &SVGNames::KeyTimesAttr(),
      &SVGNames::KeyPointsAttr(),
      &SVGNames::KeySplinesAttr(),
      &SVGNames::AttributeTypeAttr(),
      &SVGNames::CalcModeAttr(),
      &SVGNames::FromAttr(),
      &SVGNames::ToAttr(),
      &SVGNames::ByAttr(),
    };
    return SVGTests::PropertyRegistry::IsKnownAttribute(attrName)
           || std::ranges::any_of(supportedAttributes,
                                  [&](const auto *name) { return name->Matches(attrName); });
  }

  void SVGAnimationElement::AttributeChanged(const dom::QualifiedName &name,
                                             Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::ParsingFailed;
    auto newValue = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::ValuesAttr())
    {
      // Per the SMIL specification, leading and trailing white space,
      // and white space before and after semicolon separators, is allowed and will be ignored.
      // http://www.w3.org/TR/SVG11/animate.html#ValuesAttribute
      _values.clear();
      if (value)
      {
        ForEachSemicolonSeparated(
          newValue, [this](dom::DOMStringView innerValue)
          { _values.emplace_back(Trim(innerValue, &text::IsASCIIWhitespace<char8_t>)); });
      }
      UpdateAnimationMode();
    }
    else if (name == SVGNames::KeyTimesAttr())
    {
      _keyTimesFromAttribute = ParseKeyTimes(newValue, true);
    }
    else if (name == SVGNames::KeyPointsAttr())
    {
      if (Interface() == SVGElementInterface::AnimateMotion)
      {
        // This is specified to be an animateMotion attribute only but it is simpler to put it here
        // where the other timing calculatations are.
        _keyPoints = ParseKeyTimes(newValue, false);
      }
    }
    else if (name == SVGNames::KeySplinesAttr())
    {
      if (auto keySplines = ParseKeySplines(newValue))
      {
        _keySplines = krys::move(*keySplines);
      }
      else
      {
        _keySplines.clear();
        ReportAttributeParsingError(parseError, name, newValue);
      }
    }
    else if (name == SVGNames::AttributeTypeAttr())
    {
      SetAttributeType(newValue);
    }
    else if (name == SVGNames::CalcModeAttr())
    {
      SetCalcMode(newValue);
    }
    else if (name == SVGNames::FromAttr() || name == SVGNames::ToAttr() || name == SVGNames::ByAttr())
    {
      UpdateAnimationMode();
    }

    SVGTests::ParseAttribute(name, value);
    SVGSMILElement::AttributeChanged(name, oldValue, value);
  }

  void SVGAnimationElement::SVGAttributeChanged(const dom::QualifiedName &attrName) noexcept
  {
    if (!IsSupportedAttribute(attrName))
    {
      SVGSMILElement::SVGAttributeChanged(attrName);
      return;
    }

    AnimationAttributeChanged();
  }

  void SVGAnimationElement::AnimationAttributeChanged() noexcept
  {
    // Assumptions may not hold after an attribute change.
    _animationValid = false;
    SetInactive();
  }

  dom::ExceptionOr<float> SVGAnimationElement::GetStartTime() const noexcept
  {
    auto intervalBegin = IntervalBegin();
    if (!intervalBegin.IsFinite())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"The animation element does not have a current interval."};
    }
    return static_cast<float>(intervalBegin.Value());
  }

  float SVGAnimationElement::GetCurrentTime() const noexcept
  {
    return static_cast<float>(Elapsed().Value());
  }

  dom::ExceptionOr<float> SVGAnimationElement::GetSimpleDuration() const noexcept
  {
    auto simpleDuration = SimpleDuration();
    if (!simpleDuration.IsFinite())
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError,
                             u8"The simple duration is not determined on the given element."};
    }
    return static_cast<float>(simpleDuration.Value());
  }

  void SVGAnimationElement::BeginElementAt(float offset) noexcept
  {
    krys_debug_assert(std::isfinite(offset));
    AddInstanceTime(Begin, Elapsed() + offset, SMILTimeWithOrigin::Origin::Script);
  }

  void SVGAnimationElement::EndElementAt(float offset) noexcept
  {
    krys_debug_assert(std::isfinite(offset));
    if (GetActiveState() == Inactive)
    {
      return;
    }
    AddInstanceTime(End, Elapsed() + offset, SMILTimeWithOrigin::Origin::Script);
  }

  void SVGAnimationElement::UpdateAnimationMode() noexcept
  {
    // http://www.w3.org/TR/2001/REC-smil-animation-20010904/#AnimFuncValues
    if (AttributeValue(SVGNames::ValuesAttr()))
    {
      SetAnimationMode(AnimationMode::Values);
    }
    else if (!ToValue().empty())
    {
      SetAnimationMode(FromValue().empty() ? AnimationMode::To : AnimationMode::FromTo);
    }
    else if (!ByValue().empty())
    {
      SetAnimationMode(FromValue().empty() ? AnimationMode::By : AnimationMode::FromBy);
    }
    else
    {
      SetAnimationMode(AnimationMode::None);
    }
  }

  void SVGAnimationElement::SetCalcMode(dom::DOMStringView calcMode) noexcept
  {
    if (calcMode == u8"discrete")
    {
      SetCalcMode(CalcMode::Discrete);
    }
    else if (calcMode == u8"linear")
    {
      SetCalcMode(CalcMode::Linear);
    }
    else if (calcMode == u8"paced")
    {
      SetCalcMode(CalcMode::Paced);
    }
    else if (calcMode == u8"spline")
    {
      SetCalcMode(CalcMode::Spline);
    }
    else
    {
      SetCalcMode(Interface() == SVGElementInterface::AnimateMotion ? CalcMode::Paced : CalcMode::Linear);
    }
  }

  void SVGAnimationElement::SetAttributeType(dom::DOMStringView attributeType) noexcept
  {
    if (attributeType == u8"CSS")
    {
      _attributeType = AttributeType::CSS;
    }
    else if (attributeType == u8"XML")
    {
      _attributeType = AttributeType::XML;
    }
    else
    {
      _attributeType = AttributeType::Auto;
    }
  }

  dom::DOMString SVGAnimationElement::ToValue() const noexcept
  {
    return AttributeValue(SVGNames::ToAttr()).value_or(dom::DOMString {});
  }

  dom::DOMString SVGAnimationElement::ByValue() const noexcept
  {
    return AttributeValue(SVGNames::ByAttr()).value_or(dom::DOMString {});
  }

  dom::DOMString SVGAnimationElement::FromValue() const noexcept
  {
    return AttributeValue(SVGNames::FromAttr()).value_or(dom::DOMString {});
  }

  bool SVGAnimationElement::IsAdditive() const noexcept
  {
    return AttributeValue(SVGNames::AdditiveAttr()) == dom::DOMStringView {u8"sum"}
           || GetAnimationMode() == AnimationMode::By;
  }

  bool SVGAnimationElement::IsAccumulated() const noexcept
  {
    return AttributeValue(SVGNames::AccumulateAttr()) == dom::DOMStringView {u8"sum"}
           && GetAnimationMode() != AnimationMode::To;
  }

  bool SVGAnimationElement::IsTargetAttributeCSSProperty(SVGElement *targetElement,
                                                         const dom::QualifiedName &attributeName) noexcept
  {
    return targetElement->IsAnimatedStyleAttribute(attributeName);
  }

  void SVGAnimationElement::CalculateKeyTimesForCalcModePaced() noexcept
  {
    krys_debug_assert(GetCalcMode() == CalcMode::Paced);
    krys_debug_assert(GetAnimationMode() == AnimationMode::Values);

    auto valuesCount = static_cast<uint32>(_values.size());
    krys_debug_assert(valuesCount >= 1);
    if (valuesCount == 1)
    {
      return;
    }

    _keyTimesForPaced.clear();

    List<float> keyTimesForPaced;
    float totalDistance = 0;
    keyTimesForPaced.push_back(0);
    for (uint32 n = 0; n < valuesCount - 1; ++n)
    {
      // Distance in any units
      auto distance = CalculateDistance(_values[n], _values[n + 1]);
      if (!distance)
      {
        return;
      }
      totalDistance += *distance;
      keyTimesForPaced.push_back(*distance);
    }
    if (!std::isfinite(totalDistance) || !totalDistance)
    {
      return;
    }

    // Normalize.
    for (size_t n = 1; n < keyTimesForPaced.size() - 1; ++n)
    {
      keyTimesForPaced[n] = keyTimesForPaced[n - 1] + keyTimesForPaced[n] / totalDistance;
    }
    keyTimesForPaced[keyTimesForPaced.size() - 1] = 1;

    _keyTimesForPaced = krys::move(keyTimesForPaced);
  }

  const List<float> &SVGAnimationElement::KeyTimes() const noexcept
  {
    return (GetCalcMode() == CalcMode::Paced && GetAnimationMode() != AnimationMode::Path)
             ? _keyTimesForPaced
             : _keyTimesFromAttribute;
  }

  uint32 SVGAnimationElement::CalculateKeyTimesIndex(float percent) const noexcept
  {
    const auto &keyTimes = KeyTimes();
    uint32 index;
    auto keyTimesCount = static_cast<uint32>(keyTimes.size());
    // For linear and spline animations, the last value must be '1'. In those
    // cases we don't need to consider the last value, since |percent| is never
    // greater than one.
    if (keyTimesCount && GetCalcMode() != CalcMode::Discrete)
    {
      --keyTimesCount;
    }
    for (index = 1; index < keyTimesCount; ++index)
    {
      if (keyTimes[index] > percent)
      {
        break;
      }
    }
    return --index;
  }

  float SVGAnimationElement::CalculatePercentForSpline(float percent, uint32 splineIndex) const noexcept
  {
    krys_debug_assert(GetCalcMode() == CalcMode::Spline);
    krys_assert(splineIndex < _keySplines.size());
    const UnitBezier &bezier = _keySplines[splineIndex];
    SMILTime duration = SimpleDuration();
    if (!duration.IsFinite())
    {
      duration = 100.0;
    }
    return static_cast<float>(bezier.Solve(percent, SolveEpsilon(duration.Value())));
  }

  float SVGAnimationElement::CalculatePercentFromKeyPoints(float percent) const noexcept
  {
    const auto &keyTimes = _keyTimesFromAttribute;

    krys_debug_assert(!_keyPoints.empty());
    krys_debug_assert(GetCalcMode() != CalcMode::Paced || GetAnimationMode() == AnimationMode::Path);
    krys_debug_assert(keyTimes.size() > 1);
    krys_debug_assert(_keyPoints.size() == keyTimes.size());

    if (percent == 1)
    {
      return _keyPoints[_keyPoints.size() - 1];
    }

    uint32 index = CalculateKeyTimesIndex(percent);
    float fromKeyPoint = _keyPoints[index];

    if (GetCalcMode() == CalcMode::Discrete)
    {
      return fromKeyPoint;
    }

    krys_debug_assert(index + 1 < keyTimes.size());
    float fromPercent = keyTimes[index];
    float toPercent = keyTimes[index + 1];
    float toKeyPoint = _keyPoints[index + 1];
    float keyPointPercent = (percent - fromPercent) / (toPercent - fromPercent);

    if (GetCalcMode() == CalcMode::Spline)
    {
      krys_debug_assert(_keySplines.size() == _keyPoints.size() - 1);
      keyPointPercent = CalculatePercentForSpline(keyPointPercent, index);
    }
    return (toKeyPoint - fromKeyPoint) * keyPointPercent + fromKeyPoint;
  }

  float SVGAnimationElement::CalculatePercentForFromTo(float percent) const noexcept
  {
    const auto &keyTimes = KeyTimes();
    if (GetCalcMode() == CalcMode::Discrete && keyTimes.size() == 2)
    {
      return percent > keyTimes[1] ? 1.0f : 0.0f;
    }

    return percent;
  }

  void SVGAnimationElement::CurrentValuesFromKeyPoints(float percent, float &effectivePercent,
                                                       dom::DOMString &from,
                                                       dom::DOMString &to) const noexcept
  {
    krys_debug_assert(!_keyPoints.empty());
    krys_debug_assert(_keyPoints.size() == KeyTimes().size());
    krys_debug_assert(GetCalcMode() != CalcMode::Paced);
    effectivePercent = CalculatePercentFromKeyPoints(percent);
    auto index = effectivePercent == 1
                   ? _values.size() - 2
                   : static_cast<size_t>(effectivePercent * static_cast<float>(_values.size() - 1));
    from = _values[index];
    to = _values[index + 1];
  }

  void SVGAnimationElement::CurrentValuesForValuesAnimation(float percent, float &effectivePercent,
                                                            dom::DOMString &from, dom::DOMString &to) noexcept
  {
    auto valuesCount = static_cast<uint32>(_values.size());
    krys_debug_assert(_animationValid);
    krys_debug_assert(valuesCount >= 1);

    if (percent == 1 || valuesCount == 1)
    {
      from = _values[valuesCount - 1];
      to = _values[valuesCount - 1];
      effectivePercent = 1;
      return;
    }

    CalcMode calcMode = GetCalcMode();
    if (Interface() == SVGElementInterface::Animate)
    {
      krys_debug_assert(TargetElement());
      if (static_cast<SVGAnimateElementBase &>(*this).IsDiscreteAnimator())
      {
        calcMode = CalcMode::Discrete;
      }
    }
    if (!_keyPoints.empty() && calcMode != CalcMode::Paced)
    {
      return CurrentValuesFromKeyPoints(percent, effectivePercent, from, to);
    }

    const auto &keyTimes = KeyTimes();
    auto keyTimesCount = static_cast<uint32>(keyTimes.size());
    krys_debug_assert(!keyTimesCount || valuesCount == keyTimesCount);
    krys_debug_assert(!keyTimesCount || (keyTimesCount > 1 && !keyTimes[0]));

    uint32 index = CalculateKeyTimesIndex(percent);
    if (calcMode == CalcMode::Discrete)
    {
      if (!keyTimesCount)
      {
        index = static_cast<uint32>(percent * static_cast<float>(valuesCount));
      }
      from = _values[index];
      to = _values[index];
      effectivePercent = 0;
      return;
    }

    float fromPercent;
    float toPercent;
    if (keyTimesCount)
    {
      fromPercent = keyTimes[index];
      toPercent = keyTimes[index + 1];
    }
    else
    {
      index = static_cast<uint32>(std::floor(percent * static_cast<float>(valuesCount - 1)));
      fromPercent = static_cast<float>(index) / static_cast<float>(valuesCount - 1);
      toPercent = static_cast<float>(index + 1) / static_cast<float>(valuesCount - 1);
    }

    if (index == valuesCount - 1)
    {
      --index;
    }
    from = _values[index];
    to = _values[index + 1];
    krys_debug_assert(toPercent > fromPercent);
    effectivePercent = (percent - fromPercent) / (toPercent - fromPercent);

    if (calcMode == CalcMode::Spline)
    {
      krys_debug_assert(_keySplines.size() == _values.size() - 1);
      effectivePercent = CalculatePercentForSpline(effectivePercent, index);
    }
  }

  void SVGAnimationElement::StartedActiveInterval() noexcept
  {
    _animationValid = false;

    if (!HasValidAttributeType())
    {
      return;
    }

    bool hasKeyPoints = AttributeValue(SVGNames::KeyPointsAttr()).has_value();
    bool hasKeyTimes = AttributeValue(SVGNames::KeyTimesAttr()).has_value();

    // These validations are appropriate for all animation modes.
    if (hasKeyPoints && _keyPoints.size() != _keyTimesFromAttribute.size())
    {
      return;
    }

    const auto &keyTimes = KeyTimes();

    AnimationMode animationMode = GetAnimationMode();
    CalcMode calcMode = GetCalcMode();
    if (calcMode == CalcMode::Spline)
    {
      auto splinesCount = _keySplines.size();
      if (!splinesCount || (hasKeyPoints && _keyPoints.size() - 1 != splinesCount)
          || (animationMode == AnimationMode::Values && _values.size() - 1 != splinesCount)
          || (hasKeyTimes && keyTimes.size() - 1 != splinesCount))
      {
        return;
      }
    }

    dom::DOMString from = FromValue();
    dom::DOMString to = ToValue();
    dom::DOMString by = ByValue();
    if (animationMode == AnimationMode::None)
    {
      return;
    }
    if ((animationMode == AnimationMode::FromTo || animationMode == AnimationMode::FromBy
         || animationMode == AnimationMode::To || animationMode == AnimationMode::By)
        && (hasKeyPoints && hasKeyTimes
            && (keyTimes.size() < 2 || keyTimes.back() != 1 || keyTimes.size() != _keyPoints.size())))
    {
      return;
    }
    if (animationMode == AnimationMode::FromTo)
    {
      _animationValid = SetFromAndToValues(from, to);
    }
    else if (animationMode == AnimationMode::To)
    {
      // For to-animations the from value is the current accumulated value from lower priority animations.
      // The value is not static and is determined during the animation.
      _animationValid = SetFromAndToValues({}, to);
    }
    else if (animationMode == AnimationMode::FromBy)
    {
      _animationValid = SetFromAndByValues(from, by);
    }
    else if (animationMode == AnimationMode::By)
    {
      _animationValid = SetFromAndByValues({}, by);
    }
    else if (animationMode == AnimationMode::Values)
    {
      _animationValid = _values.size() >= 1
                        && (calcMode == CalcMode::Paced || !hasKeyTimes || hasKeyPoints
                            || (_values.size() == keyTimes.size()))
                        && (calcMode == CalcMode::Discrete || keyTimes.empty() || keyTimes.back() == 1)
                        && (calcMode != CalcMode::Spline
                            || ((!_keySplines.empty() && (_keySplines.size() == _values.size() - 1))
                                || _keySplines.size() == _keyPoints.size() - 1))
                        && (!hasKeyPoints || (keyTimes.size() > 1 && keyTimes.size() == _keyPoints.size()));
      if (_animationValid)
      {
        _animationValid = SetToAtEndOfDurationValue(_values.back());
      }
      if (calcMode == CalcMode::Paced && _animationValid)
      {
        CalculateKeyTimesForCalcModePaced();
      }
    }
    else if (animationMode == AnimationMode::Path)
    {
      _animationValid = !hasKeyPoints || (keyTimes.size() > 1 && keyTimes.size() == _keyPoints.size());
    }
  }

  void SVGAnimationElement::UpdateAnimation(float percent, uint32 repeatCount) noexcept
  {
    if (!_animationValid)
    {
      return;
    }

    float effectivePercent;
    CalcMode calcMode = GetCalcMode();
    AnimationMode animationMode = GetAnimationMode();
    if (animationMode == AnimationMode::Values)
    {
      dom::DOMString from;
      dom::DOMString to;
      CurrentValuesForValuesAnimation(percent, effectivePercent, from, to);
      if (from != _lastValuesAnimationFrom || to != _lastValuesAnimationTo)
      {
        _animationValid = SetFromAndToValues(from, to);
        if (!_animationValid)
        {
          return;
        }
        _lastValuesAnimationFrom = from;
        _lastValuesAnimationTo = to;
      }
    }
    else if (!_keyPoints.empty() && (calcMode != CalcMode::Paced || animationMode == AnimationMode::Path))
    {
      effectivePercent = CalculatePercentFromKeyPoints(percent);
    }
    else if (_keyPoints.empty() && calcMode == CalcMode::Spline && KeyTimes().size() > 1)
    {
      effectivePercent = CalculatePercentForSpline(percent, CalculateKeyTimesIndex(percent));
    }
    else if (animationMode == AnimationMode::FromTo || animationMode == AnimationMode::To)
    {
      effectivePercent = CalculatePercentForFromTo(percent);
    }
    else
    {
      effectivePercent = percent;
    }

    CalculateAnimatedValue(effectivePercent, repeatCount);
  }

  void SVGAnimationElement::ResetAnimation() noexcept
  {
    _lastValuesAnimationFrom = {};
    _lastValuesAnimationTo = {};
  }
}
