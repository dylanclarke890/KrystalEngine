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

  /// @brief An animation's ready or finished promise, as script would wait on it: it's fulfilled with the
  /// animation, or rejected with an exception, once. Callbacks given before or after it settles run as
  /// microtasks once it has.
  /// @see https://drafts.csswg.org/web-animations-1/#dom-animation-ready
  /// @see https://drafts.csswg.org/web-animations-1/#dom-animation-finished
  class AnimationPromise final : public RefCounted<AnimationPromise>
  {
    KRYS_NON_COPY_MOVABLE(AnimationPromise);

  public:
    enum class Status : uint8
    {
      Pending,
      Fulfilled,
      Rejected
    };

    using FulfilledCallback = Func<void(WebAnimation &animation)>;
    using RejectedCallback = Func<void(const dom::Exception &exception)>;

  private:
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

    KRYS_NODISCARD Status GetStatus() const noexcept
    {
      return _status;
    }

    /// @brief Whether the promise has settled: it's been fulfilled or rejected.
    KRYS_NODISCARD bool IsFulfilled() const noexcept
    {
      return _status != Status::Pending;
    }

    /// @brief The animation the promise is for; null once it's gone.
    KRYS_NODISCARD WebAnimation *Animation() const noexcept;

    /// @brief The exception the promise was rejected with, if it was.
    KRYS_NODISCARD const Maybe<dom::Exception> &GetException() const noexcept
    {
      return _exception;
    }

    /// @brief Calls @p onFulfilled or @p onRejected (either may be null) as a microtask once the promise
    /// settles. A fulfilled promise's callback isn't called if the animation is gone.
    void Then(FulfilledCallback &&onFulfilled, RejectedCallback &&onRejected = nullptr) noexcept;

    /// @brief Fulfils the promise with its animation.
    void Resolve() noexcept;
    void Reject(dom::Exception &&exception) noexcept;

  private:
    void Settle() noexcept;
  };
}
