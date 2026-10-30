#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/DOM/VisibilityChangeClient.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadHapticEffectType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class Gamepad;
  struct GamepadEffectParameters;

  /// @brief How a haptic effect ended.
  /// @see https://w3c.github.io/gamepad/#dom-gamepadhapticsresult
  enum class GamepadHapticsResult : uint8
  {
    /// @brief It played to the end (or the effects were reset).
    Complete,
    /// @brief Another effect, a reset or the document going away stopped it.
    Preempted
  };

  /// @brief The promise `playEffect()` and `reset()` return, as a callback: called once, with how the
  /// effect ended, or the exception it failed with.
  using GamepadHapticsCallback = Func<void(dom::ExceptionOr<GamepadHapticsResult> &&)>;

  /// @brief A gamepad's haptic motors, which play effects (`playEffect()`) until they end, another effect
  /// takes their place, or they're reset (`reset()`).
  /// @see https://w3c.github.io/gamepad/#gamepadhapticactuator-interface
  class GamepadHapticActuator final : public RefCounted<GamepadHapticActuator>,
                                      public dom::VisibilityChangeClient
  {
    KRYS_NON_COPY_MOVABLE(GamepadHapticActuator);

  public:
    using EffectType = GamepadHapticEffectType;

    /// @see https://w3c.github.io/gamepad/#dom-gamepadhapticactuatortype
    enum class Type : uint8
    {
      Vibration,
      DualRumble
    };

  private:
    /// @brief A pending promise, whose identity says whether a completion is still the current effect's.
    struct PendingPromise final : public RefCounted<PendingPromise>
    {
      GamepadHapticsCallback Callback;

      explicit PendingPromise(GamepadHapticsCallback &&callback) noexcept : Callback(krys::move(callback))
      {
      }
    };

    WeakPtr<dom::Document> _document;
    Type _type;
    WeakPtr<Gamepad> _gamepad;
    RefPtr<PendingPromise> _dualRumbleEffectPromise;

  public:
    GamepadHapticActuator(dom::Document *document, Type type, Gamepad &gamepad) noexcept;
    ~GamepadHapticActuator() noexcept;

    KRYS_NODISCARD static Ref<GamepadHapticActuator> Create(dom::Document *document, Type type,
                                                            Gamepad &gamepad) noexcept
    {
      return CreateRef<GamepadHapticActuator>(document, type, gamepad);
    }

    /// @see https://w3c.github.io/gamepad/#dom-gamepadhapticactuator-type
    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    /// @brief Whether the pad can play @p effectType's effects.
    KRYS_NODISCARD bool CanPlayEffectType(EffectType effectType) const noexcept;

    /// @brief Plays an effect of @p effectType, preempting the one playing; @p promise is called as it ends.
    /// @return Through @p promise: a `TypeError` if @p effectParameters are out of range, or a
    /// `NotSupportedError` if the pad can't play it.
    /// @see https://w3c.github.io/gamepad/#dom-gamepadhapticactuator-playeffect
    void PlayEffect(EffectType effectType, GamepadEffectParameters &&effectParameters,
                    GamepadHapticsCallback &&promise) noexcept;

    /// @brief Stops the effect playing, preempting it; @p promise is called once it has stopped.
    /// @see https://w3c.github.io/gamepad/#dom-gamepadhapticactuator-reset
    void Reset(GamepadHapticsCallback &&promise) noexcept;

    /// @brief Stops the effect playing, as the document is suspended or goes away.
    void Stop() noexcept;

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }
    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }
    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

  private:
    /// @brief Stops the effect playing as the document is hidden.
    void VisibilityStateChanged() noexcept final;

    void StopEffects(Func<void()> &&completionHandler) noexcept;
    KRYS_NODISCARD RefPtr<PendingPromise> &PromiseForEffectType(EffectType effectType) noexcept;
    /// @brief Calls @p promise with @p result in a task of the document's.
    void QueueTaskToSettle(RefPtr<PendingPromise> &&promise,
                           dom::ExceptionOr<GamepadHapticsResult> &&result) noexcept;
  };
}
