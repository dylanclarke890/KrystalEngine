#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Gfx/Device.hpp"

namespace krys::gfx::tests
{
  /// @brief Creates a validating device for GPU tests, failing the test if there's no OpenGL 4.5 device.
  /// The device logs its debug output and leaks through the global logger, which the first call sets up.
  KRYS_NODISCARD inline UniquePtr<Device> CreateTestDevice()
  {
    static const auto logger = []
    {
      auto created = log::CreateLogger({.Name = "Gfx.Tests", .FilePath = {}, .Level = log::Level::Warn});
      REQUIRE(created.has_value());
      log::SetGlobalLogger(*created);
      return krys::move(*created);
    }();

    auto device = Device::Create({.EnableValidation = true, .TransientMemoryPerFrame = 64u * 1'024u});
    if (!device)
    {
      FAIL("No OpenGL 4.5 device: " << device.error());
    }
    return krys::move(*device);
  }
}
