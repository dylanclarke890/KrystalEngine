#pragma once

#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.Platform/Input.hpp"

namespace Krys::Gfx
{
  /// @brief Interface for a camera in 3D space.
  class ICamera
  {
  protected:
    ICamera() noexcept = default;

  public:
    virtual ~ICamera() noexcept = default;

    /// @brief Gets the view matrix of the camera.
    /// @return The view matrix.
    virtual Maths::Mat4 ViewMatrix() const noexcept = 0;

    /// @brief Gets the projection matrix of the camera.
    /// @return The projection matrix.
    virtual Maths::Mat4 ProjectionMatrix() const noexcept = 0;

    /// @brief Gets the position of the camera in world space.
    /// @return The position of the camera.
    virtual const Maths::Vec3 &Position() const noexcept = 0;

    /// @brief Gets the forward direction of the camera.
    /// @return The forward direction of the camera.
    virtual const Maths::Vec3 &Forward() const noexcept = 0;

    /// @brief Gets the up direction of the camera.
    /// @return The up direction of the camera.
    virtual const Maths::Vec3 &Up() const noexcept = 0;

    /// @brief Gets the right direction of the camera.
    /// @return The right direction of the camera.
    virtual const Maths::Vec3 &Right() const noexcept = 0;

    /// @brief Notifies the camera that the viewport has been resized.
    virtual void OnResize(uint32 width, uint32 height) noexcept = 0;

    /// @brief Updates the camera's position and orientation based on input.
    /// @param deltaTime The time elapsed since the last frame in seconds.
    /// @param input The current input state.
    virtual void Update(double deltaTime, const Platform::Input &input) noexcept = 0;
  };
}