#include "Krystal.UI/LayoutEngine.hpp"
#include "Krystal.UI/Document.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("LayoutEngine::ComputeLayout(Simple, All Percentages)", "[LayoutEngine]")
  {
    using namespace UI::Literals;

    UI::DocumentSettings settings = {.Viewport = UI::BoundingBox(0.f, 0.f, 1000.f, 1000.f)};
    UI::Document document(settings);
    UI::LayoutEngine &layoutEngine = document.GetLayoutEngine();

    UI::Body body;
    UI::Element &firstBodyChild = document.CreateElement<UI::Element>();
    firstBodyChild.Properties().Width = 50._pct;
    firstBodyChild.Properties().Height = 50._pct;
    body.AppendChild(firstBodyChild);

    UI::Element &firstBodySubChild = document.CreateElement<UI::Element>();
    firstBodySubChild.Properties().Width = 50._pct;
    firstBodySubChild.Properties().Height = 50._pct;
    firstBodyChild.AppendChild(firstBodySubChild);

    UI::Element &secondBodyChild = document.CreateElement<UI::Element>();
    secondBodyChild.Properties().Width = 50._pct;
    secondBodyChild.Properties().Height = 50._pct;
    body.AppendChild(secondBodyChild);

    layoutEngine.ComputeLayout(body, settings.Viewport);

    const auto &computedLayout = body.GetComputedLayout();
    REQUIRE(computedLayout.Width.Value == 1000.f);
    REQUIRE(computedLayout.Height.Value == 1000.f);

    const auto &firstChildLayout = firstBodyChild.GetComputedLayout();
    REQUIRE(firstChildLayout.Width.Value == 500.f);
    REQUIRE(firstChildLayout.Height.Value == 500.f);
    REQUIRE(firstChildLayout.X == 0.f);
    REQUIRE(firstChildLayout.Y == 0.f);

    const auto &firstSubChildLayout = firstBodySubChild.GetComputedLayout();
    REQUIRE(firstSubChildLayout.Width.Value == 250.f);
    REQUIRE(firstSubChildLayout.Height.Value == 250.f);
    REQUIRE(firstSubChildLayout.X == 0.f);
    REQUIRE(firstSubChildLayout.Y == 0.f);

    const auto &secondChildLayout = secondBodyChild.GetComputedLayout();
    REQUIRE(secondChildLayout.Width.Value == 500.f);
    REQUIRE(secondChildLayout.Height.Value == 500.f);
    REQUIRE(secondChildLayout.X == 500.f);
    REQUIRE(secondChildLayout.Y == 0.f);
  }

  TEST_CASE("LayoutEngine::ComputeLayout(Simple, Mixed Units)", "[LayoutEngine]")
  {
    using namespace UI::Literals;

    UI::DocumentSettings settings = {.Viewport = UI::BoundingBox(0.f, 0.f, 800.f, 600.f)};
    UI::Document document(settings);
    UI::LayoutEngine &layoutEngine = document.GetLayoutEngine();

    UI::Body body;
    UI::Element &firstBodyChild = document.CreateElement<UI::Element>();
    firstBodyChild.Properties().Width = 400._px;
    firstBodyChild.Properties().Height = 50._pct;
    body.AppendChild(firstBodyChild);

    UI::Element &firstBodySubChild = document.CreateElement<UI::Element>();
    firstBodySubChild.Properties().Width = 50._pct;
    firstBodySubChild.Properties().Height = 100._px;
    firstBodyChild.AppendChild(firstBodySubChild);

    UI::Element &secondBodyChild = document.CreateElement<UI::Element>();
    secondBodyChild.Properties().Width = 50._pct;
    secondBodyChild.Properties().Height = 300._px;
    body.AppendChild(secondBodyChild);

    layoutEngine.ComputeLayout(body, settings.Viewport);

    const auto &computedLayout = body.GetComputedLayout();
    REQUIRE(computedLayout.Width.Value == 800.f);
    REQUIRE(computedLayout.Height.Value == 600.f);

    const auto &firstChildLayout = firstBodyChild.GetComputedLayout();
    REQUIRE(firstChildLayout.Width.Value == 400.f);
    REQUIRE(firstChildLayout.Height.Value == 300.f);
    REQUIRE(firstChildLayout.X == 0.f);
    REQUIRE(firstChildLayout.Y == 0.f);

    const auto &firstSubChildLayout = firstBodySubChild.GetComputedLayout();
    REQUIRE(firstSubChildLayout.Width.Value == 200.f);
    REQUIRE(firstSubChildLayout.Height.Value == 100.f);
    REQUIRE(firstSubChildLayout.X == 0.f);
    REQUIRE(firstSubChildLayout.Y == 0.f);

    const auto &secondChildLayout = secondBodyChild.GetComputedLayout();
    REQUIRE(secondChildLayout.Width.Value == 400.f);
    REQUIRE(secondChildLayout.Height.Value == 300.f);
    REQUIRE(secondChildLayout.X == 400.f);
    REQUIRE(secondChildLayout.Y == 0.f);
  }

  TEST_CASE("LayoutEngine::ComputeLayout(Simple, Auto)", "[LayoutEngine]")
  {
    using namespace UI::Literals;

    UI::DocumentSettings settings = {.Viewport = UI::BoundingBox(0.f, 0.f, 800.f, 600.f)};
    UI::Document document(settings);
    UI::LayoutEngine &layoutEngine = document.GetLayoutEngine();

    UI::Body body;
    UI::Element &firstBodyChild = document.CreateElement<UI::Element>();
    firstBodyChild.Properties().Width = 400._px;
    firstBodyChild.Properties().Height = UI::Auto;
    body.AppendChild(firstBodyChild);

    UI::Element &firstBodySubChild = document.CreateElement<UI::Element>();
    firstBodySubChild.Properties().Width = 50._pct;
    firstBodySubChild.Properties().Height = 100._px;
    firstBodyChild.AppendChild(firstBodySubChild);

    layoutEngine.ComputeLayout(body, settings.Viewport);

    const auto &computedLayout = body.GetComputedLayout();
    REQUIRE(computedLayout.Width.Value == 800.f);
    REQUIRE(computedLayout.Height.Value == 600.f);

    const auto &firstChildLayout = firstBodyChild.GetComputedLayout();
    REQUIRE(firstChildLayout.Width.Value == 400.f);
    REQUIRE(firstChildLayout.Height.Value == 100.f);
    REQUIRE(firstChildLayout.X == 0.f);
    REQUIRE(firstChildLayout.Y == 0.f);

    const auto &firstSubChildLayout = firstBodySubChild.GetComputedLayout();
    REQUIRE(firstSubChildLayout.Width.Value == 200.f);
    REQUIRE(firstSubChildLayout.Height.Value == 100.f);
    REQUIRE(firstSubChildLayout.X == 0.f);
    REQUIRE(firstSubChildLayout.Y == 0.f);
  }
}