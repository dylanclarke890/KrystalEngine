#include "Krystal.Gfx/Cameras/FirstPersonCamera.hpp"

#include "Krystal.Maths/Convert.hpp"
#include "Krystal.Maths/Clamp.hpp"

namespace Krys::Gfx
{
  FirstPersonCamera::FirstPersonCamera(const Maths::Vec3 &position, const Maths::Vec3 &target,
                                       const Maths::Vec3 &up, float fovY, float aspect, float nearPlane,
                                       float farPlane) noexcept
      : _position(position), _up(up), _fovY(fovY), _aspect(aspect)
  {
    using namespace Maths;

    _forward = Normalize(target - position);
    _right = Normalize(Cross(_forward, _up));
    _up = Normalize(Cross(_right, _forward));
    _viewMatrix = LookAt(position, target, _up);
    _projectionMatrix = Perspective(_fovY, _aspect, nearPlane, farPlane);
  }

  const Maths::Mat4 &FirstPersonCamera::ViewMatrix() const noexcept
  {
    return _viewMatrix;
  }

  const Maths::Mat4 &FirstPersonCamera::ProjectionMatrix() const noexcept
  {
    return _projectionMatrix;
  }

  const Maths::Vec3 &FirstPersonCamera::Position() const noexcept
  {
    return _position;
  }

  const Maths::Vec3 &FirstPersonCamera::Forward() const noexcept
  {
    return _forward;
  }

  const Maths::Vec3 &FirstPersonCamera::Up() const noexcept
  {
    return _up;
  }

  const Maths::Vec3 &FirstPersonCamera::Right() const noexcept
  {
    return _right;
  }

  void FirstPersonCamera::Update(double deltaTime, const Platform::Input &input) noexcept
  {
    using namespace Platform;
    using namespace Maths;

    float cameraSpeed = 5.f * static_cast<float>(deltaTime);

    const auto &keyboard = input.GetKeyboard();

    if (keyboard.IsKeyPressed(Key::F))
    {
      _fixedYPosition = !_fixedYPosition;
    }
    auto yPosition = _position.y;
    
    if (keyboard.IsKeyHeld(Key::W))
    {
      _position += cameraSpeed * _forward;
    }
    if (keyboard.IsKeyHeld(Key::S))
    {
      _position -= cameraSpeed * _forward;
    }
    if (keyboard.IsKeyHeld(Key::A))
    {
      _position -= Normalize(Cross(_up, _forward)) * cameraSpeed;
    }
    if (keyboard.IsKeyHeld(Key::D))
    {
      _position += Normalize(Cross(_up, _forward)) * cameraSpeed;
    }

    if (_fixedYPosition)
      _position.y = yPosition;

    float lookSensitivity = 0.1f;
    const auto &mouse = input.GetMouse();
    if (mouse.IsButtonHeld(MouseButton::Left))
    {
      auto deltaX = mouse.DeltaX();
      auto deltaY = -mouse.DeltaY();

      deltaX *= lookSensitivity;
      deltaY *= lookSensitivity;

      _yaw -= deltaX;
      _pitch += deltaY;
      _pitch = Maths::Clamp(_pitch, -89.0f, 89.0f);

      Vec3 direction {};
      direction.x = std::cos(Radians(_yaw)) * std::cos(Radians(_pitch));
      direction.y = std::sin(Radians(_pitch));
      direction.z = std::sin(Radians(_yaw)) * std::cos(Radians(_pitch));
      _forward = Normalize(direction);
    }

    _viewMatrix = LookAt(_position, _position + _forward, _up);

    auto scrollDelta = mouse.ScrollDelta();
    if (scrollDelta != 0.0)
    {
      _fovY -= (float)mouse.ScrollDelta();
      _fovY = Maths::Clamp(_fovY, 1.0f, 45.0f);
      _projectionMatrix = Perspective(_fovY, _aspect, 0.1f, 100.0f);
    }
  }

  void FirstPersonCamera::OnResize(uint32 width, uint32 height) noexcept
  {
    using namespace Maths;
    _aspect = static_cast<float>(width) / static_cast<float>(height);
    _projectionMatrix = Perspective(_fovY, _aspect, 0.1f, 100.0f);
  }
}