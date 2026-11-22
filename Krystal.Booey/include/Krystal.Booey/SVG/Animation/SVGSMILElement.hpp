/*
 * Copyright (C) 2008-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2013-2020 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Events/EventSender.hpp"
#include "Krystal.Booey/SVG/Animation/SMILTime.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo
{
  class SVGSMILElement;
}

namespace krys::boo::svg
{
  class ConditionEventListener;
  class SMILTimeContainer;

  using SMILEventSender = dom::EventSender<SVGSMILElement>;
}

namespace krys::boo
{
  /// @brief An element timed by SMIL's interval timing model, as SVG animation needs it.
  /// @see https://svgwg.org/specs/animations/#TimingAttributes
  class SVGSMILElement : public SVGElement
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGSMILElement);
    friend class svg::ConditionEventListener;

  public:
    enum Restart : uint8
    {
      RestartAlways,
      RestartWhenNotActive,
      RestartNever,
    };

    enum FillMode : uint8
    {
      FillRemove,
      FillFreeze,
    };

  protected:
    enum ActiveState : uint8
    {
      Inactive,
      Active,
      Frozen,
    };

    enum BeginOrEnd : uint8
    {
      Begin,
      End,
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
        AccessKey,
      };

      Condition(Type, BeginOrEnd, DOMString baseID, DOMStringAtom name, svg::SMILTime offset,
                int32 repeats = -1) noexcept;

      Type ConditionType;
      BeginOrEnd ConditionBeginOrEnd;
      DOMString BaseID;
      DOMStringAtom Name;
      svg::SMILTime Offset;
      int32 Repeats {-1};
      RefPtr<Element> SyncBase;
      RefPtr<svg::ConditionEventListener> EventListener;
    };

    QualifiedName _attributeName;

    WeakPtr<SVGElement> _targetElement;

    List<Condition> _conditions;
    bool _conditionsConnected {false};
    bool _hasEndEventConditions {false};

    bool _isWaitingForFirstInterval {true};

    List<WeakPtr<SVGSMILElement>> _timeDependents;

    // Instance time lists
    List<svg::SMILTimeWithOrigin> _beginTimes;
    List<svg::SMILTimeWithOrigin> _endTimes;

    // This is the upcoming or current interval
    svg::SMILTime _intervalBegin {svg::SMILTime::Unresolved()};
    svg::SMILTime _intervalEnd {svg::SMILTime::Unresolved()};

    svg::SMILTime _previousIntervalBegin {svg::SMILTime::Unresolved()};

    ActiveState _activeState {Inactive};
    float _lastPercent {0};
    uint32 _lastRepeat {0};

    svg::SMILTime _nextProgressTime {0};

    RefPtr<svg::SMILTimeContainer> _timeContainer;
    uint32 _documentOrderIndex {0};

    mutable svg::SMILTime _cachedDur;
    mutable svg::SMILTime _cachedRepeatDur;
    mutable svg::SMILTime _cachedRepeatCount;
    mutable svg::SMILTime _cachedMin;
    mutable svg::SMILTime _cachedMax;

  public:
    SVGSMILElement(Document &, UniquePtr<SVGPropertyRegistry> &&, SVGElementInterface) noexcept;

    ~SVGSMILElement() noexcept override;

    void SVGAttributeChanged(const QualifiedName &) noexcept override;

    KRYS_NODISCARD virtual bool HasValidAttributeType() const noexcept = 0;

    KRYS_NODISCARD virtual bool HasValidAttributeName() const noexcept;

    virtual void AnimationAttributeChanged() noexcept = 0;

    KRYS_NODISCARD svg::SMILTimeContainer *TimeContainer() noexcept;

    KRYS_NODISCARD SVGElement *TargetElement() const noexcept
    {
      return _targetElement.get();
    }

    KRYS_NODISCARD const QualifiedName &AttributeName() const noexcept
    {
      return _attributeName;
    }

    void BeginByLinkActivation() noexcept;

    KRYS_NODISCARD Restart GetRestart() const noexcept;

    KRYS_NODISCARD FillMode Fill() const noexcept;

    KRYS_NODISCARD svg::SMILTime Dur() const noexcept;

    KRYS_NODISCARD svg::SMILTime RepeatDur() const noexcept;

    KRYS_NODISCARD svg::SMILTime RepeatCount() const noexcept;

    KRYS_NODISCARD svg::SMILTime MaxValue() const noexcept;

    KRYS_NODISCARD svg::SMILTime MinValue() const noexcept;

    KRYS_NODISCARD svg::SMILTime Elapsed() const noexcept;

    KRYS_NODISCARD svg::SMILTime IntervalBegin() const noexcept
    {
      return _intervalBegin;
    }

    KRYS_NODISCARD svg::SMILTime PreviousIntervalBegin() const noexcept
    {
      return _previousIntervalBegin;
    }

    KRYS_NODISCARD svg::SMILTime SimpleDuration() const noexcept;

    void SeekToIntervalCorrespondingToTime(svg::SMILTime elapsed) noexcept;

    bool Progress(svg::SMILTime elapsed, SVGSMILElement &firstAnimation, bool seekToTime) noexcept;

    KRYS_NODISCARD svg::SMILTime NextProgressTime() const noexcept;

    void Reset() noexcept;

    KRYS_NODISCARD static svg::SMILTime ParseClockValue(Maybe<DOMStringView>) noexcept;

    KRYS_NODISCARD static svg::SMILTime ParseOffsetValue(DOMStringView) noexcept;

    KRYS_NODISCARD bool IsContributing(svg::SMILTime elapsed) const noexcept;

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

    void DispatchPendingEvent(svg::SMILEventSender *, const StringAtom &eventType) noexcept;

  protected:
    KRYS_NODISCARD ActiveState GetActiveState() const noexcept
    {
      return _activeState;
    }

    void SetInactive() noexcept
    {
      _activeState = Inactive;
    }

    void AttributeChanged(const QualifiedName &, Maybe<DOMStringView> oldValue,
                          Maybe<DOMStringView> value) noexcept override;

    void OnInsert() noexcept override;

    void OnPostConnection() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept override
    {
      return false;
    }

    // Sub-classes may need to take action when the target is changed.
    virtual void SetTargetElement(SVGElement *) noexcept;

    virtual void SetAttributeName(const QualifiedName &) noexcept;

    void AddInstanceTime(BeginOrEnd, svg::SMILTime,
                         svg::SMILTimeWithOrigin::Origin = svg::SMILTimeWithOrigin::Origin::Parser) noexcept;

    /// @brief The value of the attribute @p name, if it's set.
    KRYS_NODISCARD Maybe<DOMString> AttributeValue(const QualifiedName &name) const noexcept;

  private:
    void BuildPendingResource() noexcept override;

    void ClearResourceReferences() noexcept;

    void ClearTarget() noexcept override;

    virtual void StartedActiveInterval() noexcept = 0;

    void EndedActiveInterval() noexcept;

    virtual void UpdateAnimation(float percent, uint32 repeat) noexcept = 0;

    KRYS_NODISCARD static bool IsSupportedAttribute(const QualifiedName &) noexcept;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    KRYS_NODISCARD QualifiedName ConstructAttributeName() const noexcept;

    void UpdateAttributeName() noexcept;

    KRYS_NODISCARD svg::SMILTime FindInstanceTime(BeginOrEnd, svg::SMILTime minimumTime,
                                                  bool equalsMinimumOK) const noexcept;

    void ResolveFirstInterval() noexcept;

    bool ResolveNextInterval() noexcept;

    void ResolveInterval(bool first, svg::SMILTime &beginResult, svg::SMILTime &endResult) const noexcept;

    KRYS_NODISCARD svg::SMILTime ResolveActiveEnd(svg::SMILTime resolvedBegin,
                                                  svg::SMILTime resolvedEnd) const noexcept;

    KRYS_NODISCARD svg::SMILTime RepeatingDuration() const noexcept;

    void CheckRestart(svg::SMILTime elapsed) noexcept;

    void BeginListChanged(svg::SMILTime eventTime) noexcept;

    void EndListChanged(svg::SMILTime eventTime) noexcept;

    bool ParseCondition(DOMStringView, BeginOrEnd) noexcept;

    void ParseBeginOrEnd(DOMStringView, BeginOrEnd) noexcept;

    KRYS_NODISCARD RefPtr<Element> EventBaseFor(const Condition &) noexcept;

    void DisconnectConditions() noexcept;

    void NotifyDependentsIntervalChanged() noexcept;

    void CreateInstanceTimesFromSyncbase(SVGSMILElement *syncbase) noexcept;

    void AddTimeDependent(SVGSMILElement *) noexcept;

    void RemoveTimeDependent(SVGSMILElement *) noexcept;

    KRYS_NODISCARD ActiveState DetermineActiveState(svg::SMILTime elapsed) const noexcept;

    KRYS_NODISCARD float CalculateAnimationPercentAndRepeat(svg::SMILTime elapsed,
                                                            uint32 &repeat) const noexcept;

    KRYS_NODISCARD svg::SMILTime CalculateNextProgressTime(svg::SMILTime elapsed) const noexcept;

    KRYS_NODISCARD bool IsSMILElement() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGSMILElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::SVGElement &element) noexcept
  {
    return element.IsSMILElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
