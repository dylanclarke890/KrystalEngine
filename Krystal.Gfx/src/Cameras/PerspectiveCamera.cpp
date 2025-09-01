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

    float cameraSpeed = 0.05f; // Adjust accordingly

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

    if (keyboard.IsKeyHeld(Key::UpArrow))
    {
      _position += cameraSpeed * _up;
    }

    if (keyboard.IsKeyHeld(Key::DownArrow))
    {
      _position -= cameraSpeed * _up;
    }

    float sensitivity = 0.1f; // Adjust accordingly

    const auto &mouse = input.GetMouse();
    const auto deltaX = mouse.DeltaX();
    const auto deltaY = mouse.DeltaY();

    if (deltaX != 0 || deltaY != 0)
    {
      float yaw = Radians(deltaX * sensitivity);
      float pitch = Radians(deltaY * sensitivity);
      // Rotate around the up vector (yaw)
      Mat4 yawMatrix = Rotate(Mat4(1.0f), yaw, _up);
      _forward = Normalize(Vec3(yawMatrix * Vec4(_forward, 0.0f)));
      _right = Normalize(Cross(_forward, _up));
      // Rotate around the right vector (pitch)
      Mat4 pitchMatrix = Rotate(Mat4(1.0f), pitch, _right);
      Vec3 newForward = Normalize(Vec3(pitchMatrix * Vec4(_forward, 0.0f)));
      // Prevent flipping
      float pitchAngle = Degrees(Asin(newForward.y));
      if (pitchAngle > -89.0f && pitchAngle < 89.0f)
      {
        _forward = newForward;
        _up = Normalize(Cross(_right, _forward));
      }
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