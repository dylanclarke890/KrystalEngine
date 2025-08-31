#pragma once

#include "Krystal.Maths/Vector.hpp"
#include "Krystal.Maths/Matrix.hpp"

namespace Krys::Gfx
{
  /// @brief Interface for a camera in 3D space.
  class ICamera
  {
  public:
    virtual ~ICamera() noexcept = default;

    /// @brief Gets the view matrix of the camera.
    /// @return The view matrix.
    virtual const Maths::Mat4 &ViewMatrix() const noexcept = 0;
    
    /// @brief Gets the projection matrix of the camera.
    /// @return The projection matrix.
    virtual const Maths::Mat4 &ProjectionMatrix() const noexcept = 0;
    
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
  };
}