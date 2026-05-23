#pragma once

#include "Krystal.Gfx/ICamera.hpp"
#include "Krystal.Maths/Clipspace.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Transform.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  class FirstPersonCamera : public ICamera
  {
  private:
    Maths::Vec3 _position {};
    Maths::Vec3 _forward {};
    Maths::Vec3 _up {};
    Maths::Vec3 _right {};
    float _fovY {0.0f};
    float _aspect {1.0f};
    float _nearPlane {0.1f};
    float _farPlane {100.f};
    float _yaw {-90.0f}; // Yaw is initialized to -90.0 degrees to look along the negative Z axis
    float _pitch {0.0f};
    bool _fixedYPosition {false};

  public:
    virtual ~FirstPersonCamera() noexcept = default;

    FirstPersonCamera(const Maths::Vec3 &position, const Maths::Vec3 &target, const Maths::Vec3 &up,
                      float fovY, float aspect, float nearPlane, float farPlane) noexcept;

    /// @brief Gets the view matrix of the camera.
    /// @return The view matrix.
    Maths::Mat4 ViewMatrix() const noexcept override;

    /// @brief Gets the projection matrix of the camera.
    /// @return The projection matrix.
    Maths::Mat4 ProjectionMatrix() const noexcept override;

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

    /// @brief Notifies the camera that the viewport has been resized.
    void OnResize(uint32 width, uint32 height) noexcept override;

    /// @brief Updates the camera's position and orientation based on input.
    /// @param deltaTime The time elapsed since the last frame in seconds.
    /// @param input The current input state.
    virtual void Update(double deltaTime, const Platform::Input &input) noexcept override;

    private:
    void UpdateCameraVectors() noexcept;
  };
}
