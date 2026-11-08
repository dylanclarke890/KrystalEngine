#include "Krystal.UI/ValidationMessageOverlay.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.UI/View.hpp"
#include <algorithm>
#include <chrono>

namespace krys::ui
{
  namespace
  {
    /// @brief The widest the bubble gets before its message wraps.
    constexpr int32 MaximumValidationMessageWidth = 300;

    /// @brief How long the bubble shows @p message for: five seconds, or a twentieth of a second a
    /// character for a longer one, as in Chrome.
    KRYS_NODISCARD MonotonicTime::duration ValidationMessageDuration(utf8_stringview message) noexcept
    {
      using namespace std::chrono_literals;
      auto duration = std::chrono::duration<double>(static_cast<double>(message.size()) * 0.05);
      return std::chrono::duration_cast<MonotonicTime::duration>(
        std::max<std::chrono::duration<double>>(duration, 5s));
    }
  }

  ValidationMessageOverlay::ValidationMessageOverlay(View &owner) noexcept : _owner(&owner)
  {
  }

  ValidationMessageOverlay::~ValidationMessageOverlay() noexcept = default;

  void ValidationMessageOverlay::Detach() noexcept
  {
    Hide();
    _owner = nullptr;
  }

  void ValidationMessageOverlay::Show(boo::dom::Element &anchor, utf8_stringview message) noexcept
  {
    if (!_owner)
    {
      return;
    }

    // The bubble goes where the anchor is once focusing it has scrolled it into view.
    auto &document = anchor.NodeDocument();
    (void)document.UpdateLayout();
    auto *renderer = anchor.Renderer();
    auto *frameView = document.View();
    if (!renderer || !frameView)
    {
      Hide();
      return;
    }
    auto anchorRect = frameView->ContentsToView(renderer->AbsoluteBoundingBoxRect());
    bool rightToLeft = !renderer->Style().IsLeftToRightDirection();

    _anchor = CreateWeakPtr(&anchor);
    _message = utf8_string(message);

    utf8_string html = rightToLeft ? u8"<!DOCTYPE html><html dir=rtl><style>" : u8"<!DOCTYPE html><style>";
    html += u8"html { width: max-content; max-width: 300px; box-sizing: border-box; margin: 0;"
            u8" border: 1px solid GrayText; border-radius: 4px; cursor: default; background: Canvas;"
            u8" color: CanvasText; font: 13px sans-serif; }\n"
            u8"body { margin: 0; padding: 6px 8px; display: flex; gap: 6px; align-items: start; }\n"
            u8".icon { flex: none; width: 16px; height: 16px; border-radius: 2px; background: #f9ab00;"
            u8" color: white; font-weight: bold; text-align: center; line-height: 16px; }\n"
            u8".message { white-space: pre-wrap; overflow-wrap: anywhere; }\n"
            u8"</style><body><span class=icon>!</span><span class=message>";
    AppendEscaped(html, message);
    html += u8"</span>";

    auto viewport = _owner->ViewportSize();
    boo::IntSize layoutSize(std::min(MaximumValidationMessageWidth, std::max(viewport.Width(), 1)),
                            std::max(viewport.Height(), 1));
    if (!_view)
    {
      _view = CreateUnique<View>(layoutSize);
    }
    else
    {
      _view->Resize(layoutSize);
    }
    ReplaceDocument(*_view, krys::move(html));
    (void)_view->Document().UpdateLayout();

    boo::IntSize contentSize(1, 1);
    if (auto *root = _view->Document().DocumentElement().get())
    {
      if (auto *box = DynamicDowncast<boo::RenderBox>(root->Renderer()))
      {
        contentSize = boo::IntSize(std::max(box->Width().ToInt(), 1), std::max(box->Height().ToInt(), 1));
      }
    }
    _rect = Place(anchorRect, contentSize, viewport, rightToLeft);
    _view->Resize(_rect.Size());

    _hideTimer.StartOneShot(ValidationMessageDuration(message));
    if (!_visible)
    {
      _visible = true;
      ShowIn(*_owner, *this);
    }
  }

  void ValidationMessageOverlay::Hide() noexcept
  {
    _hideTimer.Stop();
    if (!_visible)
    {
      return;
    }

    // The owner lets go of the bubble, which its chrome client still holds.
    Ref<ValidationMessageOverlay> protectedThis = ShareRef(*this);
    _visible = false;
    _anchor = nullptr;
    _message.clear();
    if (_owner)
    {
      HiddenFrom(*_owner, *this);
    }
  }

  void ValidationMessageOverlay::MouseReleased(const boo::IntPoint &) noexcept
  {
    Hide();
  }
}
