#include "Krystal.Booey/Modules/Gamepad/GamepadHapticActuator.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Modules/Gamepad/Gamepad.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadEffectParameters.hpp"
#include "Krystal.Booey/Platform/Gamepad/GamepadProvider.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool AreEffectParametersValid(GamepadHapticEffectType effectType,
                                                 const GamepadEffectParameters &parameters) noexcept
    {
      if (parameters.Duration < 0 || parameters.StartDelay < 0)
      {
        return false;
      }

      if (effectType == GamepadHapticEffectType::DualRumble)
      {
        if (parameters.WeakMagnitude < 0 || parameters.StrongMagnitude < 0 || parameters.WeakMagnitude > 1
            || parameters.StrongMagnitude > 1)
        {
          return false;
        }
      }
      return true;
    }

    /// @brief Calls @p promise with @p result, as a microtask.
    void Settle(GamepadHapticsCallback &&promise, dom::ExceptionOr<GamepadHapticsResult> &&result) noexcept
    {
      if (!promise)
      {
        return;
      }
      html::EventLoop::Current().QueueMicrotask(
        [promise = krys::move(promise), result = krys::move(result)]() mutable
        { promise(krys::move(result)); });
    }
  }

  GamepadHapticActuator::GamepadHapticActuator(dom::Document *document, Type type, Gamepad &gamepad) noexcept
      : _document(CreateWeakPtr(document)), _type(type), _gamepad(CreateWeakPtr(&gamepad))
  {
    if (document)
    {
      document->RegisterForVisibilityStateChangedCallbacks(*this);
    }
  }

  GamepadHapticActuator::~GamepadHapticActuator() noexcept
  {
    if (auto *document = _document.get())
    {
      document->UnregisterForVisibilityStateChangedCallbacks(*this);
    }
  }

  bool GamepadHapticActuator::CanPlayEffectType(EffectType effectType) const noexcept
  {
    auto *gamepad = _gamepad.get();
    return gamepad && gamepad->SupportedEffectTypes().Contains(effectType);
  }

  void GamepadHapticActuator::PlayEffect(EffectType effectType, GamepadEffectParameters &&effectParameters,
                                         GamepadHapticsCallback &&promise) noexcept
  {
    if (!AreEffectParametersValid(effectType, effectParameters))
    {
      Settle(krys::move(promise),
             dom::Exception {dom::ExceptionCode::TypeError, u8"Invalid effect parameter"});
      return;
    }

    RefPtr<dom::Document> document = ShareRefPtr(_document.get());
    auto *gamepad = _gamepad.get();
    if (!document || !document->IsFullyActive() || document->Hidden() || !gamepad)
    {
      Settle(krys::move(promise), GamepadHapticsResult::Preempted);
      return;
    }

    auto &currentEffectPromise = PromiseForEffectType(effectType);
    if (auto playingEffectPromise = std::exchange(currentEffectPromise, nullptr))
    {
      QueueTaskToSettle(krys::move(playingEffectPromise), GamepadHapticsResult::Preempted);
    }

    if (!CanPlayEffectType(effectType))
    {
      Settle(krys::move(promise), dom::Exception {dom::ExceptionCode::NotSupportedError,
                                                  u8"This gamepad doesn't support playing such effect"});
      return;
    }

    effectParameters.Duration = std::min(effectParameters.Duration, GamepadEffectParameters::MaximumDuration);
    currentEffectPromise = CreateRef<PendingPromise>(krys::move(promise));
    GamepadProvider::Singleton().PlayEffect(
      gamepad->Index(), gamepad->Id(), effectType, effectParameters,
      [protectedThis = ShareRef(*this), playingEffectPromise = currentEffectPromise, effectType](bool success)
      {
        auto &currentEffectPromise = protectedThis->PromiseForEffectType(effectType);
        if (playingEffectPromise != currentEffectPromise)
        {
          // Was already pre-empted.
          return;
        }
        protectedThis->QueueTaskToSettle(std::exchange(currentEffectPromise, nullptr),
                                         success ? GamepadHapticsResult::Complete
                                                 : GamepadHapticsResult::Preempted);
      });
  }

  void GamepadHapticActuator::Reset(GamepadHapticsCallback &&promise) noexcept
  {
    RefPtr<dom::Document> document = ShareRefPtr(_document.get());
    if (!document || !document->IsFullyActive() || document->Hidden() || !_gamepad.get())
    {
      Settle(krys::move(promise), GamepadHapticsResult::Preempted);
      return;
    }

    StopEffects(
      [protectedThis = ShareRef(*this), promise = krys::move(promise)]() mutable
      {
        protectedThis->QueueTaskToSettle(CreateRef<PendingPromise>(krys::move(promise)),
                                         GamepadHapticsResult::Complete);
      });
  }

  void GamepadHapticActuator::Stop() noexcept
  {
    StopEffects([] {});
  }

  void GamepadHapticActuator::VisibilityStateChanged() noexcept
  {
    RefPtr<dom::Document> document = ShareRefPtr(_document.get());
    if (!document || !document->Hidden())
    {
      return;
    }
    StopEffects([] {});
  }

  void GamepadHapticActuator::StopEffects(Func<void()> &&completionHandler) noexcept
  {
    if (!_dualRumbleEffectPromise)
    {
      completionHandler();
      return;
    }

    QueueTaskToSettle(std::exchange(_dualRumbleEffectPromise, nullptr), GamepadHapticsResult::Preempted);
    auto *gamepad = _gamepad.get();
    if (!gamepad)
    {
      completionHandler();
      return;
    }
    GamepadProvider::Singleton().StopEffects(gamepad->Index(), gamepad->Id(), krys::move(completionHandler));
  }

  RefPtr<GamepadHapticActuator::PendingPromise> &
    GamepadHapticActuator::PromiseForEffectType(EffectType effectType) noexcept
  {
    switch (effectType)
    {
      case EffectType::DualRumble: break;
    }
    return _dualRumbleEffectPromise;
  }

  void GamepadHapticActuator::QueueTaskToSettle(RefPtr<PendingPromise> &&promise,
                                                dom::ExceptionOr<GamepadHapticsResult> &&result) noexcept
  {
    auto *document = _document.get();
    if (!document || !promise)
    {
      return;
    }
    html::EventLoop::Current().QueueTask(
      html::TaskSource::Gamepad, *document,
      [protectedThis = ShareRef(*this), promise = krys::move(promise), result = krys::move(result)]() mutable
      {
        if (promise->Callback)
        {
          promise->Callback(krys::move(result));
        }
      });
  }
}
