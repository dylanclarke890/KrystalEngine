#include "Krystal.Booey/Animation/AnimationPromise.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo
{
  AnimationPromise::AnimationPromise(WebAnimation &animation) noexcept : _animation(CreateWeakPtr(&animation))
  {
  }

  AnimationPromise::~AnimationPromise() noexcept = default;

  Ref<AnimationPromise> AnimationPromise::Create(WebAnimation &animation) noexcept
  {
    return AdoptRef(*new AnimationPromise(animation));
  }

  WebAnimation *AnimationPromise::Animation() const noexcept
  {
    return _animation.get();
  }

  void AnimationPromise::Then(FulfilledCallback &&onFulfilled, RejectedCallback &&onRejected) noexcept
  {
    _callbacks.emplace_back(krys::move(onFulfilled), krys::move(onRejected));
    if (_status != Status::Pending)
    {
      Settle();
    }
  }

  void AnimationPromise::Resolve() noexcept
  {
    if (_status != Status::Pending)
    {
      return;
    }
    _status = Status::Fulfilled;
    Settle();
  }

  void AnimationPromise::Reject(dom::Exception &&exception) noexcept
  {
    if (_status != Status::Pending)
    {
      return;
    }
    _status = Status::Rejected;
    _exception = krys::move(exception);
    Settle();
  }

  void AnimationPromise::Settle() noexcept
  {
    // A promise's reactions run as microtasks.
    for (auto &[onFulfilled, onRejected] : std::exchange(_callbacks, {}))
    {
      html::EventLoop::Current().QueueMicrotask(
        [protectedThis = Ref<AnimationPromise>(ShareRef(*this)), onFulfilled = krys::move(onFulfilled),
         onRejected = krys::move(onRejected)]() mutable
        {
          if (protectedThis->_status == Status::Fulfilled && onFulfilled)
          {
            if (RefPtr animation = ShareRefPtr(protectedThis->_animation.get()))
            {
              onFulfilled(*animation);
            }
          }
          else if (protectedThis->_status == Status::Rejected && onRejected)
          {
            onRejected(*protectedThis->_exception);
          }
        });
    }
  }
}
