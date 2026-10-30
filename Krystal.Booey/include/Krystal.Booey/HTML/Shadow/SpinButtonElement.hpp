#pragma once

#include "Krystal.Booey/HTML/HTMLDivElement.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"

namespace krys::boo::html
{
  /// @brief What a spin button steps: the input type of the field it's in.
  class SpinButtonOwner
  {
  public:
    virtual ~SpinButtonOwner() noexcept = default;
    virtual void FocusAndSelectSpinButtonOwner() noexcept = 0;
    KRYS_NODISCARD virtual bool ShouldSpinButtonRespondToMouseEvents() const noexcept = 0;
    virtual void SpinButtonStepDown() noexcept = 0;
    virtual void SpinButtonStepUp() noexcept = 0;
  };

  /// @brief A number field's spin button (the `::-webkit-inner-spin-button` part): its upper half steps up
  /// and its lower half down, repeating while held.
  class SpinButtonElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SpinButtonElement);

  public:
    enum class UpDownState : uint8
    {
      Indeterminate, // Hovered, but the event is not handled.
      Down,
      Up,
    };

  private:
    // The owner clears this when it goes (RemoveSpinButtonOwner).
    SpinButtonOwner *_spinButtonOwner;
    bool _capturing {false};
    UpDownState _upDownState {UpDownState::Indeterminate};
    UpDownState _pressStartingState {UpDownState::Indeterminate};
    Timer _repeatingTimer;

  public:
    SpinButtonElement(dom::Document &, SpinButtonOwner &) noexcept;

    // The owner of SpinButtonElement must call removeSpinButtonOwner
    // because SpinButtonElement can be outlive SpinButtonOwner
    // implementation, e.g. during event handling.
    KRYS_NODISCARD static Ref<SpinButtonElement> Create(dom::Document &, SpinButtonOwner &) noexcept;

    KRYS_NODISCARD UpDownState GetUpDownState() const noexcept
    {
      return _upDownState;
    }

    void ReleaseCapture() noexcept;
    void RemoveSpinButtonOwner() noexcept
    {
      _spinButtonOwner = nullptr;
    }

    void Step(int32 amount) noexcept;

    KRYS_NODISCARD bool IsSpinButtonElement() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept override;
    KRYS_NODISCARD bool MatchesReadWritePseudoClass() const noexcept override;
    void DefaultEventHandler(dom::Event &) noexcept override;

    void SetHovered(bool, style::InvalidationScope = style::InvalidationScope::All,
                    HitTestRequest = {}) noexcept override;

  private:
    // FIXME: Releasing the capture when a popup opens (PopupOpeningObserver::willOpenPopup), and when the
    // renderer is detached (willDetachRenderers), once there's a hook.
    void DoStepAction(int32) noexcept;
    void StartRepeatingTimer() noexcept;
    void StopRepeatingTimer() noexcept;
    void RepeatingTimerFired() noexcept;

    KRYS_NODISCARD bool ShouldRespondToMouseEvents() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::SpinButtonElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &element) noexcept
  {
    return element.IsSpinButtonElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsHTMLElement() && IsType(Downcast<krys::boo::html::HTMLElement>(node));
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
