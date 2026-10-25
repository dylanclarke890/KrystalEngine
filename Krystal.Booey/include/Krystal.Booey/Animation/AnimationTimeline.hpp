#pragma once

#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class AnimationTimelinesController;
  class WebAnimation;

  namespace style
  {
    struct SingleAnimationRange;
  }

  using AnimationCollection = ListHashSet<Ref<WebAnimation>>;

  /// @brief A source of time for animations.
  /// @see https://drafts.csswg.org/web-animations-1/#timelines
  // FIXME: Accelerated timelines, once there are accelerated animations.
  class AnimationTimeline : public RefCounted<AnimationTimeline>, public CanMakeWeakPtr<AnimationTimeline>
  {
  protected:
    AnimationCollection _animations;

  private:
    Maybe<WebAnimationTime> _currentTime;
    Maybe<WebAnimationTime> _duration;

  protected:
    explicit AnimationTimeline(Maybe<WebAnimationTime> duration = null) noexcept;

  public:
    virtual ~AnimationTimeline() noexcept;

    KRYS_NODISCARD virtual bool IsDocumentTimeline() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsScrollTimeline() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsViewTimeline() const noexcept
    {
      return false;
    }

    /// @see https://drafts.csswg.org/web-animations-1/#monotonically-increasing-timeline
    KRYS_NODISCARD bool IsMonotonic() const noexcept
    {
      return !_duration;
    }
    KRYS_NODISCARD bool IsProgressBased() const noexcept
    {
      return !IsMonotonic();
    }

    KRYS_NODISCARD const AnimationCollection &RelevantAnimations() const noexcept
    {
      return _animations;
    }

    virtual void AnimationTimingDidChange(WebAnimation &animation) noexcept;
    virtual void RemoveAnimation(WebAnimation &animation) noexcept;

    KRYS_NODISCARD virtual Maybe<WebAnimationTime>
      CurrentTime(UseCachedCurrentTime = UseCachedCurrentTime::Yes) noexcept
    {
      return _currentTime;
    }
    KRYS_NODISCARD virtual Maybe<WebAnimationTime> Duration() const noexcept
    {
      return _duration;
    }

    virtual void DetachFromDocument() noexcept;

    enum class ShouldUpdateAnimationsAndSendEvents : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD virtual ShouldUpdateAnimationsAndSendEvents
      DocumentWillUpdateAnimationsAndSendEvents() noexcept
    {
      return ShouldUpdateAnimationsAndSendEvents::No;
    }

    virtual void SuspendAnimations() noexcept;
    virtual void ResumeAnimations() noexcept;
    KRYS_NODISCARD bool AnimationsAreSuspended() const noexcept;

    KRYS_NODISCARD virtual AnimationTimelinesController *Controller() const noexcept
    {
      return nullptr;
    }

    /// @brief The attachment range of animations whose range is `normal`.
    KRYS_NODISCARD virtual style::SingleAnimationRange DefaultRange() const noexcept;

    /// @brief Gives @p animation its position in the global animation list, if it has none yet.
    static void UpdateGlobalPosition(WebAnimation &animation) noexcept;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_TIMELINE(ToValueTypeName, predicate)                           \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::AnimationTimeline &value) noexcept                    \
    {                                                                                                        \
      return value.predicate;                                                                                \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
