#include "Krystal.Gfx/Device.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx/OpenGL/GLDevice.hpp"

namespace krys::gfx
{
  Expected<UniquePtr<Device>> Device::Create(const DeviceDesc &desc) noexcept
  {
    auto device = CreateUnique<opengl::GLDevice>(desc);
    if (auto result = device->Initialise(); !result)
    {
      return Unexpected<string>(result.error());
    }
    return UniquePtr<Device>(krys::move(device));
  }
}
