#include "Krystal.Booey/SVG/Animation/SVGSMILElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/SVG/Animation/SMILTimeContainer.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"
#include "Krystal.Core/Text/StringToIntegerConversion.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace krys::boo::svg
{
  namespace
  {
    KRYS_NODISCARD SMILEventSender &SmilEventSender() noexcept
    {
      static NeverDestroyed<SMILEventSender> sender;
      return sender;
    }

    constexpr utf8_stringview IndefiniteValue = u8"indefinite";

    // This is used for duration type time values that can't be negative.
    constexpr double InvalidCachedTime = -1.;

    KRYS_NODISCARD dom::DOMStringView Trim(dom::DOMStringView string) noexcept
    {
      while (!string.empty() && text::IsUnicodeCompatibleASCIIWhitespace(string.front()))
      {
        string.remove_prefix(1);
      }
      while (!string.empty() && text::IsUnicodeCompatibleASCIIWhitespace(string.back()))
      {
        string.remove_suffix(1);
      }
      return string;
    }

    void ClearTimesWithDynamicOrigins(List<SMILTimeWithOrigin> &timeList) noexcept
    {
      std::erase_if(timeList, [](const SMILTimeWithOrigin &time) { return time.OriginIsScript(); });
    }

    void InsertSorted(List<SMILTimeWithOrigin> &list, SMILTimeWithOrigin time) noexcept
    {
      krys_debug_assert(std::is_sorted(list.begin(), list.end()));
      list.insert(std::lower_bound(list.begin(), list.end(), time), time);
    }

    KRYS_NODISCARD const dom::QualifiedName &AnyQName() noexcept
    {
      static const NeverDestroyed<dom::QualifiedName> anyName {
        dom::DOMStringAtom(u8"*"), dom::DOMStringAtom(u8"*"), dom::DOMStringAtom(u8"*")};
      return anyName;
    }
  }

  class ConditionEventListener final : public dom::EventListenerCallback
  {
  private:
    WeakPtr<SVGSMILElement> _animation;
    SVGSMILElement::Condition *_condition;

  public:
    KRYS_NODISCARD static Ref<ConditionEventListener> Create(SVGSMILElement *animation,
                                                             SVGSMILElement::Condition *condition) noexcept
    {
      return AdoptRef(*new ConditionEventListener(animation, condition));
    }

    void DisconnectAnimation() noexcept
    {
      _animation = nullptr;
    }

  private:
    ConditionEventListener(SVGSMILElement *animation, SVGSMILElement::Condition *condition) noexcept
        : _animation(CreateWeakPtr(animation)), _condition(condition)
    {
    }

    void HandleEvent(dom::Event &) noexcept final
    {
      if (RefPtr animation = ShareRefPtr(_animation.get()))
      {
        animation->AddInstanceTime(_condition->ConditionBeginOrEnd,
                                   animation->Elapsed() + _condition->Offset);
      }
    }
  };

  SVGSMILElement::Condition::Condition(Type type, BeginOrEnd beginOrEnd, dom::DOMString baseID,
                                       dom::DOMStringAtom name, SMILTime offset, int32 repeats) noexcept
      : ConditionType(type), ConditionBeginOrEnd(beginOrEnd), BaseID(krys::move(baseID)),
        Name(krys::move(name)), Offset(offset), Repeats(repeats)
  {
  }

  SVGSMILElement::SVGSMILElement(dom::Document &document, UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                                 SVGElementInterface interface) noexcept
      : SVGElement(document, krys::move(propertyRegistry), interface), _attributeName(AnyQName()),
        _cachedDur(InvalidCachedTime), _cachedRepeatDur(InvalidCachedTime),
        _cachedRepeatCount(InvalidCachedTime), _cachedMin(InvalidCachedTime), _cachedMax(InvalidCachedTime)
  {
  }

  SVGSMILElement::~SVGSMILElement() noexcept
  {
    ClearResourceReferences();
    SmilEventSender().CancelEvent(*this);
    DisconnectConditions();
    if (RefPtr timeContainer = _timeContainer; timeContainer && _targetElement && HasValidAttributeName())
    {
      timeContainer->Unschedule(this, _targetElement.get(), _attributeName);
    }
  }

  SMILTimeContainer *SVGSMILElement::TimeContainer() noexcept
  {
    return _timeContainer.get();
  }

  Maybe<dom::DOMString> SVGSMILElement::AttributeValue(const dom::QualifiedName &name) const noexcept
  {
    return GetAttributeNS(name.NamespaceURI(), name.LocalName());
  }

  void SVGSMILElement::ClearResourceReferences() noexcept
  {
    RemoveElementReference();
  }

  void SVGSMILElement::ClearTarget() noexcept
  {
    SetTargetElement(nullptr);
  }

  void SVGSMILElement::BuildPendingResource() noexcept
  {
    ClearResourceReferences();

    if (!IsConnected())
    {
      // Reset the target element if we are no longer in the document.
      SetTargetElement(nullptr);
      return;
    }

    auto id = dom::DOMStringAtom::Null();
    RefPtr<dom::Element> target;
    auto href = AttributeValue(SVGNames::HrefAttr());
    if (!href)
    {
      href = AttributeValue(XLinkNames::HrefAttr());
    }
    if (!href || href->empty())
    {
      target = ShareRefPtr(ParentElement());
    }
    else
    {
      auto result = SVGURIReference::TargetElementFromIRIString(*href, TreeScopeForSVGReferences());
      target = krys::move(result.Element);
      id = krys::move(result.Identifier);
    }
    RefPtr svgTarget =
      ShareRefPtr(target && target->IsConnected() ? DynamicDowncast<SVGElement>(target.get()) : nullptr);

    if (svgTarget.get() != TargetElement())
    {
      SetTargetElement(svgTarget.get());
    }

    if (!svgTarget)
    {
      // Do not register as pending if we are already pending this resource.
      if (TreeScopeForSVGReferences().IsPendingSVGResource(*this, id))
      {
        return;
      }

      if (!id.View().empty())
      {
        TreeScopeForSVGReferences().AddPendingSVGResource(id, *this);
        krys_debug_assert(HasPendingResources());
      }
    }
    else
    {
      svgTarget->AddReferencingElement(*this);
    }
  }

  bool SVGSMILElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                          dom::DOMStringAtom namespaceURI) const noexcept
  {
    // https://svgwg.org/svg2-draft/styling.html#PresentationAttributes
    if (localName == SVGNames::FillAttr().LocalName() && namespaceURI == dom::DOMStringAtom::Null())
    {
      return false;
    }
    return StyledElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  dom::QualifiedName SVGSMILElement::ConstructAttributeName() const noexcept
  {
    auto value = AttributeValue(SVGNames::AttributeNameAttr()).value_or(dom::DOMString {});
    dom::DOMStringView qualifiedName = value;

    // FIXME: Document::parseQualifiedName also checks the name's characters are XML name characters.
    auto colon = qualifiedName.find(u8':');
    dom::DOMStringView prefix;
    dom::DOMStringView localName = qualifiedName;
    if (colon != dom::DOMStringView::npos)
    {
      prefix = qualifiedName.substr(0, colon);
      localName = qualifiedName.substr(colon + 1);
      if (!dom::NameValidation::IsValidNamespacePrefix(prefix))
      {
        return AnyQName();
      }
    }
    if (localName.empty() || !dom::NameValidation::IsValidAttributeLocalName(localName))
    {
      return AnyQName();
    }

    if (prefix.empty())
    {
      return {dom::DOMStringAtom::Null(), dom::DOMStringAtom::Null(), dom::DOMStringAtom(localName)};
    }

    auto namespaceURI = LookupNamespaceURI(dom::DOMStringAtom(prefix));
    if (namespaceURI.View().empty())
    {
      return AnyQName();
    }

    return {namespaceURI, dom::DOMStringAtom::Null(), dom::DOMStringAtom(localName)};
  }

  void SVGSMILElement::UpdateAttributeName() noexcept
  {
    SetAttributeName(ConstructAttributeName());
  }

  void SVGSMILElement::Reset() noexcept
  {
    StopAnimation(TargetElement());

    _activeState = Inactive;
    _isWaitingForFirstInterval = true;
    _intervalBegin = SMILTime::Unresolved();
    _intervalEnd = SMILTime::Unresolved();
    _previousIntervalBegin = SMILTime::Unresolved();
    _lastPercent = 0;
    _lastRepeat = 0;
    _nextProgressTime = 0;
    ResolveFirstInterval();
  }

  void SVGSMILElement::OnInsert() noexcept
  {
    SVGElement::OnInsert();
    if (!IsConnected())
    {
      return;
    }

    // Verify we are not in <use> instance tree.
    krys_debug_assert(
      !IsInShadowTree()
      || !Is<SVGUseElement>(DynamicDowncast<SVGElement>(dom::TreeQueries::ShadowHost(*this))));

    UpdateAttributeName();

    RefPtr owner = ShareRefPtr(OwnerSVGElement());
    if (!owner)
    {
      return;
    }

    _timeContainer = ShareRefPtr(&owner->TimeContainer());
    _timeContainer->SetDocumentOrderIndexesDirty();

    // "If no attribute is present, the default begin value (an offset-value of 0) must be evaluated."
    if (!AttributeValue(SVGNames::BeginAttr()))
    {
      _beginTimes.emplace_back();
    }

    if (_isWaitingForFirstInterval)
    {
      ResolveFirstInterval();
    }

    if (RefPtr timeContainer = _timeContainer)
    {
      timeContainer->NotifyIntervalsChanged();
    }
  }

  void SVGSMILElement::OnPostConnection() noexcept
  {
    SVGElement::OnPostConnection();
    BuildPendingResource();
  }

  void SVGSMILElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    if (oldAncestor.IsConnected() && !IsConnected())
    {
      ClearResourceReferences();
      DisconnectConditions();
      SetTargetElement(nullptr);
      SetAttributeName(AnyQName());
      AnimationAttributeChanged();
      _timeContainer = nullptr;
    }

    SVGElement::OnRemove(isSubtreeRoot, oldAncestor);
  }

  bool SVGSMILElement::HasValidAttributeName() const noexcept
  {
    return AttributeName() != AnyQName();
  }

  SMILTime SVGSMILElement::ParseOffsetValue(dom::DOMStringView data) noexcept
  {
    Maybe<double> result;
    auto parse = Trim(data);
    if (parse.ends_with(u8'h'))
    {
      result =
        ToDouble(parse.substr(0, parse.size() - 1)).transform([](double value) { return value * 60 * 60; });
    }
    else if (parse.ends_with(u8"min"))
    {
      result = ToDouble(parse.substr(0, parse.size() - 3)).transform([](double value) { return value * 60; });
    }
    else if (parse.ends_with(u8"ms"))
    {
      result =
        ToDouble(parse.substr(0, parse.size() - 2)).transform([](double value) { return value / 1'000; });
    }
    else if (parse.ends_with(u8's'))
    {
      result = ToDouble(parse.substr(0, parse.size() - 1));
    }
    else
    {
      result = ToDouble(parse);
    }
    if (!result || std::isnan(*result) || !SMILTime(*result).IsFinite())
    {
      return SMILTime::Unresolved();
    }
    return *result;
  }

  SMILTime SVGSMILElement::ParseClockValue(Maybe<dom::DOMStringView> data) noexcept
  {
    if (!data)
    {
      return SMILTime::Unresolved();
    }

    auto parse = Trim(*data);
    if (parse == IndefiniteValue)
    {
      return SMILTime::Indefinite();
    }

    Maybe<double> seconds;
    double result = 0;
    auto doublePointOne = parse.find(u8':');
    auto doublePointTwo =
      doublePointOne == dom::DOMStringView::npos ? doublePointOne : parse.find(u8':', doublePointOne + 1);
    if (doublePointOne == 2 && doublePointTwo == 5 && parse.size() >= 8)
    {
      auto hour = text::ParseInteger<uint8>(parse.substr(0, 2));
      auto minute = text::ParseInteger<uint8>(parse.substr(3, 2));
      if (!hour || !minute)
      {
        return SMILTime::Unresolved();
      }
      seconds = ToDouble(parse.substr(6));
      result = *hour * 60 * 60 + *minute * 60 + seconds.value_or(0);
    }
    else if (doublePointOne == 2 && doublePointTwo == dom::DOMStringView::npos && parse.size() >= 5)
    {
      auto minute = text::ParseInteger<uint8>(parse.substr(0, 2));
      if (!minute)
      {
        return SMILTime::Unresolved();
      }
      seconds = ToDouble(parse.substr(3));
      result = *minute * 60 + seconds.value_or(0);
    }
    else
    {
      return ParseOffsetValue(parse);
    }
    if (!seconds || std::isnan(result) || !SMILTime(result).IsFinite())
    {
      return SMILTime::Unresolved();
    }
    return result;
  }

  bool SVGSMILElement::ParseCondition(dom::DOMStringView value, BeginOrEnd beginOrEnd) noexcept
  {
    auto parseString = Trim(value);

    double sign = 1.;
    auto pos = parseString.find(u8'+');
    if (pos == dom::DOMStringView::npos)
    {
      pos = parseString.find(u8'-');
      if (pos != dom::DOMStringView::npos)
      {
        sign = -1.;
      }
    }
    dom::DOMStringView conditionString;
    SMILTime offset = 0;
    if (pos == dom::DOMStringView::npos)
    {
      conditionString = parseString;
    }
    else
    {
      conditionString = Trim(parseString.substr(0, pos));
      auto offsetString = Trim(parseString.substr(pos + 1));
      offset = ParseOffsetValue(offsetString);
      if (offset.IsUnresolved())
      {
        return false;
      }
      offset = offset * sign;
    }
    if (conditionString.empty())
    {
      return false;
    }
    pos = conditionString.find(u8'.');

    dom::DOMStringView baseID;
    dom::DOMStringView nameView;
    if (pos == dom::DOMStringView::npos)
    {
      nameView = conditionString;
    }
    else
    {
      baseID = conditionString.substr(0, pos);
      nameView = conditionString.substr(pos + 1);
    }
    if (nameView.empty())
    {
      return false;
    }

    auto nameString = dom::DOMStringAtom::Null();
    Condition::Type type;
    int32 repeats = -1;
    if (nameView.starts_with(u8"repeat(") && nameView.ends_with(u8')'))
    {
      // FIXME: For repeat events we just need to add the data carrying TimeEvent class and fire the events at
      // appropiate times.
      auto parsedRepeat = text::ParseInteger<uint32>(nameView.substr(7, nameView.size() - 8));
      if (!parsedRepeat)
      {
        return false;
      }
      // FIXME: By assigning an unsigned to a signed, this can turn large integers into negative numbers.
      repeats = static_cast<int32>(*parsedRepeat);
      nameString = dom::DOMStringAtom(u8"repeat");
      type = Condition::EventBase;
    }
    else if (nameView == u8"begin" || nameView == u8"end")
    {
      if (baseID.empty())
      {
        return false;
      }
      type = Condition::Syncbase;
      nameString = dom::DOMStringAtom(nameView);
    }
    else if (nameView.starts_with(u8"accesskey("))
    {
      // FIXME: accesskey() support.
      type = Condition::AccessKey;
      nameString = dom::DOMStringAtom(nameView);
    }
    else
    {
      type = Condition::EventBase;
      nameString = dom::DOMStringAtom(nameView);
    }

    _conditions.emplace_back(type, beginOrEnd, dom::DOMString(baseID), krys::move(nameString), offset,
                             repeats);

    if (type == Condition::EventBase && beginOrEnd == End)
    {
      _hasEndEventConditions = true;
    }

    return true;
  }

  void SVGSMILElement::ParseBeginOrEnd(dom::DOMStringView parseString, BeginOrEnd beginOrEnd) noexcept
  {
    auto &timeList = beginOrEnd == Begin ? _beginTimes : _endTimes;
    if (beginOrEnd == End)
    {
      _hasEndEventConditions = false;
    }
    List<double> existing;
    for (const auto &time : timeList)
    {
      existing.push_back(time.Time().Value());
    }
    while (true)
    {
      auto separator = parseString.find(u8';');
      auto string = parseString.substr(0, separator);
      if (!string.empty())
      {
        SMILTime value = ParseClockValue(string);
        if (value.IsUnresolved())
        {
          (void)ParseCondition(string, beginOrEnd);
        }
        else if (std::ranges::find(existing, value.Value()) == existing.end())
        {
          timeList.emplace_back(value, SMILTimeWithOrigin::Origin::Parser);
        }
      }
      if (separator == dom::DOMStringView::npos)
      {
        break;
      }
      parseString.remove_prefix(separator + 1);
    }
    std::sort(timeList.begin(), timeList.end());
  }

  bool SVGSMILElement::IsSupportedAttribute(const dom::QualifiedName &attrName) noexcept
  {
    static const Array supportedAttributes {
      &SVGNames::BeginAttr(),     &SVGNames::EndAttr(),           &SVGNames::DurAttr(),
      &SVGNames::RepeatDurAttr(), &SVGNames::RepeatCountAttr(),   &SVGNames::MinAttr(),
      &SVGNames::MaxAttr(),       &SVGNames::AttributeNameAttr(), &SVGNames::HrefAttr(),
      &XLinkNames::HrefAttr(),
    };
    return std::ranges::any_of(supportedAttributes,
                               [&](const auto *name) { return name->Matches(attrName); });
  }

  void SVGSMILElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    SVGElement::AttributeChanged(name, oldValue, value);

    if (name == SVGNames::BeginAttr())
    {
      if (!_conditions.empty())
      {
        DisconnectConditions();
        _conditions.clear();
        ParseBeginOrEnd(AttributeValue(SVGNames::EndAttr()).value_or(dom::DOMString {}), End);
      }
      ParseBeginOrEnd(value.value_or(dom::DOMStringView {}), Begin);
      if (IsConnected())
      {
        ConnectConditions();
      }
    }
    else if (name == SVGNames::EndAttr())
    {
      if (!_conditions.empty())
      {
        DisconnectConditions();
        _conditions.clear();
        ParseBeginOrEnd(AttributeValue(SVGNames::BeginAttr()).value_or(dom::DOMString {}), Begin);
      }
      ParseBeginOrEnd(value.value_or(dom::DOMStringView {}), End);
      if (IsConnected())
      {
        ConnectConditions();
      }
    }
    // NOTE: onbegin, onend and onrepeat need scripting.
  }

  void SVGSMILElement::SVGAttributeChanged(const dom::QualifiedName &attrName) noexcept
  {
    if (!IsSupportedAttribute(attrName))
    {
      SVGElement::SVGAttributeChanged(attrName);
      return;
    }

    if (attrName == SVGNames::DurAttr())
    {
      _cachedDur = InvalidCachedTime;
    }
    else if (attrName == SVGNames::RepeatDurAttr())
    {
      _cachedRepeatDur = InvalidCachedTime;
    }
    else if (attrName == SVGNames::RepeatCountAttr())
    {
      _cachedRepeatCount = InvalidCachedTime;
    }
    else if (attrName == SVGNames::MinAttr())
    {
      _cachedMin = InvalidCachedTime;
    }
    else if (attrName == SVGNames::MaxAttr())
    {
      _cachedMax = InvalidCachedTime;
    }
    else if (attrName == SVGNames::AttributeNameAttr())
    {
      UpdateAttributeName();
    }
    else if (attrName.Matches(SVGNames::HrefAttr()) || attrName.Matches(XLinkNames::HrefAttr()))
    {
      InstanceInvalidationGuard guard(*this);
      BuildPendingResource();
    }
    else if (attrName == SVGNames::BeginAttr())
    {
      if (IsConnected())
      {
        BeginListChanged(Elapsed());
      }
    }
    else if (attrName == SVGNames::EndAttr())
    {
      if (IsConnected())
      {
        EndListChanged(Elapsed());
      }
    }
    AnimationAttributeChanged();
  }

  RefPtr<dom::Element> SVGSMILElement::EventBaseFor(const Condition &condition) noexcept
  {
    if (condition.BaseID.empty())
    {
      return ShareRefPtr(TargetElement());
    }
    return dom::mixins::NonElementParentNode::GetElementById(TreeScope().RootNode(), condition.BaseID);
  }

  void SVGSMILElement::ConnectConditions() noexcept
  {
    if (_conditionsConnected)
    {
      DisconnectConditions();
    }
    _conditionsConnected = true;
    for (auto &condition : _conditions)
    {
      if (condition.ConditionType == Condition::EventBase)
      {
        krys_debug_assert(!condition.SyncBase);
        RefPtr eventBase = EventBaseFor(condition);
        if (!eventBase)
        {
          continue;
        }
        krys_debug_assert(!condition.EventListener);
        condition.EventListener = ConditionEventListener::Create(this, &condition);
        eventBase->AddEventListener(
          condition.Name, ShareRef(static_cast<dom::EventListenerCallback &>(*condition.EventListener)),
          false);
      }
      else if (condition.ConditionType == Condition::Syncbase)
      {
        krys_debug_assert(!condition.BaseID.empty());
        condition.SyncBase =
          dom::mixins::NonElementParentNode::GetElementById(TreeScope().RootNode(), condition.BaseID);
        if (!condition.SyncBase)
        {
          continue;
        }
        auto *svgSMILElement = DynamicDowncast<SVGSMILElement>(*condition.SyncBase);
        if (!svgSMILElement)
        {
          condition.SyncBase = nullptr;
          continue;
        }
        svgSMILElement->AddTimeDependent(this);
      }
    }
  }

  void SVGSMILElement::DisconnectConditions() noexcept
  {
    if (!_conditionsConnected)
    {
      return;
    }
    _conditionsConnected = false;
    for (auto &condition : _conditions)
    {
      if (condition.ConditionType == Condition::EventBase)
      {
        krys_debug_assert(!condition.SyncBase);
        if (!condition.EventListener)
        {
          continue;
        }
        // Note: It's a memory optimization to try to remove our condition
        // event listener, but it's not guaranteed to work, since we have
        // no guarantee that eventBaseFor() will be able to find our condition's
        // original eventBase. So, we also have to disconnect ourselves from
        // our condition event listener, in case it later fires.
        if (RefPtr eventBase = EventBaseFor(condition))
        {
          eventBase->RemoveEventListener(
            condition.Name, ShareRef(static_cast<dom::EventListenerCallback &>(*condition.EventListener)),
            false);
        }
        condition.EventListener->DisconnectAnimation();
        condition.EventListener = nullptr;
      }
      else if (condition.ConditionType == Condition::Syncbase)
      {
        if (condition.SyncBase)
        {
          Downcast<SVGSMILElement>(*condition.SyncBase).RemoveTimeDependent(this);
        }
      }
      condition.SyncBase = nullptr;
    }
  }

  void SVGSMILElement::SetAttributeName(const dom::QualifiedName &attributeName) noexcept
  {
    if (RefPtr timeContainer = _timeContainer;
        timeContainer && _targetElement && _attributeName != attributeName)
    {
      if (HasValidAttributeName())
      {
        timeContainer->Unschedule(this, _targetElement.get(), _attributeName);
      }
      _attributeName = attributeName;
      if (HasValidAttributeName())
      {
        timeContainer->Schedule(this, _targetElement.get(), _attributeName);
      }
    }
    else
    {
      _attributeName = attributeName;
    }

    // Only clear the animated type, if we had a target before.
    if (RefPtr targetElement = ShareRefPtr(_targetElement.get()))
    {
      StopAnimation(targetElement.get());
    }
  }

  void SVGSMILElement::SetTargetElement(SVGElement *target) noexcept
  {
    if (RefPtr timeContainer = _timeContainer; timeContainer && HasValidAttributeName())
    {
      if (RefPtr targetElement = ShareRefPtr(_targetElement.get()))
      {
        timeContainer->Unschedule(this, targetElement.get(), _attributeName);
      }
      if (target)
      {
        timeContainer->Schedule(this, target, _attributeName);
      }
    }

    if (RefPtr targetElement = ShareRefPtr(_targetElement.get()))
    {
      // Clear values that may depend on the previous target.
      StopAnimation(targetElement.get());
      DisconnectConditions();
    }

    // If the animation state is not Inactive, always reset to a clear state before leaving the old target
    // element.
    if (_activeState != Inactive)
    {
      EndedActiveInterval();
    }

    _targetElement = CreateWeakPtr(target);
  }

  SMILTime SVGSMILElement::Elapsed() const noexcept
  {
    return _timeContainer ? _timeContainer->Elapsed() : 0;
  }

  bool SVGSMILElement::IsFrozen() const noexcept
  {
    return _activeState == Frozen;
  }

  SVGSMILElement::Restart SVGSMILElement::GetRestart() const noexcept
  {
    auto value = AttributeValue(SVGNames::RestartAttr());
    if (value == u8"never")
    {
      return RestartNever;
    }
    if (value == u8"whenNotActive")
    {
      return RestartWhenNotActive;
    }
    return RestartAlways;
  }

  SVGSMILElement::FillMode SVGSMILElement::Fill() const noexcept
  {
    auto value = AttributeValue(SVGNames::FillAttr());
    return value == u8"freeze" ? FillFreeze : FillRemove;
  }

  SMILTime SVGSMILElement::Dur() const noexcept
  {
    if (_cachedDur != InvalidCachedTime)
    {
      return _cachedDur;
    }
    auto value = AttributeValue(SVGNames::DurAttr());
    SMILTime clockValue = ParseClockValue(value);
    return _cachedDur = clockValue <= 0 ? SMILTime::Unresolved() : clockValue;
  }

  SMILTime SVGSMILElement::RepeatDur() const noexcept
  {
    if (_cachedRepeatDur != InvalidCachedTime)
    {
      return _cachedRepeatDur;
    }
    auto value = AttributeValue(SVGNames::RepeatDurAttr());
    SMILTime clockValue = ParseClockValue(value);
    _cachedRepeatDur = clockValue <= 0 ? SMILTime::Unresolved() : clockValue;
    return _cachedRepeatDur;
  }

  // So a count is not really a time but let just all pretend we did not notice.
  SMILTime SVGSMILElement::RepeatCount() const noexcept
  {
    if (_cachedRepeatCount != InvalidCachedTime)
    {
      return _cachedRepeatCount;
    }
    auto value = AttributeValue(SVGNames::RepeatCountAttr());
    if (!value)
    {
      return SMILTime::Unresolved();
    }

    if (*value == IndefiniteValue)
    {
      return SMILTime::Indefinite();
    }
    auto result = ToDouble(*value);
    return _cachedRepeatCount =
             (result && *result > 0 && std::isfinite(*result)) ? SMILTime(*result) : SMILTime::Unresolved();
  }

  SMILTime SVGSMILElement::MaxValue() const noexcept
  {
    if (_cachedMax != InvalidCachedTime)
    {
      return _cachedMax;
    }
    auto value = AttributeValue(SVGNames::MaxAttr());
    SMILTime result = ParseClockValue(value);
    return _cachedMax = (result.IsUnresolved() || result <= 0) ? SMILTime::Indefinite() : result;
  }

  SMILTime SVGSMILElement::MinValue() const noexcept
  {
    if (_cachedMin != InvalidCachedTime)
    {
      return _cachedMin;
    }
    auto value = AttributeValue(SVGNames::MinAttr());
    SMILTime result = ParseClockValue(value);
    return _cachedMin = (result.IsUnresolved() || result < 0) ? SMILTime(0) : result;
  }

  SMILTime SVGSMILElement::SimpleDuration() const noexcept
  {
    return std::min(Dur(), SMILTime::Indefinite());
  }

  void SVGSMILElement::AddInstanceTime(BeginOrEnd beginOrEnd, SMILTime time,
                                       SMILTimeWithOrigin::Origin origin) noexcept
  {
    SMILTime elapsed = Elapsed();
    if (elapsed.IsUnresolved())
    {
      return;
    }
    InsertSorted(beginOrEnd == Begin ? _beginTimes : _endTimes, SMILTimeWithOrigin(time, origin));
    if (beginOrEnd == Begin)
    {
      BeginListChanged(elapsed);
    }
    else
    {
      EndListChanged(elapsed);
    }
  }

  SMILTime SVGSMILElement::FindInstanceTime(BeginOrEnd beginOrEnd, SMILTime minimumTime,
                                            bool equalsMinimumOK) const noexcept
  {
    const auto &list = beginOrEnd == Begin ? _beginTimes : _endTimes;
    auto sizeOfList = static_cast<int32>(list.size());

    if (!sizeOfList)
    {
      return beginOrEnd == Begin ? SMILTime::Unresolved() : SMILTime::Indefinite();
    }

    // Find an item close to minimumTime, as approximateBinarySearch does: the first item that isn't smaller,
    // or the last item.
    auto position = std::lower_bound(list.begin(), list.end(), minimumTime,
                                     [](const SMILTimeWithOrigin &item, const SMILTime &time)
                                     { return item.Time() < time; });
    int32 indexOfResult = std::min(static_cast<int32>(position - list.begin()), sizeOfList - 1);

    if (list[indexOfResult].Time() < minimumTime && indexOfResult < sizeOfList - 1)
    {
      ++indexOfResult;
    }

    const SMILTime &currentTime = list[indexOfResult].Time();

    // The special value "indefinite" does not yield an instance time in the begin list.
    if (currentTime.IsIndefinite() && beginOrEnd == Begin)
    {
      return SMILTime::Unresolved();
    }

    if (currentTime < minimumTime)
    {
      return SMILTime::Unresolved();
    }
    if (currentTime > minimumTime)
    {
      return currentTime;
    }

    krys_debug_assert(currentTime == minimumTime);
    if (equalsMinimumOK)
    {
      return currentTime;
    }

    // If the equals is not accepted, return the next bigger item in the list.
    SMILTime nextTime = currentTime;
    while (indexOfResult < sizeOfList - 1)
    {
      nextTime = list[indexOfResult + 1].Time();
      if (nextTime > minimumTime)
      {
        return nextTime;
      }
      ++indexOfResult;
    }

    return beginOrEnd == Begin ? SMILTime::Unresolved() : SMILTime::Indefinite();
  }

  SMILTime SVGSMILElement::RepeatingDuration() const noexcept
  {
    // Computing the active duration
    // http://www.w3.org/TR/SMIL2/smil-timing.html#Timing-ComputingActiveDur
    SMILTime repeatCount = RepeatCount();
    SMILTime repeatDur = RepeatDur();
    SMILTime simpleDuration = SimpleDuration();
    if (!simpleDuration || (repeatDur.IsUnresolved() && repeatCount.IsUnresolved()))
    {
      return simpleDuration;
    }
    SMILTime repeatCountDuration = simpleDuration * repeatCount;
    return std::min(repeatCountDuration, std::min(repeatDur, SMILTime::Indefinite()));
  }

  SMILTime SVGSMILElement::ResolveActiveEnd(SMILTime resolvedBegin, SMILTime resolvedEnd) const noexcept
  {
    // Computing the active duration
    // http://www.w3.org/TR/SMIL2/smil-timing.html#Timing-ComputingActiveDur
    SMILTime preliminaryActiveDuration;
    if (!resolvedEnd.IsUnresolved() && Dur().IsUnresolved() && RepeatDur().IsUnresolved()
        && RepeatCount().IsUnresolved())
    {
      preliminaryActiveDuration = resolvedEnd - resolvedBegin;
    }
    else if (!resolvedEnd.IsFinite())
    {
      preliminaryActiveDuration = RepeatingDuration();
    }
    else
    {
      preliminaryActiveDuration = std::min(RepeatingDuration(), resolvedEnd - resolvedBegin);
    }

    SMILTime minValue = MinValue();
    SMILTime maxValue = MaxValue();
    if (minValue > maxValue)
    {
      // Ignore both.
      // http://www.w3.org/TR/2001/REC-smil-animation-20010904/#MinMax
      minValue = 0;
      maxValue = SMILTime::Indefinite();
    }
    return resolvedBegin + std::min(maxValue, std::max(minValue, preliminaryActiveDuration));
  }

  void SVGSMILElement::ResolveInterval(bool first, SMILTime &beginResult, SMILTime &endResult) const noexcept
  {
    // See the pseudocode in http://www.w3.org/TR/SMIL3/smil-timing.html#q90.
    SMILTime beginAfter = first ? -std::numeric_limits<double>::infinity() : _intervalEnd;
    SMILTime lastIntervalTempEnd = std::numeric_limits<double>::infinity();
    while (true)
    {
      bool equalsMinimumOK = !first || _intervalEnd > _intervalBegin;
      SMILTime tempBegin = FindInstanceTime(Begin, beginAfter, equalsMinimumOK);
      if (tempBegin.IsUnresolved())
      {
        break;
      }
      SMILTime tempEnd = FindInstanceTime(End, tempBegin, true);
      if (!_endTimes.empty())
      {
        if ((first && tempBegin == tempEnd && tempEnd == lastIntervalTempEnd)
            || (!first && tempEnd == _intervalEnd))
        {
          tempEnd = FindInstanceTime(End, tempBegin, false);
        }
        if (tempEnd.IsUnresolved() && !_hasEndEventConditions)
        {
          break;
        }
      }
      tempEnd = ResolveActiveEnd(tempBegin, tempEnd);
      if (!first || tempEnd > SMILTime() || (!tempBegin.Value() && !tempEnd.Value()))
      {
        beginResult = tempBegin;
        endResult = tempEnd;
        return;
      }

      beginAfter = tempEnd;
      lastIntervalTempEnd = tempEnd;
    }
    beginResult = SMILTime::Unresolved();
    endResult = SMILTime::Unresolved();
  }

  void SVGSMILElement::ResolveFirstInterval() noexcept
  {
    SMILTime begin;
    SMILTime end;
    ResolveInterval(true, begin, end);
    krys_debug_assert(!begin.IsIndefinite());

    // Compare raw SMILTime values to avoids treating open-ended intervals as "new" on every call.
    if (!begin.IsUnresolved()
        && (begin.Value() != _intervalBegin.Value() || end.Value() != _intervalEnd.Value()))
    {
      _intervalBegin = begin;
      _intervalEnd = end;
      NotifyDependentsIntervalChanged();
      _nextProgressTime = std::min(_nextProgressTime, _intervalBegin);

      if (RefPtr timeContainer = _timeContainer)
      {
        timeContainer->NotifyIntervalsChanged();
      }
    }
  }

  bool SVGSMILElement::ResolveNextInterval() noexcept
  {
    SMILTime begin;
    SMILTime end;
    ResolveInterval(false, begin, end);
    krys_debug_assert(!begin.IsIndefinite());

    if (!begin.IsUnresolved() && begin != _intervalBegin)
    {
      _intervalBegin = begin;
      _intervalEnd = end;
      NotifyDependentsIntervalChanged();
      _nextProgressTime = std::min(_nextProgressTime, _intervalBegin);
      return true;
    }
    return false;
  }

  SMILTime SVGSMILElement::NextProgressTime() const noexcept
  {
    return _nextProgressTime;
  }

  void SVGSMILElement::BeginListChanged(SMILTime eventTime) noexcept
  {
    if (_isWaitingForFirstInterval)
    {
      ResolveFirstInterval();
    }
    else
    {
      if (GetRestart() == RestartNever)
      {
        return;
      }

      SMILTime newBegin = FindInstanceTime(Begin, eventTime, true);
      if (newBegin.IsFinite() && (_intervalEnd <= eventTime || newBegin < _intervalBegin))
      {
        // Begin time changed, re-resolve the interval.
        SMILTime oldBegin = _intervalBegin;
        _intervalEnd = eventTime;
        ResolveInterval(false, _intervalBegin, _intervalEnd);
        krys_debug_assert(!_intervalBegin.IsUnresolved());
        if (_intervalBegin != oldBegin)
        {
          if (_activeState == Active && _intervalBegin > eventTime)
          {
            _activeState = DetermineActiveState(eventTime);
            if (_activeState != Active)
            {
              EndedActiveInterval();
            }
          }
          NotifyDependentsIntervalChanged();
        }
      }
    }
    _nextProgressTime = Elapsed();

    if (RefPtr timeContainer = _timeContainer)
    {
      timeContainer->NotifyIntervalsChanged();
    }
  }

  void SVGSMILElement::EndListChanged(SMILTime) noexcept
  {
    SMILTime elapsed = Elapsed();
    if (_isWaitingForFirstInterval)
    {
      ResolveFirstInterval();
    }
    else if (elapsed < _intervalEnd && _intervalBegin.IsFinite())
    {
      SMILTime newEnd = FindInstanceTime(End, _intervalBegin, false);
      if (newEnd < _intervalEnd)
      {
        newEnd = ResolveActiveEnd(_intervalBegin, newEnd);
        if (newEnd != _intervalEnd)
        {
          _intervalEnd = newEnd;
          NotifyDependentsIntervalChanged();
        }
      }
    }
    _nextProgressTime = elapsed;

    if (RefPtr timeContainer = _timeContainer)
    {
      timeContainer->NotifyIntervalsChanged();
    }
  }

  void SVGSMILElement::CheckRestart(SMILTime elapsed) noexcept
  {
    krys_debug_assert(!_isWaitingForFirstInterval);
    krys_debug_assert(elapsed >= _intervalBegin);

    Restart restart = GetRestart();
    if (restart == RestartNever)
    {
      return;
    }

    if (elapsed < _intervalEnd)
    {
      if (restart != RestartAlways)
      {
        return;
      }
      SMILTime nextBegin = FindInstanceTime(Begin, _intervalBegin, false);
      if (nextBegin < _intervalEnd)
      {
        _intervalEnd = nextBegin;
        NotifyDependentsIntervalChanged();
      }
    }

    if (elapsed >= _intervalEnd)
    {
      (void)ResolveNextInterval();
    }
  }

  void SVGSMILElement::SeekToIntervalCorrespondingToTime(SMILTime elapsed) noexcept
  {
    krys_debug_assert(!_isWaitingForFirstInterval);
    krys_debug_assert(elapsed >= _intervalBegin);

    // Manually seek from interval to interval, just as if the animation would run regulary.
    while (true)
    {
      // Figure out the next value in the begin time list after the current interval begin.
      SMILTime nextBegin = FindInstanceTime(Begin, _intervalBegin, false);

      // If the 'nextBegin' time is unresolved (eg. just one defined interval), we're done seeking.
      if (nextBegin.IsUnresolved())
      {
        return;
      }

      // If the 'nextBegin' time is larger than or equal to the current interval end time, we're done seeking.
      // If the 'elapsed' time is smaller than the next begin interval time, we're done seeking.
      if (nextBegin < _intervalEnd && elapsed >= nextBegin)
      {
        // End current interval, and start a new interval from the 'nextBegin' time.
        _intervalEnd = nextBegin;
        if (!ResolveNextInterval())
        {
          break;
        }
        continue;
      }

      // If the desired 'elapsed' time is past the current interval, advance to the next.
      if (elapsed >= _intervalEnd)
      {
        if (!ResolveNextInterval())
        {
          break;
        }
        continue;
      }

      return;
    }
  }

  float SVGSMILElement::CalculateAnimationPercentAndRepeat(SMILTime elapsed, uint32 &repeat) const noexcept
  {
    SMILTime simpleDuration = SimpleDuration();
    repeat = 0;
    if (simpleDuration.IsIndefinite())
    {
      repeat = 0;
      return 0.f;
    }
    if (!simpleDuration)
    {
      repeat = 0;
      return 1.f;
    }
    krys_debug_assert(_intervalBegin.IsFinite());
    krys_debug_assert(simpleDuration.IsFinite());
    SMILTime activeTime = elapsed - _intervalBegin;
    SMILTime repeatingDuration = RepeatingDuration();

    if ((elapsed >= _intervalEnd && !repeatingDuration.IsIndefinite()) || activeTime > repeatingDuration)
    {
      repeat = static_cast<uint32>(repeatingDuration.Value() / simpleDuration.Value());
      if (!std::fmod(repeatingDuration.Value(), simpleDuration.Value()))
      {
        --repeat;
      }
    }
    else
    {
      repeat = static_cast<uint32>(activeTime.Value() / simpleDuration.Value());
    }

    double percent;
    if (elapsed >= _intervalEnd || activeTime > repeatingDuration)
    {
      double lastActiveDuration =
        elapsed >= _intervalEnd ? _intervalEnd.Value() - _intervalBegin.Value() : repeatingDuration.Value();
      percent = lastActiveDuration / simpleDuration.Value();
      percent = percent - std::floor(percent);
      if (percent < std::numeric_limits<float>::epsilon()
          || 1 - percent < std::numeric_limits<float>::epsilon())
      {
        percent = 1.0f;
      }
    }
    else
    {
      SMILTime simpleTime = std::fmod(activeTime.Value(), simpleDuration.Value());
      percent = simpleTime.Value() / simpleDuration.Value();
    }

    return static_cast<float>(percent);
  }

  SMILTime SVGSMILElement::CalculateNextProgressTime(SMILTime elapsed) const noexcept
  {
    if (_timeContainer && _activeState == Active)
    {
      // If duration is indefinite the value does not actually change over time. Same is true for <set>.
      SMILTime simpleDuration = SimpleDuration();
      if (simpleDuration.IsIndefinite() || Interface() == SVGElementInterface::Set)
      {
        SMILTime repeatingDurationEnd = _intervalBegin + RepeatingDuration();
        // We are supposed to do freeze semantics when repeating ends, even if the element is still active.
        // Take care that we get a timer callback at that point.
        if (elapsed < repeatingDurationEnd && repeatingDurationEnd < _intervalEnd
            && repeatingDurationEnd.IsFinite())
        {
          return repeatingDurationEnd;
        }
        return _intervalEnd;
      }
      return elapsed + _timeContainer->AnimationFrameDelay().count();
    }
    return _intervalBegin >= elapsed ? _intervalBegin : SMILTime::Unresolved();
  }

  SVGSMILElement::ActiveState SVGSMILElement::DetermineActiveState(SMILTime elapsed) const noexcept
  {
    if (elapsed >= _intervalBegin && elapsed < _intervalEnd)
    {
      return Active;
    }

    return Fill() == FillFreeze ? Frozen : Inactive;
  }

  bool SVGSMILElement::IsContributing(SMILTime elapsed) const noexcept
  {
    // Animation does not contribute during the active time if it is past its repeating duration and has
    // fill=remove.
    return (_activeState == Active
            && (Fill() == FillFreeze || elapsed <= _intervalBegin + RepeatingDuration()))
           || _activeState == Frozen;
  }

  bool SVGSMILElement::Progress(SMILTime elapsed, SVGSMILElement &firstAnimation, bool seekToTime) noexcept
  {
    krys_debug_assert(_timeContainer);
    krys_debug_assert(_isWaitingForFirstInterval || _intervalBegin.IsFinite());

    if (!_conditionsConnected)
    {
      ConnectConditions();
    }

    if (!_intervalBegin.IsFinite())
    {
      krys_debug_assert(_activeState == Inactive);
      _nextProgressTime = SMILTime::Unresolved();
      return false;
    }

    if (elapsed < _intervalBegin)
    {
      krys_debug_assert(_activeState != Active);
      bool isFrozen = (_activeState == Frozen);
      if (isFrozen)
      {
        if (this == &firstAnimation)
        {
          StartAnimation();
        }
        UpdateAnimation(_lastPercent, _lastRepeat);
      }
      _nextProgressTime = _intervalBegin;
      // If the animation is frozen, it's still contributing.
      return isFrozen;
    }

    _previousIntervalBegin = _intervalBegin;

    if (_isWaitingForFirstInterval)
    {
      _isWaitingForFirstInterval = false;
      ResolveFirstInterval();
    }

    // This call may obtain a new interval -- never call calculateAnimationPercentAndRepeat() before!
    if (seekToTime)
    {
      SeekToIntervalCorrespondingToTime(elapsed);
      if (elapsed < _intervalBegin)
      {
        // elapsed is not within an interval.
        _nextProgressTime = _intervalBegin;
        return false;
      }
    }

    uint32 repeat = 0;
    float percent = CalculateAnimationPercentAndRepeat(elapsed, repeat);
    CheckRestart(elapsed);

    ActiveState oldActiveState = _activeState;
    _activeState = DetermineActiveState(elapsed);
    bool animationIsContributing = IsContributing(elapsed);

    auto beginEvent = dom::DOMStringAtom(EventNames::BeginEvent);
    auto endEvent = dom::DOMStringAtom(EventNames::EndEvent);
    auto repeatEvent = dom::DOMStringAtom(EventNames::RepeatEvent);

    if (animationIsContributing)
    {
      // Only start the animation of the lowest priority animation that animates and contributes to a
      // particular element/attribute pair.
      if (this == &firstAnimation)
      {
        StartAnimation();
      }

      if (oldActiveState == Inactive)
      {
        StartedActiveInterval();
      }

      // Only send repeat events here during normal animation run.
      // When seekToTime is true, all repeat events are handled in the seekToTime block below.
      if (!seekToTime && repeat && repeat != _lastRepeat)
      {
        SmilEventSender().DispatchEventSoon(*this, repeatEvent);
      }

      UpdateAnimation(percent, repeat);
      _lastPercent = percent;
      _lastRepeat = repeat;
    }

    if (oldActiveState == Active && _activeState != Active)
    {
      SmilEventSender().DispatchEventSoon(*this, endEvent);
      EndedActiveInterval();
      if (_activeState != Frozen)
      {
        StopAnimation(TargetElement());
      }
    }
    else if (oldActiveState != Active && _activeState == Active)
    {
      SmilEventSender().DispatchEventSoon(*this, beginEvent);
    }

    // Triggering all the pending events if the animation timeline is changed.
    // Handle repeat events entirely here when seeking.
    if (seekToTime)
    {
      if (_activeState == Inactive || _activeState == Frozen)
      {
        SmilEventSender().DispatchEventSoon(*this, endEvent);
      }

      if (repeat)
      {
        // We intentionally dispatch repeat - 1 events here because the first repeat
        // event (for the initial loop) is sent elsewhere during continuous animation run.
        // If repeat == 1, no events are dispatched here.
        for (uint32 i = 0; i < repeat - 1; ++i)
        {
          SmilEventSender().DispatchEventSoon(*this, repeatEvent);
        }

        if (_activeState == Inactive)
        {
          SmilEventSender().DispatchEventSoon(*this, repeatEvent);
        }
      }
    }

    _nextProgressTime = CalculateNextProgressTime(elapsed);
    return animationIsContributing;
  }

  void SVGSMILElement::NotifyDependentsIntervalChanged() noexcept
  {
    krys_debug_assert(_intervalBegin.IsFinite());

    static NeverDestroyed<List<const SVGSMILElement *>> visitedSetStorage;
    List<const SVGSMILElement *> &visitedSet = visitedSetStorage;

    if (std::ranges::find(visitedSet, this) != visitedSet.end())
    {
      return;
    }

    visitedSet.push_back(this);

    auto timeDependents = _timeDependents;
    for (auto &weakDependent : timeDependents)
    {
      if (RefPtr dependent = ShareRefPtr(weakDependent.get()))
      {
        dependent->CreateInstanceTimesFromSyncbase(this);
      }
    }

    std::erase(visitedSet, this);
  }

  void SVGSMILElement::CreateInstanceTimesFromSyncbase(SVGSMILElement *syncbase) noexcept
  {
    // FIXME: To be really correct, this should handle updating exising interval by changing
    // the associated times instead of creating new ones.
    for (auto &condition : _conditions)
    {
      if (condition.ConditionType == Condition::Syncbase && condition.SyncBase.get() == syncbase)
      {
        krys_debug_assert(condition.Name == u8"begin" || condition.Name == u8"end");
        // No nested time containers in SVG, no need for crazy time space conversions. Phew!
        SMILTime time = 0;
        if (condition.Name == u8"begin")
        {
          time = syncbase->_intervalBegin + condition.Offset;
        }
        else
        {
          time = syncbase->_intervalEnd + condition.Offset;
        }
        if (!time.IsFinite())
        {
          continue;
        }
        AddInstanceTime(condition.ConditionBeginOrEnd, time);
      }
    }
  }

  void SVGSMILElement::AddTimeDependent(SVGSMILElement *animation) noexcept
  {
    std::erase_if(_timeDependents, [](const auto &dependent) { return !dependent; });
    if (std::ranges::none_of(_timeDependents,
                             [&](const auto &dependent) { return dependent.get() == animation; }))
    {
      _timeDependents.push_back(CreateWeakPtr(animation));
    }
    if (_intervalBegin.IsFinite())
    {
      animation->CreateInstanceTimesFromSyncbase(this);
    }
  }

  void SVGSMILElement::RemoveTimeDependent(SVGSMILElement *animation) noexcept
  {
    std::erase_if(_timeDependents,
                  [&](const auto &dependent) { return !dependent || dependent.get() == animation; });
  }

  void SVGSMILElement::BeginByLinkActivation() noexcept
  {
    AddInstanceTime(Begin, Elapsed());
  }

  void SVGSMILElement::EndedActiveInterval() noexcept
  {
    ClearTimesWithDynamicOrigins(_beginTimes);
    ClearTimesWithDynamicOrigins(_endTimes);
  }

  void SVGSMILElement::DispatchPendingEvent(KRYS_MAYBE_UNUSED SMILEventSender *eventSender,
                                            const StringAtom &eventType) noexcept
  {
    krys_debug_assert(eventSender == &SmilEventSender());
    auto event = dom::Event::Create(eventType, false, false);
    (void)DispatchEvent(*event);
  }
}
