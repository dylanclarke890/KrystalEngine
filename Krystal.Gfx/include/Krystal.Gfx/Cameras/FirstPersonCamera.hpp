#pragma once

#include "Krystal.Core/Maths/Clipspace.hpp"
#include "Krystal.Core/Maths/Matrix.hpp"
#include "Krystal.Core/Maths/Transform.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Gfx/ICamera.hpp"

namespace krys::Gfx
{
  class FirstPersonCamera : public ICamera
  {
  private:
    Vec3 _position {};
    Vec3 _forward {};
    Vec3 _up {};
    Vec3 _right {};
    float _fovY {0.0f};
    float _aspect {1.0f};
    float _nearPlane {0.1f};
    float _farPlane {100.f};
    float _yaw {-90.0f}; // Yaw is initialized to -90.0 degrees to look along the negative Z axis
    float _pitch {0.0f};
    bool _fixedYPosition {false};

  public:
    virtual ~FirstPersonCamera() noexcept = default;

    FirstPersonCamera(const Vec3 &position, const Vec3 &target, const Vec3 &up, float fovY, float aspect,
                      float nearPlane, float farPlane) noexcept;

    /// @brief Gets the view matrix of the camera.
    /// @return The view matrix.
    Mat4 ViewMatrix() const noexcept override;

    /// @brief Gets the projection matrix of the camera.
    /// @return The projection matrix.
    Mat4 ProjectionMatrix() const noexcept override;

    /// @brief Gets the position of the camera in world space.
    /// @return The position of the camera.
    const Vec3 &Position() const noexcept override;

    /// @brief Gets the forward direction of the camera.
    /// @return The forward direction of the camera.
    const Vec3 &Forward() const noexcept override;

    /// @brief Gets the up direction of the camera.
    /// @return The up direction of the camera.
    const Vec3 &Up() const noexcept override;

    /// @brief Gets the right direction of the camera.
    /// @return The right direction of the camera.
    const Vec3 &Right() const noexcept override;

    /// @brief Notifies the camera that the viewport has been resized.
    void OnResize(uint32 width, uint32 height) noexcept override;

    /// @brief Updates the camera's position and orientation based on input.
    /// @param deltaTime The time elapsed since the last frame in seconds.
    /// @param input The current input state.
    virtual void Update(double deltaTime, const pal::Input &input) noexcept override;

  private:
    void UpdateCameraVectors() noexcept;
  };
}
