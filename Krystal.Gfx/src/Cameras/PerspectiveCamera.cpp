#include "Krystal.Gfx/Cameras/PerspectiveCamera.hpp"

namespace Krys::Gfx
{
  PerspectiveCamera::PerspectiveCamera(const Maths::Vec3 &position, const Maths::Vec3 &target,
                                       const Maths::Vec3 &up, float fovY, float aspect, float nearPlane,
                                       float farPlane) noexcept
      : _position(position), _up(up)
  {
    _forward = Maths::Normalize(target - position);
    _right = Maths::Normalize(Maths::Cross(_forward, _up));
    _up = Maths::Normalize(Maths::Cross(_right, _forward));
    _viewMatrix = Maths::LookAt(position, target, _up);
    _projectionMatrix = Maths::Perspective(fovY, aspect, nearPlane, farPlane);
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
}