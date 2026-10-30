#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Graphics/WOFFFileFormat.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace krys::boo::tests
{
  /// @brief Registers the web-platform-tests math font @p name (in `data/fonts/math`), whose family is its
  /// name.
  inline void RegisterMathFont(const char *name) noexcept
  {
    const auto *family = reinterpret_cast<const char8 *>(name);
    if (!FontDatabase::Singleton().FacesInFamily(utf8_stringview(family, std::strlen(name))).empty())
    {
      return;
    }
    std::ifstream stream(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts" / "math"
                           / (std::string {name} + ".woff"),
                         std::ios::binary);
    REQUIRE(stream);
    List<uint8> bytes {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    RefPtr<SharedBuffer> buffer = SharedBuffer::Create(krys::move(bytes));
    REQUIRE(ConvertWOFFToSfntIfNecessary(buffer));
    auto span = buffer->GetSpan();
    const auto *data = reinterpret_cast<const byte *>(span.data());
    REQUIRE(FontDatabase::Singleton().AddFontData(List<byte>(data, data + span.size())) == 1u);
  }

  /// @brief A laid out document whose formulas are in the 10px web-platform-tests math font @p font (a
  /// hundredth of a pixel a font unit), with @p body as its body's content.
  KRYS_NODISCARD inline style::tests::ResolverFixture MathFontDocument(const char *font,
                                                                       utf8_stringview body) noexcept
  {
    RegisterMathFont(font);
    utf8_string style = u8"math { font-family: ";
    style += reinterpret_cast<const char8 *>(font);
    style += u8" } mspace { font-size: 10px }";
    style::tests::ResolverFixture fixture {layout::tests::AhemDocument(style, body)};
    (void)fixture.Document->UpdateLayout();
    return fixture;
  }

  /// @brief A laid out document whose formulas are in 10px Ahem, with @p body as its body's content and
  /// spaces sized in 10px ems even in scripts.
  KRYS_NODISCARD inline style::tests::ResolverFixture AhemMathDocument(utf8_stringview body) noexcept
  {
    style::tests::ResolverFixture fixture {
      layout::tests::AhemDocument(u8"math { font-family: Ahem } mspace { font-size: 10px }", body)};
    (void)fixture.Document->UpdateLayout();
    return fixture;
  }

  /// @brief The border box of the element @p id in @p fixture, relative to the viewport.
  KRYS_NODISCARD inline FloatRect BoxOf(style::tests::ResolverFixture &fixture,
                                        dom::DOMStringView id) noexcept
  {
    (void)fixture.Document->UpdateLayout();
    return fixture.ById(id).BoundingClientRect();
  }

  /// @brief The display list items of type @p TItem that @p fixture's view paints, in order.
  template <typename TItem>
  KRYS_NODISCARD List<TItem> PaintedItems(style::tests::ResolverFixture &fixture) noexcept
  {
    (void)fixture.Document->UpdateLayout();
    display_list::RecorderImpl recorder(FloatSize(800, 600));
    fixture.Frame->View()->Paint(recorder, IntRect(0, 0, 800, 600));
    List<TItem> items;
    for (const auto &item : recorder.TakeDisplayList()->Items())
    {
      if (const auto *found = std::get_if<TItem>(&item))
      {
        items.push_back(*found);
      }
    }
    return items;
  }

  /// @brief Whether @p actual is within a pixel of @p expected, as web-platform-tests' MathML tests compare.
  KRYS_NODISCARD inline bool Near(float actual, float expected) noexcept
  {
    return std::abs(actual - expected) <= 1.0f;
  }
}
