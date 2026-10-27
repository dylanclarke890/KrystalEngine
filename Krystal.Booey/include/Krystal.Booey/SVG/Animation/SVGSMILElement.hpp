#pragma once

#include "Krystal.Booey/DOM/EventSender.hpp"
#include "Krystal.Booey/SVG/Animation/SMILTime.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  class ConditionEventListener;
  class SMILTimeContainer;
  class SVGSMILElement;

  using SMILEventSender = dom::EventSender<SVGSMILElement>;

  /// @brief An element timed by SMIL's interval timing model, as SVG animation needs it.
  /// @see https://svgwg.org/specs/animations/#TimingAttributes
  class SVGSMILElement : public SVGElement
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGSMILElement);
    friend class ConditionEventListener;

  public:
    enum Restart : uint8
    {
      RestartAlways,
      RestartWhenNotActive,
      RestartNever
    };

    enum FillMode : uint8
    {
      FillRemove,
      FillFreeze
    };

  protected:
    enum ActiveState : uint8
    {
      Inactive,
      Active,
      Frozen
    };

    enum BeginOrEnd : uint8
    {
      Begin,
      End
    };

  private:
    // This represents conditions on elements begin or end list that need to be resolved on runtime
    // for example <animate begin="otherElement.begin + 8s; button.click" ... />
    struct Condition
    {
      enum Type : uint8
      {
        EventBase,
        Syncbase,
        AccessKey
      };

      Condition(Type, BeginOrEnd, dom::DOMString baseID, dom::DOMStringAtom name, SMILTime offset,
                int32 repeats = -1) noexcept;

      Type ConditionType;
      BeginOrEnd ConditionBeginOrEnd;
      dom::DOMString BaseID;
      dom::DOMStringAtom Name;
      SMILTime Offset;
      int32 Repeats {-1};
      RefPtr<dom::Element> SyncBase;
      RefPtr<ConditionEventListener> EventListener;
    };

    dom::QualifiedName _attributeName;

    WeakPtr<SVGElement> _targetElement;

    List<Condition> _conditions;
    bool _conditionsConnected {false};
    bool _hasEndEventConditions {false};

    bool _isWaitingForFirstInterval {true};

    List<WeakPtr<SVGSMILElement>> _timeDependents;

    // Instance time lists
    List<SMILTimeWithOrigin> _beginTimes;
    List<SMILTimeWithOrigin> _endTimes;

    // This is the upcoming or current interval
    SMILTime _intervalBegin {SMILTime::Unresolved()};
    SMILTime _intervalEnd {SMILTime::Unresolved()};

    SMILTime _previousIntervalBegin {SMILTime::Unresolved()};

    ActiveState _activeState {Inactive};
    float _lastPercent {0};
    uint32 _lastRepeat {0};

    SMILTime _nextProgressTime {0};

    RefPtr<SMILTimeContainer> _timeContainer;
    uint32 _documentOrderIndex {0};

    mutable SMILTime _cachedDur;
    mutable SMILTime _cachedRepeatDur;
    mutable SMILTime _cachedRepeatCount;
    mutable SMILTime _cachedMin;
    mutable SMILTime _cachedMax;

  public:
    SVGSMILElement(dom::Document &, UniquePtr<SVGPropertyRegistry> &&, SVGElementInterface) noexcept;
    ~SVGSMILElement() noexcept override;

    void SVGAttributeChanged(const dom::QualifiedName &) noexcept override;

    KRYS_NODISCARD virtual bool HasValidAttributeType() const noexcept = 0;
    KRYS_NODISCARD virtual bool HasValidAttributeName() const noexcept;
    virtual void AnimationAttributeChanged() noexcept = 0;

    KRYS_NODISCARD SMILTimeContainer *TimeContainer() noexcept;

    KRYS_NODISCARD SVGElement *TargetElement() const noexcept
    {
      return _targetElement.get();
    }

    KRYS_NODISCARD const dom::QualifiedName &AttributeName() const noexcept
    {
      return _attributeName;
    }

    void BeginByLinkActivation() noexcept;

    KRYS_NODISCARD Restart GetRestart() const noexcept;
    KRYS_NODISCARD FillMode Fill() const noexcept;

    KRYS_NODISCARD SMILTime Dur() const noexcept;
    KRYS_NODISCARD SMILTime RepeatDur() const noexcept;
    KRYS_NODISCARD SMILTime RepeatCount() const noexcept;
    KRYS_NODISCARD SMILTime MaxValue() const noexcept;
    KRYS_NODISCARD SMILTime MinValue() const noexcept;

    KRYS_NODISCARD SMILTime Elapsed() const noexcept;

    KRYS_NODISCARD SMILTime IntervalBegin() const noexcept
    {
      return _intervalBegin;
    }

    KRYS_NODISCARD SMILTime PreviousIntervalBegin() const noexcept
    {
      return _previousIntervalBegin;
    }

    KRYS_NODISCARD SMILTime SimpleDuration() const noexcept;

    void SeekToIntervalCorrespondingToTime(SMILTime elapsed) noexcept;
    bool Progress(SMILTime elapsed, SVGSMILElement &firstAnimation, bool seekToTime) noexcept;
    KRYS_NODISCARD SMILTime NextProgressTime() const noexcept;

    void Reset() noexcept;

    KRYS_NODISCARD static SMILTime ParseClockValue(Maybe<dom::DOMStringView>) noexcept;
    KRYS_NODISCARD static SMILTime ParseOffsetValue(dom::DOMStringView) noexcept;

    KRYS_NODISCARD bool IsContributing(SMILTime elapsed) const noexcept;
    KRYS_NODISCARD bool IsFrozen() const noexcept;

    KRYS_NODISCARD uint32 DocumentOrderIndex() const noexcept
    {
      return _documentOrderIndex;
    }

    void SetDocumentOrderIndex(uint32 index) noexcept
    {
      _documentOrderIndex = index;
    }

    KRYS_NODISCARD virtual bool IsAdditive() const noexcept = 0;
    virtual void StartAnimation() noexcept = 0;
    virtual void StopAnimation(SVGElement *targetElement) noexcept = 0;
    virtual void ApplyResultsToTarget() noexcept = 0;

    void ConnectConditions() noexcept;

    KRYS_NODISCARD bool HasConditionsConnected() const noexcept
    {
      return _conditionsConnected;
    }

    void DispatchPendingEvent(SMILEventSender *, const StringAtom &eventType) noexcept;

  protected:
    KRYS_NODISCARD ActiveState GetActiveState() const noexcept
    {
      return _activeState;
    }

    void SetInactive() noexcept
    {
      _activeState = Inactive;
    }

    void AttributeChanged(const dom::QualifiedName &, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void OnInsert() noexcept override;
    void OnPostConnection() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept override
    {
      return false;
    }

    // Sub-classes may need to take action when the target is changed.
    virtual void SetTargetElement(SVGElement *) noexcept;
    virtual void SetAttributeName(const dom::QualifiedName &) noexcept;

    void AddInstanceTime(BeginOrEnd, SMILTime,
                         SMILTimeWithOrigin::Origin = SMILTimeWithOrigin::Origin::Parser) noexcept;

    /// @brief The value of the attribute @p name, if it's set.
    KRYS_NODISCARD Maybe<dom::DOMString> AttributeValue(const dom::QualifiedName &name) const noexcept;

  private:
    void BuildPendingResource() noexcept override;
    void ClearResourceReferences() noexcept;

    void ClearTarget() noexcept override;

    virtual void StartedActiveInterval() noexcept = 0;
    void EndedActiveInterval() noexcept;
    virtual void UpdateAnimation(float percent, uint32 repeat) noexcept = 0;

    KRYS_NODISCARD static bool IsSupportedAttribute(const dom::QualifiedName &) noexcept;
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    KRYS_NODISCARD dom::QualifiedName ConstructAttributeName() const noexcept;
    void UpdateAttributeName() noexcept;

    KRYS_NODISCARD SMILTime FindInstanceTime(BeginOrEnd, SMILTime minimumTime,
                                             bool equalsMinimumOK) const noexcept;
    void ResolveFirstInterval() noexcept;
    bool ResolveNextInterval() noexcept;
    void ResolveInterval(bool first, SMILTime &beginResult, SMILTime &endResult) const noexcept;
    KRYS_NODISCARD SMILTime ResolveActiveEnd(SMILTime resolvedBegin, SMILTime resolvedEnd) const noexcept;
    KRYS_NODISCARD SMILTime RepeatingDuration() const noexcept;
    void CheckRestart(SMILTime elapsed) noexcept;
    void BeginListChanged(SMILTime eventTime) noexcept;
    void EndListChanged(SMILTime eventTime) noexcept;

    bool ParseCondition(dom::DOMStringView, BeginOrEnd) noexcept;
    void ParseBeginOrEnd(dom::DOMStringView, BeginOrEnd) noexcept;
    KRYS_NODISCARD RefPtr<dom::Element> EventBaseFor(const Condition &) noexcept;

    void DisconnectConditions() noexcept;

    void NotifyDependentsIntervalChanged() noexcept;
    void CreateInstanceTimesFromSyncbase(SVGSMILElement *syncbase) noexcept;
    void AddTimeDependent(SVGSMILElement *) noexcept;
    void RemoveTimeDependent(SVGSMILElement *) noexcept;

    KRYS_NODISCARD ActiveState DetermineActiveState(SMILTime elapsed) const noexcept;
    KRYS_NODISCARD float CalculateAnimationPercentAndRepeat(SMILTime elapsed, uint32 &repeat) const noexcept;
    KRYS_NODISCARD SMILTime CalculateNextProgressTime(SMILTime elapsed) const noexcept;

    KRYS_NODISCARD bool IsSMILElement() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGSMILElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &element) noexcept
  {
    return element.IsSMILElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
