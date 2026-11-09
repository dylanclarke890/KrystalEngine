#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.PAL/TouchpadPan.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <directmanipulation.h>

namespace krys::pal::win32
{
  /// @brief A window's DirectManipulation viewport: it takes a precision touchpad's contacts and reports
  /// their two-finger pans, with the momentum the system gives a flick, in place of the wheel messages the
  /// system would make of them.
  /// @note The viewport stands in for no content: only how far it moves is reported, and it goes back to
  /// where it started after each pan. A timer on the window updates it while a contact may be panning.
  class DirectManipulationPan final : public IDirectManipulationViewportEventHandler
  {
    KRYS_NON_COPY_MOVABLE(DirectManipulationPan);

  public:
    /// @brief The phase and the content's movement (as `TouchpadPanInput`'s) of each step of a pan.
    using Handler = Func<void(TouchpadPanPhase, float, float)>;
    /// @brief The id of the window's timer that updates the viewport.
    static constexpr UINT_PTR TimerId = 0x4B504E;

  private:
    /// @brief What the content is doing: nothing yet, following the fingers, or coasting after a flick.
    enum class PanState : uint8
    {
      None,
      Panning,
      Coasting,
    };

    ULONG _refCount {1};
    HWND _window {nullptr};
    Handler _handler;
    IDirectManipulationManager *_manager {nullptr};
    IDirectManipulationUpdateManager *_updateManager {nullptr};
    IDirectManipulationViewport *_viewport {nullptr};
    DWORD _handlerCookie {0};
    PanState _state {PanState::None};
    /// @brief The content's offset at the last step, from where the viewport started.
    float _lastX {0};
    float _lastY {0};
    /// @brief Whether the timer runs, and for how many of its updates the viewport has been idle.
    bool _updating {false};
    uint32 _idleUpdates {0};

  public:
    /// @brief Makes the viewport for @p window, which then takes touchpad contacts.
    /// @return The viewport, holding one reference for the window (see `Destroy()`); or nothing if
    /// DirectManipulation isn't there, when the touchpad's pans come as the wheel.
    KRYS_NODISCARD static DirectManipulationPan *Create(HWND window, Handler handler) noexcept;

    /// @brief Gives the viewport the touchpad contact @p pointerId, from the window's `DM_POINTERHITTEST`.
    void SetContact(UINT32 pointerId) noexcept;
    /// @brief Updates the viewport, which reports the steps of a pan; at the window's timer.
    void Update() noexcept;
    /// @brief Stops taking contacts, before the window goes, and releases the window's reference.
    void Destroy() noexcept;

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void **object) noexcept override;
    ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ULONG STDMETHODCALLTYPE Release() noexcept override;

    HRESULT STDMETHODCALLTYPE OnViewportStatusChanged(IDirectManipulationViewport *viewport,
                                                      DIRECTMANIPULATION_STATUS current,
                                                      DIRECTMANIPULATION_STATUS previous) noexcept override;
    HRESULT STDMETHODCALLTYPE OnViewportUpdated(IDirectManipulationViewport *viewport) noexcept override;
    HRESULT STDMETHODCALLTYPE OnContentUpdated(IDirectManipulationViewport *viewport,
                                               IDirectManipulationContent *content) noexcept override;

  private:
    DirectManipulationPan(HWND window, Handler handler) noexcept;
    ~DirectManipulationPan() noexcept;

    KRYS_NODISCARD bool Initialise() noexcept;
    void StartUpdating() noexcept;
    void StopUpdating() noexcept;
    /// @brief Puts the viewport back where it started, for the next pan.
    void Reset() noexcept;
  };
}
