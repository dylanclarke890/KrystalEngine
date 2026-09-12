#include "Krystal.Gfx/Cameras/FirstPersonCamera.hpp"
#include "Krystal.Core/Maths/Clamp.hpp"
#include "Krystal.Core/Maths/Convert.hpp"

namespace krys::Gfx
{
  // TODO(feat): we need to be able to lock the cursor to the center of the window when looking around
  FirstPersonCamera::FirstPersonCamera(const Vec3 &position, const Vec3 &target, const Vec3 &up, float fovY,
                                       float aspect, float nearPlane, float farPlane) noexcept
      : _position(position), _up(up), _fovY(fovY), _aspect(aspect), _nearPlane(nearPlane), _farPlane(farPlane)
  {
    _forward = Normalize(Vec3(0.0f, 0.0f, -1.0f));
    _right = Normalize(Cross(_forward, _up));
    _up = Normalize(Cross(_right, _forward));
  }

  Mat4 FirstPersonCamera::ViewMatrix() const noexcept
  {
    return LookAt(_position, _position + _forward, _up);
  }

  Mat4 FirstPersonCamera::ProjectionMatrix() const noexcept
  {
    return Perspective(_fovY, _aspect, _nearPlane, _farPlane);
  }

  const Vec3 &FirstPersonCamera::Position() const noexcept
  {
    return _position;
  }

  const Vec3 &FirstPersonCamera::Forward() const noexcept
  {
    return _forward;
  }

  const Vec3 &FirstPersonCamera::Up() const noexcept
  {
    return _up;
  }

  const Vec3 &FirstPersonCamera::Right() const noexcept
  {
    return _right;
  }

  void FirstPersonCamera::Update(double deltaTime, const pal::Input &input) noexcept
  {
    using namespace pal;

    float cameraSpeed = 50.f * static_cast<float>(deltaTime);

    const auto &keyboard = input.GetKeyboard();

    if (keyboard.IsKeyPressed(Key::F))
    {
      _fixedYPosition = !_fixedYPosition;
    }

    const float yBefore = _position.y;
    if (keyboard.IsKeyHeld(Key::W))
    {
      _position += _forward * cameraSpeed;
    }
    if (keyboard.IsKeyHeld(Key::S))
    {
      _position -= _forward * cameraSpeed;
    }
    if (keyboard.IsKeyHeld(Key::A))
    {
      _position -= _right * cameraSpeed;
    }
    if (keyboard.IsKeyHeld(Key::D))
    {
      _position += _right * cameraSpeed;
    }
    if (_fixedYPosition)
    {
      _position.y = yBefore;
    }

    float lookSensitivity = 0.1f;
    const auto &mouse = input.GetMouse();
    if (mouse.IsButtonHeld(MouseButton::Left))
    {
      auto deltaX = mouse.DeltaX();
      auto deltaY = -mouse.DeltaY();

      deltaX *= lookSensitivity;
      deltaY *= lookSensitivity;

      _yaw += deltaX;
      _pitch += deltaY;
      _pitch = Clamp(_pitch, -89.0f, 89.0f);

      UpdateCameraVectors();
    }

    auto scrollDelta = mouse.ScrollDelta();
    if (scrollDelta != 0.0)
    {
      _fovY -= (float)mouse.ScrollDelta();
      _fovY = Clamp(_fovY, 1.0f, 45.0f);
    }
  }

  void FirstPersonCamera::OnResize(uint32 width, uint32 height) noexcept
  {
    _aspect = static_cast<float>(width) / static_cast<float>(height);
  }

  void FirstPersonCamera::UpdateCameraVectors() noexcept
  {
    Vec3 front {};
    front.x = std::cos(Radians(_yaw)) * std::cos(Radians(_pitch));
    front.y = std::sin(Radians(_pitch));
    front.z = std::sin(Radians(_yaw)) * std::cos(Radians(_pitch));
    _forward = Normalize(front);
    _right = Normalize(Cross(_forward, Vec3(0.0f, 1.0f, 0.0f)));
    _up = Normalize(Cross(_right, _forward));
  }
}