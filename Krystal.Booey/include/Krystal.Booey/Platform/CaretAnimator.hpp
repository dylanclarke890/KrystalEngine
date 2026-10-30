#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Document;
  class Node;
}

namespace krys::boo
{
  class CaretAnimator;
  class Color;
  class FloatRect;
  class GraphicsContext;
  class LayoutPoint;
  class Page;

  enum class CaretAnimatorStopReason : uint8
  {
    Default,
    CaretRectChanged,
  };

  /// @brief What a caret animator animates: the caret of a selection.
  class CaretAnimationClient
  {
  public:
    virtual ~CaretAnimationClient() noexcept = default;

    virtual void CaretAnimationDidUpdate(CaretAnimator &) noexcept
    {
    }

    KRYS_NODISCARD virtual LayoutRect LocalCaretRect() const noexcept = 0;
    KRYS_NODISCARD virtual dom::Document *GetDocument() noexcept = 0;
    KRYS_NODISCARD virtual dom::Node *CaretNode() noexcept = 0;
  };

  /// @brief Animates a caret: whether it's shown as it blinks, serviced at each rendering update its timer
  /// asks for.
  // FIXME: The accessibility setting that prefers a caret that doesn't blink, once the host reports it.
  class CaretAnimator
  {
    KRYS_NON_COPY_MOVABLE(CaretAnimator);

  public:
    struct PresentationProperties
    {
      enum class BlinkState : bool
      {
        Off,
        On
      };

      BlinkState Blink {BlinkState::On};
      float Opacity {1.0f};
    };

  protected:
    CaretAnimationClient &_client;
    MonotonicTime::duration _startTime {};
    Timer _blinkTimer;
    PresentationProperties _presentationProperties {};

  private:
    bool _isActive {false};
    bool _isBlinkingSuspended {false};

  public:
    virtual ~CaretAnimator() noexcept = default;

    virtual void Start() noexcept = 0;
    virtual void Stop(CaretAnimatorStopReason reason = CaretAnimatorStopReason::Default) noexcept;

    KRYS_NODISCARD bool IsActive() const noexcept
    {
      return _isActive;
    }

    void ServiceCaretAnimation() noexcept;

    virtual void SetBlinkingSuspended(bool suspended) noexcept
    {
      _isBlinkingSuspended = suspended;
    }
    KRYS_NODISCARD bool IsBlinkingSuspended() const noexcept;

    virtual void SetVisible(bool visible) noexcept = 0;

    KRYS_NODISCARD PresentationProperties GetPresentationProperties() const noexcept
    {
      return _presentationProperties;
    }

    virtual void Paint(GraphicsContext &context, const FloatRect &caret, const Color &color,
                       const LayoutPoint &paintOffset) const noexcept;
    KRYS_NODISCARD virtual LayoutRect CaretRepaintRectForLocalRect(LayoutRect rect) const noexcept;

  protected:
    explicit CaretAnimator(CaretAnimationClient &client) noexcept;

    virtual void UpdateAnimationProperties() noexcept = 0;

    void DidStart(MonotonicTime::duration currentTime, Maybe<MonotonicTime::duration> interval) noexcept;
    void DidEnd() noexcept;

    // FIXME: This is layering violation. Platform code should not rely on the rest of the engine.
    KRYS_NODISCARD Page *GetPage() const noexcept;

    /// @brief The current time: the event loop's, which is the host's.
    KRYS_NODISCARD static MonotonicTime::duration Now() noexcept;

  private:
    void ScheduleAnimation() noexcept;
  };

  KRYS_NODISCARD constexpr CaretAnimator::PresentationProperties::BlinkState
    operator!(CaretAnimator::PresentationProperties::BlinkState blinkState) noexcept
  {
    using BlinkState = CaretAnimator::PresentationProperties::BlinkState;
    return blinkState == BlinkState::Off ? BlinkState::On : BlinkState::Off;
  }
}
