#include "Krystal.Booey/Graphics/BitmapImage.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/AnimatedImages.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/ImageObserver.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Counts what an image tells its observer.
    class CountingObserver final : public ImageObserver
    {
    public:
      int Draws {0};
      int FramesAvailable {0};
      int64 DecodedSize {0};
      List<EncodedDataStatus> Statuses;

      KRYS_NODISCARD url::URL SourceUrl() const noexcept override
      {
        return {};
      }
      KRYS_NODISCARD utf8_string MimeType() const noexcept override
      {
        return u8"image/png";
      }
      KRYS_NODISCARD int64 ExpectedContentLength() const noexcept override
      {
        return 0;
      }
      void EncodedDataStatusChanged(const Image &, EncodedDataStatus status) noexcept override
      {
        Statuses.push_back(status);
      }
      void DecodedSizeChanged(const Image &, int64 delta) noexcept override
      {
        DecodedSize += delta;
      }
      void DidDraw(const Image &) noexcept override
      {
        ++Draws;
      }
      void ImageFrameAvailable(const Image &, ImageAnimatingState state, const IntRect *,
                               DecodingStatus) noexcept override
      {
        if (state == ImageAnimatingState::Yes)
        {
          ++FramesAvailable;
        }
      }
      void ChangedInRect(const Image &, const IntRect *) noexcept override
      {
      }
      void ImageContentChanged(const Image &) noexcept override
      {
      }
      void ScheduleRenderingUpdate(const Image &) noexcept override
      {
      }
    };

    KRYS_NODISCARD Ref<BitmapImage> ImageFrom(Span<const uint8> bytes,
                                              ImageObserver *observer = nullptr) noexcept
    {
      auto image = BitmapImage::Create(observer);
      (void)image->SetData(SharedBuffer::Create(bytes), true);
      return image;
    }

    KRYS_NODISCARD Ref<const display_list::DisplayList> Record(Image &image,
                                                               const FloatRect &destination) noexcept
    {
      display_list::RecorderImpl recorder(FloatSize {100, 100});
      (void)recorder.DrawImage(image, destination);
      return recorder.TakeDisplayList();
    }

    KRYS_NODISCARD uint32 PixelAt(const NativeImage &image, int32 x, int32 y) noexcept
    {
      const auto &platformImage = *image.GetPlatformImage();
      return platformImage.Pixels()[static_cast<size_t>(y * platformImage.Size().Width() + x)];
    }
  }

  TEST_CASE("BitmapImage: metadata comes from the decoded data", "[Graphics][BitmapImage]")
  {
    auto image = BitmapImage::Create();
    CHECK(image->IsNull());
    CHECK(image->SetData(SharedBuffer::Create(images::PNG), true) == EncodedDataStatus::Complete);

    CHECK(Is<BitmapImage>(static_cast<Image &>(*image)));
    CHECK(image->Size() == FloatSize(3, 2));
    CHECK(image->FrameCount() == 1u);
    CHECK_FALSE(image->IsAnimated());
    CHECK(image->FilenameExtension() == u8"png");
    CHECK_FALSE(image->CurrentFrameKnownToBeOpaque());
    CHECK(image->Orientation() == ImageOrientation::Orientation::None);

    auto nativeImage = image->GetNativeImage();
    REQUIRE(nativeImage);
    CHECK(nativeImage->Size() == IntSize(3, 2));
    CHECK(PixelAt(*nativeImage, 2, 0) == 0xFF'00'00'FFu);
    // The frame is complete once it's decoded.
    CHECK(image->CurrentFrameIsComplete());
    // The decoded frame is cached.
    CHECK(image->GetNativeImage() == nativeImage);
  }

  TEST_CASE("BitmapImage: an empty image and data that isn't an image", "[Graphics][BitmapImage]")
  {
    CHECK(Image::NullImage().IsNull());
    CHECK(Image::NullImage().IsBitmapImage());

    const uint8 text[] = "definitely not an image";
    auto image = BitmapImage::Create();
    CHECK(image->SetData(SharedBuffer::Create(Span<const uint8> {text}), true) == EncodedDataStatus::Unknown);
    CHECK(image->IsNull());
    CHECK_FALSE(image->GetNativeImage());
    CHECK(Record(*image, {0, 0, 10, 10})->Items().empty());
  }

  TEST_CASE("BitmapImage: the EXIF orientation sizes and transforms the image", "[Graphics][BitmapImage]")
  {
    auto image = ImageFrom(images::RotatedJPEG);
    CHECK(image->Orientation() == ImageOrientation::Orientation::OriginRightTop);
    CHECK(image->Size() == FloatSize(8, 16));
    CHECK(image->Size(ImageOrientation::Orientation::None) == FloatSize(16, 8));

    SECTION("Drawing passes the orientation on")
    {
      auto displayList = Record(*image, {0, 0, 8, 16});
      REQUIRE(displayList->Items().size() == 1u);
      const auto &item = std::get<display_list::DrawNativeImage>(displayList->Items()[0]);
      CHECK(item.Options().Orientation() == ImageOrientation::Orientation::OriginRightTop);
      CHECK(item.DestinationRect() == FloatRect(0, 0, 8, 16));
      CHECK(item.Source() == FloatRect(0, 0, 8, 16));
      CHECK(item.GetNativeImage().Size() == IntSize(16, 8));
    }

    SECTION("The pre-transformed image is rotated")
    {
      // Red on the left and blue on the right, turned clockwise: red on top and blue below.
      auto oriented = static_cast<Image &>(*image).CurrentPreTransformedNativeImage();
      REQUIRE(oriented);
      CHECK(oriented->Size() == IntSize(8, 16));
      auto red = PixelAt(*oriented, 4, 2);
      auto blue = PixelAt(*oriented, 4, 13);
      CHECK(((red >> 16) & 0xFFu) > 0xE0u);
      CHECK((red & 0xFFu) < 0x20u);
      CHECK((blue & 0xFFu) > 0xE0u);
      CHECK(((blue >> 16) & 0xFFu) < 0x20u);
    }
  }

  TEST_CASE("BitmapImage: drawing records native image draws", "[Graphics][BitmapImage]")
  {
    auto image = ImageFrom(images::PNG);

    SECTION("An image")
    {
      auto displayList = Record(*image, {10, 10, 30, 20});
      REQUIRE(displayList->Items().size() == 1u);
      const auto &item = std::get<display_list::DrawNativeImage>(displayList->Items()[0]);
      CHECK(item.DestinationRect() == FloatRect(10, 10, 30, 20));
      CHECK(item.Source() == FloatRect(0, 0, 3, 2));
      CHECK(&item.GetNativeImage() == image->GetNativeImage().get());
    }

    SECTION("A tiled image")
    {
      display_list::RecorderImpl recorder(FloatSize {100, 100});
      (void)recorder.DrawTiledImage(*image, {0, 0, 30, 20}, {}, {3, 2}, {});
      auto displayList = recorder.TakeDisplayList();
      REQUIRE(displayList->Items().size() == 1u);
      const auto &item = std::get<display_list::DrawPatternNativeImage>(displayList->Items()[0]);
      CHECK(item.DestRect() == FloatRect(0, 0, 30, 20));
      CHECK(item.TileRect() == FloatRect(0, 0, 3, 2));
    }

    SECTION("A single pixel draws as its colour")
    {
      auto solid = ImageFrom(images::SolidPNG);
      auto displayList = Record(*solid, {0, 0, 10, 10});
      bool filled = false;
      for (const auto &item : displayList->Items())
      {
        filled |= std::holds_alternative<display_list::FillRectWithColor>(item);
        CHECK_FALSE(std::holds_alternative<display_list::DrawNativeImage>(item));
      }
      CHECK(filled);
    }
  }

  TEST_CASE("BitmapImage: tells its observer about data, decoding and drawing", "[Graphics][BitmapImage]")
  {
    auto observer = AdoptRef(*new CountingObserver());
    auto image = ImageFrom(images::PNG, observer.get());
    REQUIRE_FALSE(observer->Statuses.empty());
    CHECK(observer->Statuses.back() == EncodedDataStatus::Complete);
    CHECK(image->MimeType() == u8"image/png");

    (void)Record(*image, {0, 0, 3, 2});
    CHECK(observer->Draws == 1);
    CHECK(observer->DecodedSize == 3 * 2 * 4);

    image->DestroyDecodedData();
    CHECK(observer->DecodedSize == 0);

    // The frame decodes again when it's next drawn.
    (void)Record(*image, {0, 0, 3, 2});
    CHECK(observer->Draws == 2);
    CHECK(observer->DecodedSize == 3 * 2 * 4);
  }

  TEST_CASE("BitmapImage: decoding calls back once the frame is decoded", "[Graphics][BitmapImage]")
  {
    auto image = ImageFrom(images::GIF);
    Maybe<DecodingStatus> status;
    image->Decode([&](DecodingStatus decodingStatus) { status = decodingStatus; });
    CHECK(status == DecodingStatus::Complete);
  }

  TEST_CASE("BitmapImage: made from a native image", "[Graphics][BitmapImage]")
  {
    auto pixels = AdoptRef(*new PixelData(List<uint32> {0xFF'FF'00'00u, 0xFF'00'FF'00u}));
    PlatformImagePtr platformImage = AdoptRef(*new PlatformImage(IntSize(2, 1), krys::move(pixels), true));
    auto image = BitmapImage::Create(krys::move(platformImage));
    REQUIRE(image);
    CHECK(image->Size() == FloatSize(2, 1));
    auto nativeImage = image->GetNativeImage();
    REQUIRE(nativeImage);
    CHECK(PixelAt(*nativeImage, 1, 0) == 0xFF'00'FF'00u);

    CHECK_FALSE(BitmapImage::Create(RefPtr<NativeImage>()));
  }

  TEST_CASE("BitmapImage: animated images advance their frames in the host's time", "[Graphics][BitmapImage]")
  {
    using namespace std::chrono_literals;
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();

    for (auto bytes : {images::AnimatedGIF, images::AnimatedPNG, images::AnimatedWebP})
    {
      auto observer = AdoptRef(*new CountingObserver());
      auto image = ImageFrom(bytes, observer.get());
      REQUIRE(image->IsAnimated());
      CHECK(image->CurrentFrameIndex() == 0u);
      CHECK_FALSE(image->IsAnimating());

      // Drawing starts the animation: the first frame shows for its 100ms.
      auto start = loop.Now();
      (void)Record(*image, FloatRect(0, 0, 2, 2));
      CHECK(image->IsAnimating());
      loop.Run(start + 50ms);
      CHECK(image->CurrentFrameIndex() == 0u);
      loop.Run(start + 100ms);
      CHECK(image->CurrentFrameIndex() == 1u);
      CHECK(observer->FramesAvailable == 1);
      // The next frame's timer starts when the frame is drawn.
      CHECK_FALSE(image->IsAnimating());
      loop.Run(start + 250ms);
      CHECK(image->CurrentFrameIndex() == 1u);

      // Drawn late, the frame shows for what's left of its time (none, so it advances at once).
      (void)Record(*image, FloatRect(0, 0, 2, 2));
      loop.Run(start + 250ms);
      CHECK(image->CurrentFrameIndex() == 2u);
      (void)Record(*image, FloatRect(0, 0, 2, 2));
      loop.Run(start + 350ms);
      CHECK(image->CurrentFrameIndex() == 0u);

      image->ResetAnimation();
      CHECK(image->CurrentFrameIndex() == 0u);
      CHECK_FALSE(image->IsAnimating());
    }
  }

  TEST_CASE("BitmapImage: an animation stops after its loops", "[Graphics][BitmapImage]")
  {
    using namespace std::chrono_literals;
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();

    auto image = ImageFrom(images::OnceGIF);
    REQUIRE(image->IsAnimated());
    auto start = loop.Now();
    (void)Record(*image, FloatRect(0, 0, 2, 2));
    loop.Run(start + 100ms);
    CHECK(image->CurrentFrameIndex() == 1u);

    // It played once, so it stays on its last frame.
    (void)Record(*image, FloatRect(0, 0, 2, 2));
    CHECK_FALSE(image->IsAnimating());
    loop.Run(start + 300ms);
    CHECK(image->CurrentFrameIndex() == 1u);

    // Stopping keeps the frame; resetting goes back to the first.
    image->StopAnimation();
    CHECK(image->CurrentFrameIndex() == 1u);
    image->ResetAnimation();
    CHECK(image->CurrentFrameIndex() == 0u);
  }
}
