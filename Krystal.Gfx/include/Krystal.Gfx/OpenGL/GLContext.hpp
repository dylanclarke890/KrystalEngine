#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Utils/NativeHandle.hpp"

namespace krys::gfx::opengl
{
  /// @brief The device's OpenGL 4.5 context. It's created on a hidden window of its own, so the device works
  /// without a visible window; swapchains attach real windows, which share the context.
  class GLContext
  {
    KRYS_NON_COPY_MOVABLE(GLContext);

  private:
    void *_hiddenWindow {nullptr};
    void *_hiddenSurface {nullptr};
    void *_context {nullptr};
    void *_currentSurface {nullptr};
    int _pixelFormat {0};
    bool _supportsSrgb {false};

  public:
    /// @brief Creates the context and makes it current on the calling thread.
    KRYS_NODISCARD static Expected<UniquePtr<GLContext>> Create(bool debug) noexcept;

    GLContext() noexcept = default;
    ~GLContext() noexcept;

    /// @brief Prepares a window for presenting and returns its drawing surface (an `HDC`).
    KRYS_NODISCARD Expected<NativeHandle> AttachWindow(NativeHandle window) noexcept;
    /// @brief Releases a surface from `AttachWindow`.
    void DetachWindow(NativeHandle window, NativeHandle surface) noexcept;

    /// @brief Makes the context current on a window's surface, or on the hidden window's if null.
    void MakeCurrent(NativeHandle surface) noexcept;
    /// @brief Sets a window surface's swap interval: 0 presents immediately, 1 waits for vertical sync.
    /// @note The interval applies to the surface, not the context, on some drivers; set it per window.
    void SetSwapInterval(NativeHandle surface, int interval) noexcept;
    /// @brief Presents a window's surface.
    void SwapBuffers(NativeHandle surface) noexcept;

    /// @brief Whether windows' default framebuffers can encode sRGB.
    KRYS_NODISCARD bool SupportsSrgb() const noexcept
    {
      return _supportsSrgb;
    }

  private:
    KRYS_NODISCARD Expected<void> Initialise(bool debug) noexcept;
  };
}
