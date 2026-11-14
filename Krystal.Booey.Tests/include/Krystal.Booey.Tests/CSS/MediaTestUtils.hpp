#pragma once

#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css::tests
{
  /// @brief A chrome client whose answers to the media features are the test's.
  struct MediaChromeClient final : ChromeClient
  {
    PointerCharacteristics Primary {PointerCharacteristics::Fine};
    bool PrimaryHovers {true};
    PointerCharacteristics AllAvailable {PointerCharacteristics::Fine};
    bool AnyHovers {true};
    FloatSize Screen {1'920, 1'080};
    int32 DepthPerComponent {8};
    bool Monochrome {false};
    bool ExtendedColor {false};
    bool Rec2020Color {false};
    bool HighDynamicRange {false};
    bool Inverted {false};
    bool PrefersContrast {false};
    bool PrefersReducedMotion {false};

    void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept override
    {
    }

    KRYS_NODISCARD PointerCharacteristics PrimaryPointingDevice() const noexcept override
    {
      return Primary;
    }
    KRYS_NODISCARD bool HoverSupportedByPrimaryPointingDevice() const noexcept override
    {
      return PrimaryHovers;
    }
    KRYS_NODISCARD PointerCharacteristics AllAvailablePointingDevices() const noexcept override
    {
      return AllAvailable;
    }
    KRYS_NODISCARD bool HoverSupportedByAnyAvailablePointingDevice() const noexcept override
    {
      return AnyHovers;
    }
    KRYS_NODISCARD FloatSize ScreenSize() const noexcept override
    {
      return Screen;
    }
    KRYS_NODISCARD int32 ScreenDepthPerComponent() const noexcept override
    {
      return DepthPerComponent;
    }
    KRYS_NODISCARD bool ScreenIsMonochrome() const noexcept override
    {
      return Monochrome;
    }
    KRYS_NODISCARD bool ScreenSupportsExtendedColor() const noexcept override
    {
      return ExtendedColor;
    }
    KRYS_NODISCARD bool ScreenSupportsRec2020Color() const noexcept override
    {
      return Rec2020Color;
    }
    KRYS_NODISCARD bool ScreenSupportsHighDynamicRange() const noexcept override
    {
      return HighDynamicRange;
    }
    KRYS_NODISCARD bool ColorsAreInverted() const noexcept override
    {
      return Inverted;
    }
    KRYS_NODISCARD bool UserPrefersContrast() const noexcept override
    {
      return PrefersContrast;
    }
    KRYS_NODISCARD bool UserPrefersReducedMotion() const noexcept override
    {
      return PrefersReducedMotion;
    }
  };

  /// @brief A page whose chrome client is a `MediaChromeClient` (on a 1920x1080 screen), showing a document
  /// in an 800x600 view of its main frame.
  struct MediaPage
  {
    MediaChromeClient *Chrome {nullptr};
    Ref<Page> TestPage;

    explicit MediaPage(dom::Document &document) noexcept : TestPage(CreatePage(Chrome))
    {
      auto &frame = TestPage->MainFrame();
      frame.CreateView({800, 600});
      frame.SetDocument(ShareRefPtr(&document));
    }

    KRYS_NODISCARD LocalFrame &Frame() const noexcept
    {
      return TestPage->MainFrame();
    }

  private:
    KRYS_NODISCARD static Ref<Page> CreatePage(MediaChromeClient *&chrome) noexcept
    {
      auto client = CreateUnique<MediaChromeClient>();
      chrome = client.get();
      return Page::Create(krys::move(client));
    }
  };
}
