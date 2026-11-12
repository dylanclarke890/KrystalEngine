#include "Krystal.Booey/HTML/Images/ImageDocument.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey.Tests/Platform/ImageDecoders/TestImages.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A 300x150 PNG of one colour, larger than the 200x100 view.
    constexpr uint8 LargePNGBytes[] = {
      0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 0x00,
      0x00, 0x01, 0x2C, 0x00, 0x00, 0x00, 0x96, 0x08, 0x02, 0x00, 0x00, 0x00, 0xEB, 0x39, 0x22, 0x85, 0x00,
      0x00, 0x01, 0x4D, 0x49, 0x44, 0x41, 0x54, 0x78, 0xDA, 0xED, 0xD3, 0x41, 0x09, 0x00, 0x00, 0x08, 0x04,
      0xC1, 0xEB, 0x64, 0x27, 0x3B, 0xD9, 0xD6, 0x14, 0xE2, 0x67, 0x60, 0x12, 0x2C, 0x6C, 0xAA, 0x07, 0x78,
      0x14, 0x09, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60,
      0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42,
      0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0,
      0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84,
      0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60,
      0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42,
      0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0,
      0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0xC0, 0x84, 0x60, 0x42, 0x30, 0xA1, 0x0A, 0x60, 0x42, 0x30,
      0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21,
      0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60,
      0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42,
      0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30,
      0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21,
      0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60,
      0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x60, 0x42,
      0x30, 0x21, 0x60, 0x42, 0x30, 0x21, 0x98, 0x50, 0x05, 0x30, 0x21, 0x98, 0x10, 0x30, 0x21, 0x98, 0x10,
      0x30, 0x21, 0x98, 0x10, 0x30, 0x21, 0x98, 0x10, 0x30, 0x21, 0x98, 0x10, 0x30, 0x21, 0x98, 0x10, 0x30,
      0x21, 0x98, 0x10, 0x30, 0x21, 0x98, 0x10, 0x30, 0x21, 0x98, 0x10, 0xB8, 0xB2, 0xFD, 0xDB, 0x29, 0x5F,
      0x84, 0x6C, 0xBB, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82,
    };

    /// @brief A page whose main frame loads @p bytes as an image document of @p mimeType at @p url.
    struct ImageDocumentFixture : LoadFixture
    {
      ImageDocumentFixture(Span<const uint8> bytes, utf8_stringview mimeType, utf8_stringview url) noexcept
      {
        FrameLoadRequest request(ShareRef(Document()), ResourceRequest(url::URL(url)),
                                 dom::DOMStringAtom::Null());
        request.SetSubstituteData(
          SubstituteData(utf8_string(reinterpret_cast<const char8 *>(bytes.data()), bytes.size()),
                         utf8_string(mimeType), url::URL(url)));
        Frame().Loader().Load(krys::move(request));
        html::EventLoop::Current().RunUntilIdle();
        (void)Document().UpdateLayout();
      }

      KRYS_NODISCARD html::ImageDocument &ImageDocument() const noexcept
      {
        auto *document = DynamicDowncast<html::ImageDocument>(Document());
        REQUIRE(document);
        return *document;
      }

      KRYS_NODISCARD html::HTMLImageElement &Image() const noexcept
      {
        auto *image = ImageDocument().ImageElement();
        REQUIRE(image);
        return *image;
      }

      KRYS_NODISCARD std::string Style() const noexcept
      {
        return Narrow(Image().GetAttribute(u8"style").value_or(dom::DOMString()));
      }

      void ResizeView(int32 width, int32 height) noexcept
      {
        Frame().View()->Resize(IntSize(width, height));
        (void)Document().UpdateLayout();
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ImageDocument: a loaded image becomes an image document",
                   "[HTML][ImageDocument][html:read-media]")
  {
    ImageDocumentFixture fixture(images::PNG, u8"image/png", u8"https://example.com/dir/my%20image.png");
    auto &document = fixture.ImageDocument();

    CHECK(Narrow(document.ContentType()) == "image/png");
    CHECK_FALSE(document.InQuirksMode());
    CHECK_FALSE(document.InLimitedQuirksMode());
    CHECK(document.ReadyState() == html::DocumentReadyState::Complete);

    // An html element with a head and a body, whose only child is an img of the document's URL.
    auto *body = document.Body().get();
    REQUIRE(body);
    CHECK(document.Head());
    CHECK(body->FirstChild() == &fixture.Image());
    CHECK(body->FirstChild() == body->LastChild());
    CHECK(Narrow(fixture.Image().GetAttribute(u8"src").value_or(dom::DOMString()))
          == "https://example.com/dir/my%20image.png");

    // The image has the loaded bytes, without fetching them.
    CHECK(fixture.Image().Complete());
    CHECK(fixture.Image().NaturalWidth() == 3);
    CHECK(fixture.Image().NaturalHeight() == 2);

    // The title is the decoded file name and the image's natural size.
    CHECK(Narrow(document.Title())
          == "my image.png (3\xC3\x97"
             "2)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ImageDocument: the title falls back on the host",
                   "[HTML][ImageDocument]")
  {
    ImageDocumentFixture fixture(images::PNG, u8"image/png", u8"https://example.com/");
    CHECK(Narrow(fixture.ImageDocument().Title())
          == "example.com (3\xC3\x97"
             "2)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ImageDocument: an image larger than the view is shrunk to fit",
                   "[HTML][ImageDocument]")
  {
    ImageDocumentFixture fixture(LargePNGBytes, u8"image/png", u8"https://example.com/large.png");
    auto &image = fixture.Image();
    auto isShrunk = [&]
    {
      // Scaled down by the same factor in both directions, to fit the view.
      return image.Width() > 0 && image.Width() <= 200 && image.Height() <= 100
             && image.Width() == 2 * image.Height();
    };

    // It's larger than the 200x100 view, so it's shrunk as it loads, and a click will zoom it in.
    CHECK(isShrunk());
    CHECK(fixture.Style().find("cursor: zoom-in") != std::string::npos);

    SECTION("Growing the view to fit it restores it")
    {
      fixture.ResizeView(400, 300);
      CHECK(image.Width() == 300);
      CHECK(image.Height() == 150);
      CHECK(fixture.Style().find("cursor") == std::string::npos);

      // And shrinking the view again shrinks it.
      fixture.ResizeView(200, 100);
      CHECK(isShrunk());
    }

    SECTION("A click toggles between its natural size and the shrunk one")
    {
      image.Click();
      CHECK(image.Width() == 300);
      CHECK(image.Height() == 150);
      CHECK(fixture.Style().find("cursor: zoom-out") != std::string::npos);

      image.Click();
      CHECK(isShrunk());
      CHECK(fixture.Style().find("cursor: zoom-in") != std::string::npos);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ImageDocument: an image that fits the view is left as it is",
                   "[HTML][ImageDocument]")
  {
    ImageDocumentFixture fixture(images::PNG, u8"image/png", u8"https://example.com/image.png");
    CHECK(fixture.Image().Width() == 3);
    CHECK(fixture.Image().Height() == 2);
    CHECK(fixture.Style().find("cursor") == std::string::npos);

    // A click does nothing.
    fixture.Image().Click();
    CHECK(fixture.Image().Width() == 3);
    CHECK(fixture.Style().find("cursor") == std::string::npos);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ImageDocument: other types aren't image documents",
                   "[HTML][ImageDocument]")
  {
    ImageDocumentFixture fixture(images::PNG, u8"image/x-unknown", u8"https://example.com/image");
    CHECK_FALSE(Is<html::ImageDocument>(fixture.Document()));
  }
}
