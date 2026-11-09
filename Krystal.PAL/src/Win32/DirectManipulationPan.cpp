#include "Krystal.PAL/Win32/DirectManipulationPan.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::pal::win32
{
  namespace
  {
    /// @brief The viewport's size: any will do, as it stands in for no content.
    constexpr LONG ViewportSize = 1'000;

    /// @brief How long the timer updates an idle viewport for, after a contact that hasn't panned (a tap).
    constexpr uint32 MaximumIdleUpdates = 30;

    /// @brief How often the timer updates the viewport, in milliseconds: about once a frame.
    constexpr UINT UpdateInterval = 16;

    template <typename T>
    void ReleaseInterface(T *&object) noexcept
    {
      if (object)
      {
        object->Release();
        object = nullptr;
      }
    }
  }

  DirectManipulationPan::DirectManipulationPan(HWND window, Handler handler) noexcept
      : _window(window), _handler(krys::move(handler))
  {
  }

  DirectManipulationPan::~DirectManipulationPan() noexcept = default;

  DirectManipulationPan *DirectManipulationPan::Create(HWND window, Handler handler) noexcept
  {
    auto *pan = new DirectManipulationPan(window, krys::move(handler));
    if (!pan->Initialise())
    {
      pan->Destroy();
      return nullptr;
    }
    return pan;
  }

  bool DirectManipulationPan::Initialise() noexcept
  {
    // COM is initialised with OLE (`pal::Initialise()`).
    if (FAILED(::CoCreateInstance(__uuidof(DirectManipulationManager), nullptr, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&_manager))))
    {
      _manager = nullptr;
      return false;
    }
    if (FAILED(_manager->GetUpdateManager(IID_PPV_ARGS(&_updateManager))))
    {
      _updateManager = nullptr;
      return false;
    }
    if (FAILED(_manager->CreateViewport(nullptr, _window, IID_PPV_ARGS(&_viewport))))
    {
      _viewport = nullptr;
      return false;
    }

    // Pans on either axis (or the one they start along), coasting after a flick. Pinches don't zoom.
    constexpr auto configuration = static_cast<DIRECTMANIPULATION_CONFIGURATION>(
      DIRECTMANIPULATION_CONFIGURATION_INTERACTION | DIRECTMANIPULATION_CONFIGURATION_TRANSLATION_X
      | DIRECTMANIPULATION_CONFIGURATION_TRANSLATION_Y | DIRECTMANIPULATION_CONFIGURATION_TRANSLATION_INERTIA
      | DIRECTMANIPULATION_CONFIGURATION_RAILS_X | DIRECTMANIPULATION_CONFIGURATION_RAILS_Y);
    // The viewport is updated by the window's timer, as there's no compositor for it.
    RECT rect {0, 0, ViewportSize, ViewportSize};
    if (FAILED(_viewport->ActivateConfiguration(configuration))
        || FAILED(_viewport->SetViewportOptions(DIRECTMANIPULATION_VIEWPORT_OPTIONS_MANUALUPDATE))
        || FAILED(_viewport->AddEventHandler(_window, this, &_handlerCookie))
        || FAILED(_viewport->SetViewportRect(&rect)) || FAILED(_manager->Activate(_window))
        || FAILED(_viewport->Enable()))
    {
      return false;
    }
    (void)_updateManager->Update(nullptr);
    return true;
  }

  void DirectManipulationPan::Destroy() noexcept
  {
    StopUpdating();
    if (_viewport)
    {
      (void)_viewport->Stop();
      if (_handlerCookie)
      {
        (void)_viewport->RemoveEventHandler(_handlerCookie);
        _handlerCookie = 0;
      }
      (void)_viewport->Abandon();
    }
    if (_manager)
    {
      (void)_manager->Deactivate(_window);
    }
    ReleaseInterface(_viewport);
    ReleaseInterface(_updateManager);
    ReleaseInterface(_manager);
    Release();
  }

  void DirectManipulationPan::SetContact(UINT32 pointerId) noexcept
  {
    if (_viewport && SUCCEEDED(_viewport->SetContact(pointerId)))
    {
      StartUpdating();
    }
  }

  void DirectManipulationPan::Update() noexcept
  {
    if (!_updateManager)
    {
      return;
    }
    (void)_updateManager->Update(nullptr);

    // A contact that doesn't pan leaves the viewport ready: the timer stops after a while.
    DIRECTMANIPULATION_STATUS status {};
    if (_state == PanState::None && SUCCEEDED(_viewport->GetStatus(&status))
        && status == DIRECTMANIPULATION_READY && ++_idleUpdates > MaximumIdleUpdates)
    {
      StopUpdating();
    }
  }

  void DirectManipulationPan::StartUpdating() noexcept
  {
    _idleUpdates = 0;
    if (!_updating)
    {
      _updating = ::SetTimer(_window, TimerId, UpdateInterval, nullptr) != 0;
    }
  }

  void DirectManipulationPan::StopUpdating() noexcept
  {
    if (_updating)
    {
      (void)::KillTimer(_window, TimerId);
      _updating = false;
    }
  }

  void DirectManipulationPan::Reset() noexcept
  {
    // The content update this makes matches the last offsets, so it isn't reported.
    _lastX = 0;
    _lastY = 0;
    (void)_viewport->ZoomToRect(0, 0, static_cast<float>(ViewportSize), static_cast<float>(ViewportSize),
                                FALSE);
  }

  HRESULT DirectManipulationPan::QueryInterface(REFIID id, void **object) noexcept
  {
    if (!object)
    {
      return E_POINTER;
    }
    if (id == IID_IUnknown || id == __uuidof(IDirectManipulationViewportEventHandler))
    {
      *object = static_cast<IDirectManipulationViewportEventHandler *>(this);
      AddRef();
      return S_OK;
    }
    *object = nullptr;
    return E_NOINTERFACE;
  }

  ULONG DirectManipulationPan::AddRef() noexcept
  {
    return ++_refCount;
  }

  ULONG DirectManipulationPan::Release() noexcept
  {
    const ULONG count = --_refCount;
    if (!count)
    {
      delete this;
    }
    return count;
  }

  HRESULT DirectManipulationPan::OnViewportStatusChanged(IDirectManipulationViewport *,
                                                         DIRECTMANIPULATION_STATUS current,
                                                         DIRECTMANIPULATION_STATUS previous) noexcept
  {
    if (current == previous)
    {
      return S_OK;
    }

    switch (current)
    {
      case DIRECTMANIPULATION_INERTIA:
        // The fingers flicked as they lifted.
        if (_state == PanState::Panning)
        {
          _state = PanState::Coasting;
          _handler(TouchpadPanPhase::MomentumBegan, 0, 0);
        }
        break;
      case DIRECTMANIPULATION_RUNNING:
        // A new touch stops the coasting; its own pan begins when it moves.
        if (_state == PanState::Coasting)
        {
          _state = PanState::None;
          _handler(TouchpadPanPhase::MomentumEnded, 0, 0);
        }
        break;
      case DIRECTMANIPULATION_READY:
        // The pan is over.
        if (_state == PanState::Panning)
        {
          _handler(TouchpadPanPhase::Ended, 0, 0);
        }
        else if (_state == PanState::Coasting)
        {
          _handler(TouchpadPanPhase::MomentumEnded, 0, 0);
        }
        _state = PanState::None;
        if (_lastX || _lastY)
        {
          Reset();
        }
        break;
      default: break;
    }
    return S_OK;
  }

  HRESULT DirectManipulationPan::OnViewportUpdated(IDirectManipulationViewport *) noexcept
  {
    return S_OK;
  }

  HRESULT DirectManipulationPan::OnContentUpdated(IDirectManipulationViewport *,
                                                  IDirectManipulationContent *content) noexcept
  {
    // A 2D affine transform: the offsets are its last two values.
    float transform[6] {};
    if (!content || FAILED(content->GetContentTransform(transform, ARRAYSIZE(transform))))
    {
      return S_OK;
    }

    float x = transform[4];
    float y = transform[5];
    if (x == _lastX && y == _lastY)
    {
      return S_OK;
    }
    float deltaX = x - _lastX;
    float deltaY = y - _lastY;
    _lastX = x;
    _lastY = y;

    switch (_state)
    {
      case PanState::None:
        _state = PanState::Panning;
        _handler(TouchpadPanPhase::Began, deltaX, deltaY);
        break;
      case PanState::Panning:  _handler(TouchpadPanPhase::Changed, deltaX, deltaY); break;
      case PanState::Coasting: _handler(TouchpadPanPhase::MomentumChanged, deltaX, deltaY); break;
    }
    return S_OK;
  }
}
