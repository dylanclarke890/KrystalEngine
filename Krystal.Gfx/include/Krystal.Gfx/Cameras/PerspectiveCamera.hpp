#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx/ICamera.hpp"
#include "Krystal.Maths/Clipspace.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Transform.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  /// @brief A perspective camera implementation.
  class PerspectiveCamera : public ICamera
  {
  private:
    Maths::Mat4 _viewMatrix {};
    Maths::Mat4 _projectionMatrix {};
    Maths::Vec3 _position {};
    Maths::Vec3 _forward {};
    Maths::Vec3 _up {};
    Maths::Vec3 _right {};

  public:
    virtual ~PerspectiveCamera() noexcept = default;

    PerspectiveCamera(const Maths::Vec3 &position, const Maths::Vec3 &target, const Maths::Vec3 &up,
                      float fovY, float aspect, float nearPlane, float farPlane) noexcept;

    /// @brief Gets the view matrix of the camera.
    /// @return The view matrix.
    const Maths::Mat4 &ViewMatrix() const noexcept override;

    /// @brief Gets the projection matrix of the camera.
    /// @return The projection matrix.
    const Maths::Mat4 &ProjectionMatrix() const noexcept override;

    /// @brief Gets the position of the camera in world space.
    /// @return The position of the camera.
    const Maths::Vec3 &Position() const noexcept override;

    /// @brief Gets the forward direction of the camera.
    /// @return The forward direction of the camera.
    const Maths::Vec3 &Forward() const noexcept override;

    /// @brief Gets the up direction of the camera.
    /// @return The up direction of the camera.
    const Maths::Vec3 &Up() const noexcept override;

    /// @brief Gets the right direction of the camera.
    /// @return The right direction of the camera.
    const Maths::Vec3 &Right() const noexcept override;
  };
}
