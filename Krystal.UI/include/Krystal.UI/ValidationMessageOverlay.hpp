#pragma once

#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "Krystal.UI/Overlay.hpp"

namespace krys::ui
{
  /// @brief A form control's validation message (the constraint it doesn't satisfy), shown by the view of its
  /// page as a bubble below the control (or above it, if there's more room there) when `reportValidity()` or
  /// a submission finds it invalid. It isn't modal: the control keeps the keyboard, and typing, Escape,
  /// losing the focus or a press anywhere hides it, as does a few seconds passing, longer for a longer
  /// message.
  class ValidationMessageOverlay final : public RefCounted<ValidationMessageOverlay>, public Overlay
  {
  private:
    View *_owner {nullptr};
    UniquePtr<View> _view;
    WeakPtr<boo::dom::Element> _anchor;
    utf8_string _message;
    boo::IntRect _rect;
    boo::Timer _hideTimer {*this, &ValidationMessageOverlay::Hide};
    bool _visible {false};

  public:
    explicit ValidationMessageOverlay(View &owner) noexcept;
    ~ValidationMessageOverlay() noexcept override;

    /// @brief Shows @p message at @p anchor, in place of the message shown.
    void Show(boo::dom::Element &anchor, utf8_stringview message) noexcept;

    /// @brief Forgets the owner, which is going away.
    void Detach() noexcept;

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return _visible;
    }

    /// @brief Whether the message is shown at @p anchor.
    KRYS_NODISCARD bool IsShownAt(const boo::dom::Element &anchor) const noexcept
    {
      return _visible && _anchor.get() == &anchor;
    }

    KRYS_NODISCARD const utf8_string &Message() const noexcept
    {
      return _message;
    }

#pragma region Overlay

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

    KRYS_NODISCARD View &ContentView() const noexcept final
    {
      return *_view;
    }

    KRYS_NODISCARD const boo::IntRect &Rect() const noexcept final
    {
      return _rect;
    }

    KRYS_NODISCARD bool IsModal() const noexcept final
    {
      return false;
    }

    void Hide() noexcept final;

    void MouseMoved(const boo::IntPoint &) noexcept final
    {
    }

    /// @brief A press on the bubble hides it.
    void MouseReleased(const boo::IntPoint &point) noexcept final;
    bool MouseWheel(const boo::IntPoint &, float, float) noexcept final
    {
      return false;
    }

    /// @brief The control takes the keys.
    bool KeyPressed(pal::Key, pal::KeyModifiers) noexcept final
    {
      return false;
    }

#pragma endregion
  };
}
