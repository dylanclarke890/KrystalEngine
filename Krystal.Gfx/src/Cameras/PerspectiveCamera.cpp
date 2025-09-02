#include "Krystal.Gfx/Cameras/PerspectiveCamera.hpp"

#include "Krystal.Maths/Convert.hpp"

namespace Krys::Gfx
{
  PerspectiveCamera::PerspectiveCamera(const Maths::Vec3 &position, const Maths::Vec3 &target,
                                       const Maths::Vec3 &up, float fovY, float aspect, float nearPlane,
                                       float farPlane) noexcept
      : _position(position), _up(up)
  {
    using namespace Maths;

    _forward = Normalize(target - position);
    _right = Normalize(Cross(_forward, _up));
    _up = Normalize(Cross(_right, _forward));
    _viewMatrix = LookAt(position, target, _up);
    _projectionMatrix = Perspective(fovY, aspect, nearPlane, farPlane);
  }

  const Maths::Mat4 &PerspectiveCamera::ViewMatrix() const noexcept
  {
    return _viewMatrix;
  }

  const Maths::Mat4 &PerspectiveCamera::ProjectionMatrix() const noexcept
  {
    return _projectionMatrix;
  }

  const Maths::Vec3 &PerspectiveCamera::Position() const noexcept
  {
    return _position;
  }

  const Maths::Vec3 &PerspectiveCamera::Forward() const noexcept
  {
    return _forward;
  }

  const Maths::Vec3 &PerspectiveCamera::Up() const noexcept
  {
    return _up;
  }

  const Maths::Vec3 &PerspectiveCamera::Right() const noexcept
  {
    return _right;
  }

  void PerspectiveCamera::Update(double deltaTime, const Platform::Input& input) noexcept
  {
    using namespace Platform;
    using namespace Maths;

    float cameraSpeed = 0.05f;

    const auto &keyboard = input.GetKeyboard();
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
      _position -= Normalize(Cross(_forward, _up)) * cameraSpeed;
    }

    if (keyboard.IsKeyHeld(Key::D))
    {
      _position += Normalize(Cross(_forward, _up)) * cameraSpeed;
    }

    float lookSensitivity = 0.1f;

    const auto &mouse = input.GetMouse();
    if (mouse.IsButtonHeld(MouseButton::Right))
    {
      auto deltaX = mouse.DeltaX();
      auto deltaY = mouse.DeltaY();

      deltaX *= lookSensitivity;
      deltaY *= lookSensitivity;

      _yaw += deltaX;
      _pitch += deltaY;

      if (_pitch > 89.0f)
        _pitch = 89.0f;
      if (_pitch < -89.0f)
        _pitch = -89.0f;

      Vec3 direction {};
      direction.x = std::cos(Radians(_yaw)) * std::cos(Radians(_pitch));
      direction.y = std::sin(Radians(_pitch));
      direction.z = std::sin(Radians(_yaw)) * std::cos(Radians(_pitch));
      _forward = Normalize(direction);
    }

    _viewMatrix = LookAt(_position, _position + _forward, _up);
  }

  void PerspectiveCamera::OnResize(uint32 width, uint32 height) noexcept
  {
    using namespace Maths;
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    _projectionMatrix = Perspective(Radians(45.0f), aspect, 0.1f, 100.0f);
  }
}