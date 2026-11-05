#pragma once

#include "Krystal.Booey/DOM/Types/Exception.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class WebAnimation;
}

namespace krys::boo::detail
{
  /// @brief An animation's current ready or finished promise, which `WebAnimation::WhenReady` and
  /// `WebAnimation::WhenFinished` wait on: it's fulfilled with the animation, or rejected with an exception,
  /// once. Callbacks given before or after it settles run as microtasks once it has.
  /// @see https://drafts.csswg.org/web-animations-1/#current-ready-promise
  /// @see https://drafts.csswg.org/web-animations-1/#current-finished-promise
  class AnimationPromise final : public RefCounted<AnimationPromise>
  {
    KRYS_NON_COPY_MOVABLE(AnimationPromise);

  public:
    using FulfilledCallback = Func<void(WebAnimation &animation)>;
    using RejectedCallback = Func<void(const dom::Exception &exception)>;

  private:
    enum class Status : uint8
    {
      Pending,
      Fulfilled,
      Rejected
    };

    Status _status {Status::Pending};
    /// @brief Weak, as the animation holds its promises.
    WeakPtr<WebAnimation> _animation;
    Maybe<dom::Exception> _exception;
    List<Pair<FulfilledCallback, RejectedCallback>> _callbacks;

    explicit AnimationPromise(WebAnimation &animation) noexcept;

  public:
    ~AnimationPromise() noexcept;

    /// @brief A pending promise of @p animation.
    KRYS_NODISCARD static Ref<AnimationPromise> Create(WebAnimation &animation) noexcept;

    /// @brief Whether the promise has settled: it's been fulfilled or rejected.
    KRYS_NODISCARD bool IsSettled() const noexcept
    {
      return _status != Status::Pending;
    }

    /// @brief Calls @p onFulfilled or @p onRejected (either may be null) as a microtask once the promise
    /// settles. A fulfilled promise's callback isn't called if the animation is gone.
    void Then(FulfilledCallback onFulfilled, RejectedCallback onRejected) noexcept;

    /// @brief Fulfils the promise with its animation.
    void Resolve() noexcept;
    void Reject(dom::Exception exception) noexcept;

  private:
    void Settle() noexcept;
  };
}
