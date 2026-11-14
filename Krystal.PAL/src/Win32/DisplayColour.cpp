#include "Krystal.PAL/Win32/DisplayColour.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <cwchar>
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl/client.h>

#pragma comment(lib, "dxgi.lib")

namespace krys::pal::win32
{
  namespace
  {
    /// @brief The luminance the system shows standard dynamic range white at on @p monitor, in nits, if it
    /// says (for displays with HDR on: the "SDR content brightness" setting).
    KRYS_NODISCARD Maybe<float> SDRWhiteLevel(HMONITOR monitor) noexcept
    {
      MONITORINFOEXW info {};
      info.cbSize = sizeof(info);
      if (!::GetMonitorInfoW(monitor, &info))
      {
        return null;
      }

      UINT32 pathCount = 0;
      UINT32 modeCount = 0;
      if (::GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS)
      {
        return null;
      }
      List<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
      List<DISPLAYCONFIG_MODE_INFO> modes(modeCount);
      if (::QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.data(), &modeCount, modes.data(),
                               nullptr)
          != ERROR_SUCCESS)
      {
        return null;
      }

      for (UINT32 index = 0; index < pathCount; ++index)
      {
        const auto &path = paths[index];
        DISPLAYCONFIG_SOURCE_DEVICE_NAME source {};
        source.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        source.header.size = sizeof(source);
        source.header.adapterId = path.sourceInfo.adapterId;
        source.header.id = path.sourceInfo.id;
        if (::DisplayConfigGetDeviceInfo(&source.header) != ERROR_SUCCESS
            || std::wcscmp(source.viewGdiDeviceName, info.szDevice) != 0)
        {
          continue;
        }

        DISPLAYCONFIG_SDR_WHITE_LEVEL whiteLevel {};
        whiteLevel.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL;
        whiteLevel.header.size = sizeof(whiteLevel);
        whiteLevel.header.adapterId = path.targetInfo.adapterId;
        whiteLevel.header.id = path.targetInfo.id;
        if (::DisplayConfigGetDeviceInfo(&whiteLevel.header) != ERROR_SUCCESS)
        {
          return null;
        }
        // The level is a multiple of 80 nits, in thousandths.
        return static_cast<float>(whiteLevel.SDRWhiteLevel) / 1000.f * 80.f;
      }
      return null;
    }
  }

  DisplayColour ReadDisplayColour(HMONITOR__ *monitor) noexcept
  {
    DisplayColour colour;
    if (!monitor)
    {
      return colour;
    }

    // A new factory each time, as one made before a display change describes the displays as they were.
    Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
    if (FAILED(::CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
    {
      return colour;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0; factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND;
         ++adapterIndex)
    {
      Microsoft::WRL::ComPtr<IDXGIOutput> output;
      for (UINT outputIndex = 0; adapter->EnumOutputs(outputIndex, &output) != DXGI_ERROR_NOT_FOUND;
           ++outputIndex)
      {
        Microsoft::WRL::ComPtr<IDXGIOutput6> output6;
        DXGI_OUTPUT_DESC1 description {};
        if (FAILED(output.As(&output6)) || FAILED(output6->GetDesc1(&description))
            || description.Monitor != monitor)
        {
          continue;
        }

        colour.HighDynamicRange = description.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
        colour.BitsPerComponent = description.BitsPerColor ? description.BitsPerColor : 8u;
        colour.MaxLuminance = description.MaxLuminance;
        colour.Gamut = GamutOfPrimaries({description.RedPrimary[0], description.RedPrimary[1]},
                                        {description.GreenPrimary[0], description.GreenPrimary[1]},
                                        {description.BluePrimary[0], description.BluePrimary[1]});
        // As in Chrome, a display showing high dynamic range counts as showing Rec. 2020's gamut.
        if (colour.HighDynamicRange)
        {
          colour.Gamut = DisplayGamut::Rec2020;
        }
        if (auto whiteLevel = SDRWhiteLevel(monitor))
        {
          colour.SDRWhiteLevel = *whiteLevel;
        }
        return colour;
      }
    }
    return colour;
  }
}
