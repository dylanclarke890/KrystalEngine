#include "Krystal.Booey/Platform/CaretAnimator.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/Page.hpp"

namespace krys::boo
{
  CaretAnimator::CaretAnimator(CaretAnimationClient &client) noexcept
      : _client(client), _blinkTimer(*this, &CaretAnimator::ScheduleAnimation)
  {
  }

  bool CaretAnimator::IsBlinkingSuspended() const noexcept
  {
    return _isBlinkingSuspended;
  }

  Page *CaretAnimator::GetPage() const noexcept
  {
    if (auto *document = _client.GetDocument())
    {
      return document->GetPage();
    }

    return nullptr;
  }

  MonotonicTime::duration CaretAnimator::Now() noexcept
  {
    return html::EventLoop::Current().Now();
  }

  void CaretAnimator::DidStart(MonotonicTime::duration currentTime,
                               Maybe<MonotonicTime::duration> interval) noexcept
  {
    _startTime = currentTime;
    _isActive = true;
    SetBlinkingSuspended(!interval);
    if (interval)
    {
      _blinkTimer.StartOneShot(*interval);
    }
  }

  void CaretAnimator::DidEnd() noexcept
  {
    _isActive = false;
    _blinkTimer.Stop();
  }

  void CaretAnimator::Stop(CaretAnimatorStopReason) noexcept
  {
    if (!_isActive)
    {
      return;
    }
    DidEnd();
  }

  void CaretAnimator::ServiceCaretAnimation() noexcept
  {
    if (!IsActive())
    {
      return;
    }
    UpdateAnimationProperties();
  }

  void CaretAnimator::ScheduleAnimation() noexcept
  {
    if (auto *page = GetPage())
    {
      page->ScheduleRenderingUpdate({RenderingUpdateStep::CaretAnimation});
    }
  }

  void CaretAnimator::Paint(GraphicsContext &context, const FloatRect &caret, const Color &color,
                            const LayoutPoint &) const noexcept
  {
    context.FillRect(caret, color);
  }

  LayoutRect CaretAnimator::CaretRepaintRectForLocalRect(LayoutRect rect) const noexcept
  {
    return rect;
  }
}
