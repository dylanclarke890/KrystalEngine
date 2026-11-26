#pragma once

#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "Krystal.Web/Css/Queries/MediaQueryParser.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Page/Clients/WebClient.hpp"
#include "Krystal.Web/Page/Frame.hpp"

namespace krys::web::css::tests
{
  /// @brief A Krystal.Web client whose answers to the media features are the test's.
  struct MediaWebClient final : WebClient
  {
    PointerCharacteristics Primary {PointerCharacteristic::Fine};
    bool PrimaryHovers {true};
    PointerCharacteristics AllAvailable {PointerCharacteristic::Fine};
    bool AnyHovers {true};
    SizeF Screen {1'920, 1'080};
    int32 DepthPerComponent {8};
    bool Monochrome {false};
    bool ExtendedColor {false};
    bool Rec2020Color {false};
    bool HighDynamicRange {false};
    bool Inverted {false};
    bool PrefersContrast {false};
    bool PrefersReducedMotion {false};

    void AttachRootGraphicsLayer(web::Frame &, GraphicsLayer *) noexcept override
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

    KRYS_NODISCARD SizeF ScreenSize() const noexcept override
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

  /// @brief A frame whose Krystal.Web client is a `MediaWebClient` (on a 1920x1080 screen), showing a document
  /// in an 800x600 view.
  struct MediaFrame
  {
    MediaWebClient *Client {nullptr};
    Ref<web::Frame> TestFrame;

    explicit MediaFrame(Document &document) noexcept : TestFrame(CreateFrame(Client))
    {
      auto &frame = *TestFrame;
      frame.CreateView({800, 600});
      frame.SetDocument(ShareRefPtr(&document));
    }

    KRYS_NODISCARD web::Frame &Frame() const noexcept
    {
      return *TestFrame;
    }

  private:
    KRYS_NODISCARD static Ref<web::Frame> CreateFrame(MediaWebClient *&clientOut) noexcept
    {
      auto client = CreateUnique<MediaWebClient>();
      clientOut = client.get();
      return web::Frame::Create(krys::move(client));
    }
  };

  /// @brief The media query list @p text parses to, in strict mode.
  KRYS_NODISCARD inline mq::MediaQueryList ParseMediaQueries(utf8_stringview text)
  {
    return mq::MediaQueryParser::Parse(text, StrictParserContext());
  }
}
