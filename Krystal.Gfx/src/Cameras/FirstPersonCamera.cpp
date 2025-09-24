#include "Krystal.Gfx/Cameras/FirstPersonCamera.hpp"

#include "Krystal.Maths/Clamp.hpp"
#include "Krystal.Maths/Convert.hpp"

namespace Krys::Gfx
{
  // TODO: we need to be able to lock the cursor to the center of the window when looking around
  FirstPersonCamera::FirstPersonCamera(const Maths::Vec3 &position, const Maths::Vec3 &target,
                                       const Maths::Vec3 &up, float fovY, float aspect, float nearPlane,
                                       float farPlane) noexcept
      : _position(position), _up(up), _fovY(fovY), _aspect(aspect), _nearPlane(nearPlane), _farPlane(farPlane)
  {
    using namespace Maths;

    _forward = Normalize(Vec3(0.0f, 0.0f, -1.0f));
    _right = Normalize(Cross(_forward, _up));
    _up = Normalize(Cross(_right, _forward));
  }

  Maths::Mat4 FirstPersonCamera::ViewMatrix() const noexcept
  {
    return Maths::LookAt(_position, _position + _forward, _up);
  }

  Maths::Mat4 FirstPersonCamera::ProjectionMatrix() const noexcept
  {
    return Maths::Perspective(_fovY, _aspect, _nearPlane, _farPlane);
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
      _pitch = Maths::Clamp(_pitch, -89.0f, 89.0f);

      UpdateCameraVectors();
    }

    auto scrollDelta = mouse.ScrollDelta();
    if (scrollDelta != 0.0)
    {
      _fovY -= (float)mouse.ScrollDelta();
      _fovY = Maths::Clamp(_fovY, 1.0f, 45.0f);
    }
  }

  void FirstPersonCamera::OnResize(uint32 width, uint32 height) noexcept
  {
    _aspect = static_cast<float>(width) / static_cast<float>(height);
  }

  void FirstPersonCamera::UpdateCameraVectors() noexcept
  {
    using namespace Maths;
    Vec3 front {};
    front.x = std::cos(Radians(_yaw)) * std::cos(Radians(_pitch));
    front.y = std::sin(Radians(_pitch));
    front.z = std::sin(Radians(_yaw)) * std::cos(Radians(_pitch));
    _forward = Normalize(front);
    _right = Normalize(Cross(_forward, Vec3(0.0f, 1.0f, 0.0f)));
    _up = Normalize(Cross(_right, _forward));
  }
}