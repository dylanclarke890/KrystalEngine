#include "Krystal.UI/View.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/CommandEvent.hpp"
#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/PointerEvent.hpp"
#include "Krystal.Booey/DOM/TouchEvent.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElement.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLDetailsElement.hpp"
#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/NavigateEvent.hpp"
#include "Krystal.Booey/Page/Navigation.hpp"
#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Booey/Platform/DateComponents.hpp"
#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Platform/SelectionData.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Editor/BeanRating.hpp"
#include "Krystal.Editor/PageAnimations.hpp"
#include "Krystal.Editor/PageObservers.hpp"
#include "Krystal.Editor/PagePointers.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/ColorChooserOverlay.hpp"
#include "Krystal.UI/DateTimeChooserOverlay.hpp"
#include "Krystal.UI/FileLoaderStrategy.hpp"
#include "Krystal.UI/FileStorageProvider.hpp"
#include "Krystal.UI/Input.hpp"
#include "Krystal.UI/PopupMenuOverlay.hpp"
#include <filesystem>
#include <fstream>
#include <string>

namespace krys::ui::tests
{
  namespace
  {
    /// @brief A clipboard in memory, in place of the system's: text and markup, counting the writes.
    struct MemoryPasteboard final : boo::PasteboardStrategy
    {
      int32 Writes {0};
      utf8_string Text;
      utf8_string Markup;

      KRYS_NODISCARD List<utf8_string> Types(utf8_stringview) noexcept override
      {
        List<utf8_string> types;
        if (!Text.empty())
        {
          types.emplace_back(u8"text/plain");
        }
        if (!Markup.empty())
        {
          types.emplace_back(u8"text/html");
        }
        return types;
      }
      KRYS_NODISCARD utf8_string ReadTextFromClipboard(utf8_stringview,
                                                       utf8_stringview type) noexcept override
      {
        return type.starts_with(u8"text/plain") ? Text : type == u8"text/html" ? Markup : utf8_string();
      }
      KRYS_NODISCARD List<utf8_string> ReadFilePathsFromClipboard(utf8_stringview) noexcept override
      {
        return {};
      }
      KRYS_NODISCARD RefPtr<boo::SharedBuffer> ReadBufferFromClipboard(utf8_stringview,
                                                                       utf8_stringview) noexcept override
      {
        return nullptr;
      }
      void WriteToClipboard(utf8_stringview, boo::SelectionData &&selectionData) noexcept override
      {
        ++Writes;
        Text = selectionData.Text();
        Markup = selectionData.Markup();
      }
      void ClearClipboard(utf8_stringview) noexcept override
      {
        Text.clear();
        Markup.clear();
      }
      KRYS_NODISCARD int64 ChangeCount(utf8_stringview) noexcept override
      {
        return Writes;
      }
      KRYS_NODISCARD List<utf8_string> TypesSafeForDOMToReadAndWrite(utf8_stringview name,
                                                                     utf8_stringview) noexcept override
      {
        return Types(name);
      }
      int64 WriteCustomData(const List<boo::PasteboardCustomData> &, utf8_stringview) noexcept override
      {
        return Writes;
      }
    };

    KRYS_NODISCARD int32 WidthOf(View &view, boo::dom::DOMStringView id)
    {
      auto element = view.Document().GetElementById(id);
      REQUIRE(element);
      return element->OffsetWidth();
    }

    /// @brief Records the keyboard events it handles as `type:key/code/keyCode/charCode/location`, with
    /// `+shift` and `+ctrl` for the modifiers.
    struct KeyRecorder : public boo::dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit KeyRecorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(boo::dom::Event &event) noexcept override
      {
        auto &key = static_cast<boo::dom::KeyboardEvent &>(event);
        auto type = key.Type().View();
        auto record =
          std::string(type.begin(), type.end()) + ":" + std::string(key.Key().begin(), key.Key().end()) + "/"
          + std::string(key.Code().begin(), key.Code().end()) + "/" + std::to_string(key.KeyCode()) + "/"
          + std::to_string(key.CharCode()) + "/" + std::to_string(key.Location());
        if (key.ShiftKey())
        {
          record += "+shift";
        }
        if (key.CtrlKey())
        {
          record += "+ctrl";
        }
        Records.push_back(record);
      }
    };

    /// @brief Records the presses, releases and clicks it handles as `type:id/detail`.
    struct ClickRecorder : public boo::dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit ClickRecorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(boo::dom::Event &event) noexcept override
      {
        auto &mouse = static_cast<boo::dom::MouseEvent &>(event);
        auto type = mouse.Type().View();
        auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
        auto *element = node ? DynamicDowncast<boo::dom::Element>(*node) : nullptr;
        auto id =
          element ? element->GetAttribute(u8"id").value_or(boo::dom::DOMString()) : boo::dom::DOMString();
        Records.push_back(std::string(type.begin(), type.end()) + ":" + std::string(id.begin(), id.end())
                          + "/" + std::to_string(mouse.Detail()) + "/" + std::to_string(mouse.Buttons()));
      }
    };
  }

  TEST_CASE("View: lays out at the viewport's size", "[UI]")
  {
    View view({400, 300});
    CHECK(view.ViewportSize() == boo::IntSize(400, 300));
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } #half { width: 50% }</style>"
                  u8"<div id=full>a</div><div id=half>b</div>");
    WaitForLoad(view);
    view.UpdateRendering();
    CHECK(WidthOf(view, u8"full") == 400);
    CHECK(WidthOf(view, u8"half") == 200);

    SECTION("A resize lays it out again")
    {
      view.Resize({600, 300});
      view.UpdateRendering();
      CHECK(view.ViewportSize() == boo::IntSize(600, 300));
      CHECK(WidthOf(view, u8"full") == 600);
      CHECK(WidthOf(view, u8"half") == 300);
    }

    SECTION("Loading another document replaces it")
    {
      view.LoadHTML(
        u8"<!DOCTYPE html><style>body { margin: 0 }</style><p id=other style='width: 120px'>c</p>");
      WaitForLoad(view);
      view.UpdateRendering();
      CHECK_FALSE(view.Document().GetElementById(u8"full"));
      CHECK(WidthOf(view, u8"other") == 120);
    }
  }

  TEST_CASE("View: frees a page's document when it loads another, or goes", "[UI]")
  {
    // The pages left aren't kept for going back.
    BackForwardCacheDisabler noCache;

    // A page with style, layout, composited layers, a running animation, a transition and a hovered element:
    // all of it goes with the document.
    const utf8_string page =
      u8"<!DOCTYPE html><style>body { margin: 0 } #box { width: 40px; height: 40px; background: teal;"
      u8" transition: background 1s } #box:hover { background: red }"
      u8"#blurred { width: 40px; height: 20px; filter: blur(2px); background: navy;"
      u8" animation: spin 1s infinite } @keyframes spin { to { transform: rotate(90deg) } }</style>"
      u8"<div id=box></div><div id=blurred></div><p>Some <b>text</b></p>";
    auto time = boo::html::EventLoop::Current().Now();
    auto run = [&](View &view)
    {
      for (int32 i = 0; i < 3; ++i)
      {
        time += 16ms;
        (void)view.Update(time);
      }
    };

    const auto animationsBefore = boo::WebAnimation::Instances().size();
    View view({200, 100});
    view.LoadHTML(utf8_string(page));
    WaitForLoad(view);
    run(view);
    view.MouseMoved({10, 10});
    run(view);
    auto first = CreateWeakPtr<boo::dom::Document>(&view.Document());
    auto box = CreateWeakPtr<boo::dom::Node>(view.Document().GetElementById(u8"box").get());
    REQUIRE(first);

    view.LoadHTML(u8"<!DOCTYPE html><p>Another page</p>");
    WaitForLoad(view);
    run(view);
    CHECK_FALSE(first);
    CHECK_FALSE(box);
    // Its animations and transitions went with it.
    CHECK(boo::WebAnimation::Instances().size() == animationsBefore);

    // A page loaded from files: its style sheets, fonts and images go too.
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    for (int32 i = 0; i < 10; ++i)
    {
      run(view);
    }
    auto editor = CreateWeakPtr<boo::dom::Document>(&view.Document());
    view.LoadHTML(u8"<!DOCTYPE html><p>Another page</p>");
    WaitForLoad(view);
    run(view);
    CHECK_FALSE(editor);

    // Loading it again reuses the cached resources the freed document's elements were clients of.
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    for (int32 i = 0; i < 20; ++i)
    {
      time += 50ms;
      (void)view.Update(time);
    }
    CHECK(view.Document().GetElementById(u8"nonexistent") == nullptr);

    WeakPtr<boo::dom::Document> second;
    {
      View other({200, 100});
      other.LoadHTML(utf8_string(page));
      WaitForLoad(other);
      run(other);
      second = CreateWeakPtr<boo::dom::Document>(&other.Document());
      REQUIRE(second);
    }
    CHECK_FALSE(second);
  }

  TEST_CASE("View: loads files", "[UI]")
  {
    const auto path = std::filesystem::temp_directory_path() / "krystal-ui-page-test.html";
    {
      std::ofstream file(path, std::ios::binary);
      // A byte order mark is decoded away rather than becoming text above the box.
      file << "\xEF\xBB\xBF<!DOCTYPE html><style>body { margin: 0 }</style><div id=box style='width: "
              "75px'>x</div>";
    }

    View view({200, 100});
    REQUIRE(view.LoadFile(path).has_value());
    WaitForLoad(view);
    view.UpdateRendering();
    CHECK(WidthOf(view, u8"box") == 75);
    auto box = view.Document().GetElementById(u8"box");
    REQUIRE(box);
    CHECK(box->OffsetTop() == 0);
    std::filesystem::remove(path);

    CHECK_FALSE(view.LoadFile(path).has_value());
    WaitForLoad(view);
  }

  TEST_CASE("View: defines custom elements in the pages it loads", "[UI]")
  {
    struct Counted : boo::html::CustomElement
    {
      int32 &Constructed;

      Counted(boo::html::HTMLElement &element, int32 &constructed) noexcept
          : CustomElement(element), Constructed(constructed)
      {
        ++Constructed;
      }
    };

    int32 constructed = 0;
    View view({200, 100});
    view.LoadHTML(u8"<!DOCTYPE html><x-counter id=old></x-counter>");
    WaitForLoad(view);
    REQUIRE(view
              .DefineCustomElement(u8"x-counter", [&](boo::html::HTMLElement &element)
                                   { return CreateUnique<Counted>(element, constructed); })
              .has_value());
    CHECK(view
            .DefineCustomElement(u8"x-counter", [](boo::html::HTMLElement &)
                                 { return UniquePtr<boo::html::CustomElement> {}; })
            .error()
          == "x-counter is already defined");
    CHECK_FALSE(view
                  .DefineCustomElement(u8"counter", [](boo::html::HTMLElement &)
                                       { return UniquePtr<boo::html::CustomElement> {}; })
                  .has_value());

    // The current page's elements are upgraded.
    CHECK(constructed == 1);
    CHECK(view.Document().GetElementById(u8"old")->IsDefinedCustomElement());

    // Pages loaded later create them as they're parsed.
    view.LoadHTML(u8"<!DOCTYPE html><x-counter id=a></x-counter><x-counter id=b></x-counter>");
    WaitForLoad(view);
    CHECK(constructed == 3);
    CHECK(view.Document().GetElementById(u8"a")->IsDefinedCustomElement());
    CHECK(view.Document().GetElementById(u8"b")->GetCustomElement() != nullptr);
  }

  TEST_CASE("View: the editor page's bean rating", "[UI]")
  {
    BackForwardCacheDisabler noCache;
    View view({1'200, 1'600});
    REQUIRE(view
              .DefineCustomElement(
                u8"bean-rating", [](boo::html::HTMLElement &element)
                { return CreateUnique<BeanRating>(element); }, BeanRating::Options())
              .has_value());
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    view.UpdateRendering();

    {
      auto found = view.Document().QuerySelector(u8"bean-rating").ReleaseValue();
      REQUIRE(found);
      auto &rating = *found;
      REQUIRE(dynamic_cast<BeanRating *>(rating.GetCustomElement()));
      auto form = view.Document().QuerySelector(u8"form.order").ReleaseValue();
      REQUIRE(form);
      auto &orderForm = Downcast<boo::html::HTMLFormElement>(*form);
      auto submitted = [&]
      {
        auto formData = boo::html::DOMFormData::Create(&orderForm, nullptr).ReleaseValue();
        auto value = formData->Get(u8"rating");
        return value ? Maybe<boo::dom::DOMString>(std::get<boo::dom::DOMString>(*value)) : std::nullopt;
      };

      // Unrated, it's required.
      CHECK_FALSE(rating.Matches(u8":state(rated)").ReleaseValue());
      CHECK(rating.Matches(u8":invalid").ReleaseValue());
      CHECK_FALSE(submitted().has_value());

      // A click on the first bean rates it one.
      auto box = rating.GetBoundingClientRect();
      view.MousePressed({static_cast<int32>(box->X()) + 11, static_cast<int32>(box->Y() + box->Height() / 2)},
                        pal::MouseButton::Left);
      view.MouseReleased(
        {static_cast<int32>(box->X()) + 11, static_cast<int32>(box->Y() + box->Height() / 2)},
        pal::MouseButton::Left);
      CHECK(rating.Matches(u8":state(rated)").ReleaseValue());
      CHECK(rating.Matches(u8":valid").ReleaseValue());
      CHECK(submitted() == u8"1");

      // The click focused it: the arrow keys and digits rate it.
      CHECK(view.Document().ActiveElement().get() == &rating);
      (void)view.KeyPressed(pal::Key::RightArrow);
      CHECK(submitted() == u8"2");
      (void)view.KeyPressed(pal::Key::Num4, {}, {u8"4", u8"4"});
      CHECK(submitted() == u8"4");

      // Reset clears it.
      orderForm.Reset();
      CHECK_FALSE(rating.Matches(u8":state(rated)").ReleaseValue());
      CHECK_FALSE(submitted().has_value());
    }

    // The page goes when another loads.
    auto page = CreateWeakPtr<boo::dom::Document>(&view.Document());
    view.LoadHTML(u8"<!DOCTYPE html><p>Another page</p>");
    WaitForLoad(view);
    boo::html::EventLoop::Current().RunUntilIdle();
    CHECK_FALSE(page);
  }

  TEST_CASE("View: the editor's pages link to each other, and route the guide", "[UI]")
  {
    View view({1'200, 1'600});
    view.SetNavigationHandler([](const NavigationRequest &) { return NavigationPolicy::Allow; });
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    RefPtr<boo::dom::Document> index = ShareRefPtr<boo::dom::Document>(&view.Document());

    SECTION("the settings page, and back from the cache")
    {
      auto link = view.Document().QuerySelector(u8"a[href='settings.html']").ReleaseValue();
      REQUIRE(link);
      link->DispatchSimulatedClick(nullptr);
      WaitForLoad(view);
      CHECK(view.Document().GetURL().LastPathComponent() == u8"settings.html");
      auto name = view.Document().QuerySelector(u8"input[name=name]").ReleaseValue();
      REQUIRE(name);
      (void)Downcast<boo::html::HTMLInputElement>(*name).SetValue(u8"Ada");

      REQUIRE(view.GoBack());
      WaitForLoad(view);
      CHECK(&view.Document() == index.get());

      REQUIRE(view.GoForward());
      WaitForLoad(view);
      name = view.Document().QuerySelector(u8"input[name=name]").ReleaseValue();
      REQUIRE(name);
      CHECK(Downcast<boo::html::HTMLInputElement>(*name).Value() == u8"Ada");
    }

    SECTION("the guide's links can be intercepted")
    {
      // As the editor's router does: the guide's links stay in the page.
      List<std::string> routed;
      auto router = view.Document().Window()->GetNavigation().AddEventListener<boo::NavigateEvent>(
        u8"navigate",
        [&](boo::NavigateEvent &event)
        {
          CHECK(event.CanIntercept());
          boo::NavigateEvent::NavigationInterceptOptions options;
          options.Handler = [&, url = event.Destination()->URL()](auto &&completion)
          {
            routed.push_back(std::string(url.Query().begin(), url.Query().end()));
            completion({});
          };
          (void)event.Intercept(view.Document(), krys::move(options));
        });
      auto topic = view.Document().GetElementById(u8"topic-history");
      REQUIRE(topic);
      topic->DispatchSimulatedClick(nullptr);
      WaitForLoad(view);
      boo::html::EventLoop::Current().RunUntilIdle();
      CHECK(&view.Document() == index.get());
      CHECK(view.Document().GetURL().Query() == u8"guide=history");
      CHECK(view.CanGoBack());

      REQUIRE(view.GoBack());
      WaitForLoad(view);
      boo::html::EventLoop::Current().RunUntilIdle();
      CHECK(&view.Document() == index.get());
      CHECK(view.Document().GetURL().Query().empty());
      CHECK(routed == List<std::string> {"guide=history", ""});
    }
  }

  TEST_CASE("View: the editor page's dialogs, popovers and details", "[UI]")
  {
    View view({1'200, 1'600});
    view.SetNavigationHandler([](const NavigationRequest &) { return NavigationPolicy::Allow; });
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    auto &document = view.Document();
    auto byId = [&](utf8_stringview id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return element;
    };
    auto click = [&](utf8_stringview selectors)
    {
      auto element = document.QuerySelector(selectors).ReleaseValue();
      REQUIRE(element);
      element->DispatchSimulatedClick(nullptr);
      boo::html::EventLoop::Current().RunUntilIdle();
    };
    auto showing = [&](utf8_stringview id)
    {
      return byId(id)->Matches(u8":popover-open").ReleaseValue();
    };

    SECTION("the dialogs open and close through their commands")
    {
      auto &refill = Downcast<boo::html::HTMLDialogElement>(*byId(u8"refill"));
      click(u8"button[commandfor=refill]");
      CHECK(refill.Matches(u8":modal").ReleaseValue());
      click(u8"#refill button[value=yes]");
      CHECK_FALSE(refill.Open());
      CHECK(refill.ReturnValue() == u8"yes");

      // As the editor does for the tip's custom command.
      auto &tip = Downcast<boo::html::HTMLDialogElement>(*byId(u8"tip"));
      List<std::string> commands;
      auto handler = tip.AddEventListener<boo::dom::CommandEvent>(
        u8"command",
        [&](boo::dom::CommandEvent &event)
        {
          commands.push_back(std::string(event.Command().begin(), event.Command().end()));
          if (event.Command() == u8"--show")
          {
            (void)tip.Show();
          }
        });
      click(u8"button[commandfor=tip][command='--show']");
      CHECK(tip.Open());
      CHECK_FALSE(tip.IsModal());
      click(u8"#tip button[command=close]");
      CHECK_FALSE(tip.Open());
      CHECK(tip.ReturnValue() == u8"tip read");
      CHECK(commands == List<std::string> {"--show", "close"});
    }

    SECTION("the popovers")
    {
      click(u8"button[popovertarget=menu]");
      CHECK(showing(u8"menu"));
      // The nested popover keeps the menu open; a hint leaves it open too.
      click(u8"#menu button[popovertarget=menu-more]");
      CHECK(showing(u8"menu"));
      CHECK(showing(u8"menu-more"));
      click(u8"button[popovertarget=hint]");
      CHECK(showing(u8"hint"));
      CHECK(showing(u8"menu"));
      // The toast shows and hides only with its buttons.
      click(u8"button[popovertarget=toast][popovertargetaction=show]");
      CHECK(showing(u8"toast"));
      click(u8"button[popovertarget=toast][popovertargetaction=hide]");
      CHECK_FALSE(showing(u8"toast"));
    }

    SECTION("the details")
    {
      auto details = document.QuerySelectorAll(u8".overlays details").ReleaseValue();
      REQUIRE(details->Length() == 3u);
      auto &fresh = Downcast<boo::html::HTMLDetailsElement>(*details->Item(0));
      auto &local = Downcast<boo::html::HTMLDetailsElement>(*details->Item(1));
      auto &secret = Downcast<boo::html::HTMLDetailsElement>(*details->Item(2));
      CHECK(fresh.Open());
      // The two questions are an accordion.
      click(u8"details[name=faq]:not([open]) summary");
      CHECK(local.Open());
      CHECK_FALSE(fresh.Open());
      // The link to the hidden text opens its details.
      CHECK_FALSE(secret.Open());
      click(u8"a[href='#secret']");
      CHECK(secret.Open());
      // The text fragment link opens the question whose answer it finds, which closes the other.
      click(u8"details[name=faq]:not([open]) summary");
      CHECK(fresh.Open());
      click(u8"a[href^='#:~:text=From']");
      CHECK(local.Open());
      CHECK_FALSE(fresh.Open());
    }

    SECTION("the header's menu is anchored to its button")
    {
      click(u8"#menu-button");
      REQUIRE(showing(u8"page-menu"));
      auto button = byId(u8"menu-button")->GetBoundingClientRect();
      auto menu = byId(u8"page-menu")->GetBoundingClientRect();
      CHECK(menu->X() == Catch::Approx(button->X()));
      CHECK(menu->Y() == Catch::Approx(button->Y() + button->Height() + 4));

      // The nested menu opens to the right of its button, and keeps the first open.
      click(u8"#goto-button");
      CHECK(showing(u8"page-menu"));
      REQUIRE(showing(u8"page-menu-goto"));
      auto gotoButton = byId(u8"goto-button")->GetBoundingClientRect();
      auto gotoMenu = byId(u8"page-menu-goto")->GetBoundingClientRect();
      CHECK(gotoMenu->X() == Catch::Approx(gotoButton->X() + gotoButton->Width() + 4));
      CHECK(gotoMenu->Y() == Catch::Approx(gotoButton->Y()));

      // Escape closes them (opened without user activation, their close watchers are grouped).
      (void)view.KeyPressed(pal::Key::Escape);
      boo::html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(showing(u8"page-menu-goto"));
      CHECK_FALSE(showing(u8"page-menu"));

      // So does a click outside them.
      click(u8"#menu-button");
      click(u8"#goto-button");
      REQUIRE(showing(u8"page-menu-goto"));
      view.MousePressed({4, 4}, pal::MouseButton::Left);
      view.MouseReleased({4, 4}, pal::MouseButton::Left);
      boo::html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(showing(u8"page-menu-goto"));
      CHECK_FALSE(showing(u8"page-menu"));
    }

    SECTION("dialogs fade out in the top layer")
    {
      auto &refill = Downcast<boo::html::HTMLDialogElement>(*byId(u8"refill"));
      auto time = boo::html::EventLoop::Current().Now();
      auto run = [&](MonotonicTime::duration duration)
      {
        for (auto end = time + duration; time < end;)
        {
          time += 16ms;
          (void)view.Update(time);
        }
      };
      click(u8"button[commandfor=refill]");
      run(300ms);
      REQUIRE(refill.Renderer());
      CHECK(refill.Renderer()->AnimatedStyle()->GetOpacity().Value.Value == Catch::Approx(1.0f));

      click(u8"#refill button[value=yes]");
      CHECK_FALSE(refill.Open());
      run(48ms);
      // Closed, it stays rendered in the top layer while it fades out.
      CHECK(refill.IsInTopLayer());
      REQUIRE(refill.Renderer());
      CHECK(refill.Renderer()->AnimatedStyle()->GetOpacity().Value.Value < 1.0f);
      run(300ms);
      CHECK_FALSE(refill.IsInTopLayer());
      CHECK_FALSE(refill.Renderer());
    }
  }

  TEST_CASE("View: the editor page's observers", "[UI]")
  {
    // Images that other cases loaded load at once, from the memory cache.
    boo::MemoryCache::Singleton().EvictResources();
    View view({1'200, 300});
    view.SetNavigationHandler([](const NavigationRequest &) { return NavigationPolicy::Allow; });
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    auto &document = view.Document();
    // As the editor makes them once the page has loaded.
    editor::PageObservers observers(document);
    auto byId = [&](utf8_stringview id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return element;
    };
    auto textOf = [&](utf8_stringview id)
    {
      auto text = byId(id)->TextContent().value_or(boo::dom::DOMString {});
      return std::string(text.begin(), text.end());
    };
    auto update = [&]
    {
      view.UpdateRendering();
      boo::html::EventLoop::Current().RunUntilIdle();
    };
    update();

    SECTION("the resize card shows its size as the view resizes")
    {
      // Its content box: the card less its padding.
      auto card = byId(u8"resize-card");
      auto sizeText = [&]
      {
        return std::format("{} \u00d7 140 pixels", card->OffsetWidth() - 40);
      };
      CHECK(textOf(u8"resize-size") == sizeText());

      auto before = card->OffsetWidth();
      view.Resize({1'000, 300});
      update();
      CHECK(card->OffsetWidth() < before);
      CHECK(textOf(u8"resize-size") == sizeText());
    }

    SECTION("the lazy list's images load as it scrolls")
    {
      auto list = byId(u8"lazy-list");
      auto images = list->QuerySelectorAll(u8"img").ReleaseValue();
      REQUIRE(images->Length() == 21);
      auto &first = Downcast<boo::html::HTMLImageElement>(*images->Item(0));
      auto &last = Downcast<boo::html::HTMLImageElement>(*images->Item(20));

      // Below the page's view, none has loaded.
      CHECK(first.IsDeferred());
      CHECK(textOf(u8"lazy-count") == "No images loaded yet");

      // Scrolled to, those within a list's height of its view load; the rest wait.
      list->ScrollIntoView(true);
      update();
      update();
      CHECK(first.Complete());
      CHECK(last.IsDeferred());
      auto loaded = observers.ImagesLoaded();
      CHECK(loaded > 0);
      CHECK(loaded < 21);
      CHECK(textOf(u8"lazy-count") == std::format("{} of 21 images loaded", loaded));

      list->SetScrollTop(10'000);
      update();
      CHECK_FALSE(last.IsDeferred());
      CHECK(last.Complete());
      CHECK(observers.ImagesLoaded() > loaded);
    }

    SECTION("the chapters skip their contents off screen, and the header's chapter follows the scroll")
    {
      // At the top of the page, no chapter is in view, and each is skipped.
      CHECK(observers.CurrentChapter().empty());
      CHECK_FALSE(byId(u8"chapter-1")->IsRelevantToUser());

      byId(u8"chapter-1")->ScrollIntoView(true);
      update();
      update();
      CHECK(observers.CurrentChapter() == u8"The beans");
      CHECK(byId(u8"chapter-1")->IsRelevantToUser());
      CHECK_FALSE(byId(u8"chapter-6")->IsRelevantToUser());
      auto height = byId(u8"chapter-1")->OffsetHeight();
      CHECK(height != 320);

      // Scrolled past, the first chapter is skipped again, at the size it last had.
      byId(u8"chapter-6")->ScrollIntoView(true);
      update();
      update();
      CHECK(observers.CurrentChapter() == u8"Tasting");
      CHECK(byId(u8"chapter-6")->IsRelevantToUser());
      CHECK_FALSE(byId(u8"chapter-1")->IsRelevantToUser());
      CHECK(byId(u8"chapter-1")->OffsetHeight() == height);
    }
  }

  TEST_CASE("View: the editor page's animations", "[UI]")
  {
    using namespace std::chrono_literals;
    boo::MemoryCache::Singleton().EvictResources();
    View view({1'200, 300});
    view.SetNavigationHandler([](const NavigationRequest &) { return NavigationPolicy::Allow; });
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    auto &document = view.Document();
    // As the editor makes it once the page has loaded.
    editor::PageAnimations animations(document);
    auto byId = [&](utf8_stringview id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return element;
    };
    auto textOf = [&](utf8_stringview id)
    {
      auto text = byId(id)->TextContent().value_or(boo::dom::DOMString {});
      return std::string(text.begin(), text.end());
    };
    auto time = boo::html::EventLoop::Current().Now();
    auto run = [&](MonotonicTime::duration delta)
    {
      constexpr auto frame =
        std::chrono::duration_cast<MonotonicTime::duration>(std::chrono::microseconds {16'667});
      for (auto end = time + delta; time < end;)
      {
        time += frame;
        (void)view.Update(time);
      }
    };
    run(50ms);

    SECTION("the ball's animation from C++, with its controls and readout")
    {
      auto *animation = animations.Animation();
      REQUIRE(animation);
      CHECK(animation->GetPlayState() == boo::WebAnimation::PlayState::Paused);
      CHECK(textOf(u8"anim-time") == "0.00 s of 2 s, paused");

      // Seeking halfway puts the ball halfway along its track (ease-in-out is halfway at half time).
      animations.Seek(0.5);
      run(50ms);
      auto ball = byId(u8"anim-ball");
      auto track = byId(u8"anim-ball")->ParentElement();
      REQUIRE(track);
      CHECK(ball->OffsetLeft() - track->OffsetLeft()
            == Catch::Approx((track->OffsetWidth() - 40) / 2.0).margin(0.5));
      CHECK(textOf(u8"anim-time") == "1.00 s of 2 s, paused");
      CHECK(Downcast<boo::html::HTMLInputElement>(*byId(u8"anim-seek")).ValueAsNumber() == Catch::Approx(50));

      // The buttons play, reverse and pause it; the readout follows on the animation frames.
      Downcast<boo::html::HTMLElement>(*byId(u8"anim-play")).Click();
      run(200ms);
      CHECK(animation->GetPlayState() == boo::WebAnimation::PlayState::Running);
      CHECK(textOf(u8"anim-time").ends_with(", running"));
      auto timeBeforeReversing = animation->CurrentTime()->Time()->count();
      CHECK(timeBeforeReversing > 1);

      Downcast<boo::html::HTMLElement>(*byId(u8"anim-reverse")).Click();
      run(200ms);
      CHECK(animation->PlaybackRate() == -1);
      CHECK(animation->CurrentTime()->Time()->count() < timeBeforeReversing);

      Downcast<boo::html::HTMLElement>(*byId(u8"anim-pause")).Click();
      run(50ms);
      CHECK(animation->GetPlayState() == boo::WebAnimation::PlayState::Paused);
      CHECK(textOf(u8"anim-time").ends_with(", paused"));
    }

    SECTION("the reading bar and the cards follow the page's scroll")
    {
      auto progress = [&]
      {
        auto barAnimations = byId(u8"reading-progress")->GetAnimations();
        REQUIRE(barAnimations.size() == 1);
        auto currentTime = barAnimations[0]->CurrentTime();
        REQUIRE(currentTime);
        REQUIRE(currentTime->Percentage());
        return *currentTime->Percentage();
      };
      auto opacityOf = [&](utf8_stringview id)
      {
        auto *renderer = byId(id)->Renderer();
        REQUIRE(renderer);
        return renderer->Style().GetOpacity().Value.Value;
      };

      // At the top of the page, the bar is empty and the cards below the view haven't come in.
      CHECK(progress() == Catch::Approx(0));
      CHECK(opacityOf(u8"reveal-1") == 0);

      // Scrolled to, a card has come in.
      byId(u8"reveal-1")->ScrollIntoView(true);
      run(50ms);
      CHECK(progress() > 0);
      CHECK(progress() < 100);
      CHECK(opacityOf(u8"reveal-1") == 1);

      // At the bottom of the page, the bar is full.
      view.Document().ScrollingElement()->SetScrollTop(1'000'000);
      run(50ms);
      CHECK(progress() == Catch::Approx(100));
    }
  }

  TEST_CASE("View: the editor page's pointers", "[UI]")
  {
    boo::MemoryCache::Singleton().EvictResources();
    View view({1'200, 600});
    view.SetNavigationHandler([](const NavigationRequest &) { return NavigationPolicy::Allow; });
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    auto &document = view.Document();
    // As the editor makes it once the page has loaded.
    editor::PagePointers pointers(document);
    auto byId = [&](utf8_stringview id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return element;
    };
    auto textOf = [&](utf8_stringview id)
    {
      auto text = byId(id)->TextContent().value_or(boo::dom::DOMString {});
      return std::string(text.begin(), text.end());
    };

    // The pointer cards at the top of the view.
    auto padRect = byId(u8"pointer-pad")->GetBoundingClientRect();
    view.ScrollTo(boo::IntPoint(0, view.ScrollPosition().Y() + static_cast<int32>(padRect->Top()) - 20));
    view.UpdateRendering();
    auto centre = [&](utf8_stringview id)
    {
      auto rect = byId(id)->GetBoundingClientRect();
      return boo::IntPoint(static_cast<int32>(rect->Left() + rect->Width() / 2),
                           static_cast<int32>(rect->Top() + rect->Height() / 2));
    };
    auto touchInput = [](pal::PointerAction action, uint32 id)
    {
      pal::PointerInput input;
      input.Id = id;
      input.Type = pal::PointerType::Touch;
      input.Action = action;
      input.InContact = action == pal::PointerAction::Pressed || action == pal::PointerAction::Moved;
      input.Pressure = 0.5f;
      return input;
    };
    CHECK(textOf(u8"pointer-readout") == "Move a mouse, a pen or a finger over the pad.\nTouches held: 0");

    SECTION("the pad shows the pointers over it and the touches it holds, which don't pinch")
    {
      auto pad = centre(u8"pointer-pad");
      view.MouseMoved(pad);
      CHECK(textOf(u8"pointer-readout").starts_with("mouse 1 at "));

      auto first = boo::DoublePoint(pad.X() - 40, pad.Y());
      auto second = boo::DoublePoint(pad.X() + 40, pad.Y());
      (void)view.TouchInput(first, touchInput(pal::PointerAction::Pressed, 10));
      (void)view.TouchInput(second, touchInput(pal::PointerAction::Pressed, 11));
      (void)view.TouchInput(boo::DoublePoint(pad.X() + 120, pad.Y()),
                            touchInput(pal::PointerAction::Moved, 11));
      auto readout = textOf(u8"pointer-readout");
      CHECK(readout.contains("\ntouch 3 at "));
      CHECK(readout.contains("pressure 0.50"));
      CHECK(readout.contains("\ntouch 4 at "));
      CHECK(readout.ends_with("Touches held: 2"));
      CHECK(view.GetPage().PageScaleFactor() == 1);

      (void)view.TouchInput(first, touchInput(pal::PointerAction::Released, 10));
      (void)view.TouchInput(second, touchInput(pal::PointerAction::Released, 11));
      readout = textOf(u8"pointer-readout");
      CHECK_FALSE(readout.contains("touch"));
      CHECK(readout.ends_with("Touches held: 0"));
    }

    SECTION("the chip follows the pointer that captured it, within its area")
    {
      auto chip = centre(u8"drag-chip");
      view.MouseMoved(chip);
      // The capture takes hold at the pointer's next event.
      view.MousePressed(chip, pal::MouseButton::Left);
      view.MouseMoved(boo::IntPoint(chip.X() + 50, chip.Y() + 30));
      CHECK(textOf(u8"drag-chip") == "Dragging (pointer 1)");
      CHECK(byId(u8"drag-chip")->GetAttribute(u8"style") == boo::dom::DOMString(u8"left: 50px; top: 30px"));
      // Far outside its area, it stops at the area's edge.
      view.MouseMoved(boo::IntPoint(chip.X() - 500, chip.Y() + 1'000));
      auto style = byId(u8"drag-chip")->GetAttribute(u8"style").value_or(boo::dom::DOMString());
      CHECK(std::string(style.begin(), style.end()).starts_with("left: 0px; top: "));
      view.MouseReleased(boo::IntPoint(chip.X() - 500, chip.Y() + 1'000), pal::MouseButton::Left);
      CHECK(textOf(u8"drag-chip") == "Drag me");
    }

    SECTION("the box locks the pointer, and shows the movement it gets")
    {
      CHECK(textOf(u8"lock-readout") == "Click the box to lock the pointer to it.");
      view.SetPointerLockHandler([](bool) { return true; });
      auto box = centre(u8"lock-box");
      view.MouseMoved(box);
      view.MousePressed(box, pal::MouseButton::Left);
      view.MouseReleased(box, pal::MouseButton::Left);
      boo::html::EventLoop::Current().RunUntilIdle();
      CHECK(view.PointerLocked());
      CHECK(textOf(u8"lock-readout") == "Locked: move the mouse. Escape unlocks it.");
      view.LockedMouseMoved({5, -3}, {5, -3});
      view.LockedMouseMoved({2, 1}, {2, 1});
      CHECK(textOf(u8"lock-readout") == "Locked: moved 2, 1; 7, -2 in all. Escape unlocks it.");
      (void)view.KeyPressed(pal::Key::Escape);
      boo::html::EventLoop::Current().RunUntilIdle();
      CHECK(textOf(u8"lock-readout") == "Click the box to lock the pointer to it.");
    }

    SECTION("the tiles pan with a finger, and a tap clicks them")
    {
      auto area = centre(u8"pan-area");
      (void)view.TouchInput(boo::DoublePoint(area), touchInput(pal::PointerAction::Pressed, 1));
      (void)view.TouchInput(boo::DoublePoint(area), touchInput(pal::PointerAction::Released, 1));
      CHECK(textOf(u8"gesture-log").starts_with("click from touch at "));

      (void)view.TouchInput(boo::DoublePoint(area), touchInput(pal::PointerAction::Pressed, 1));
      (void)view.TouchInput(boo::DoublePoint(area.X() - 30, area.Y() - 40),
                            touchInput(pal::PointerAction::Moved, 1));
      (void)view.TouchInput(boo::DoublePoint(area.X() - 30, area.Y() - 40),
                            touchInput(pal::PointerAction::Released, 1));
      CHECK(byId(u8"pan-area")->ScrollLeft() == 30);
      CHECK(byId(u8"pan-area")->ScrollTop() == 40);
    }
  }

  TEST_CASE("View: the editor's settings page asks before saving", "[UI]")
  {
    View view({1'200, 800});
    List<NavigationRequest> submissions;
    view.SetNavigationHandler(
      [&](const NavigationRequest &request)
      {
        if (request.IsFormSubmission)
        {
          submissions.push_back(request);
          return NavigationPolicy::Ignore;
        }
        return NavigationPolicy::Allow;
      });
    REQUIRE(
      view
        .LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "settings.html")
        .has_value());
    WaitForLoad(view);
    auto &document = view.Document();
    auto confirm = document.GetElementById(u8"confirm-save");
    REQUIRE(confirm);
    auto &dialog = Downcast<boo::html::HTMLDialogElement>(*confirm);
    auto &form = Downcast<boo::html::HTMLFormElement>(*document.GetElementById(u8"settings"));
    // As the editor does: the form is submitted when the dialog closes with "save".
    auto closer = dialog.AddEventListener(u8"close",
                                          [&](boo::dom::Event &)
                                          {
                                            if (dialog.ReturnValue() == u8"save")
                                            {
                                              (void)form.RequestSubmit();
                                            }
                                            dialog.ReturnValue({});
                                          });
    auto click = [&](utf8_stringview selectors)
    {
      auto element = document.QuerySelector(selectors).ReleaseValue();
      REQUIRE(element);
      element->DispatchSimulatedClick(nullptr);
      boo::html::EventLoop::Current().RunUntilIdle();
      (void)view.Update(boo::html::EventLoop::Current().Now());
    };

    // Save shows the dialog modally, over the page.
    click(u8".settings button[command=show-modal]");
    REQUIRE(dialog.Matches(u8":modal").ReleaseValue());
    CHECK(document.ActiveElement().get()
          == document.QuerySelector(u8"#confirm-save button[value=save]").Value().get());

    // Escape cancels; so does Cancel.
    (void)view.KeyPressed(pal::Key::Escape);
    boo::html::EventLoop::Current().RunUntilIdle();
    CHECK_FALSE(dialog.Open());
    click(u8".settings button[command=show-modal]");
    click(u8"#confirm-save button[value=cancel]");
    CHECK_FALSE(dialog.Open());
    CHECK(submissions.empty());

    // Saving submits the form.
    click(u8".settings button[command=show-modal]");
    click(u8"#confirm-save button[value=save]");
    CHECK_FALSE(dialog.Open());
    REQUIRE(submissions.size() == 1);
    CHECK(submissions[0].SourceElement.get() == &form);
    CHECK(submissions[0].URL.starts_with(u8"https://example.com/settings?"));

    // Escape after a save doesn't save again.
    click(u8".settings button[command=show-modal]");
    (void)view.KeyPressed(pal::Key::Escape);
    boo::html::EventLoop::Current().RunUntilIdle();
    (void)view.Update(boo::html::EventLoop::Current().Now());
    CHECK(submissions.size() == 1);
  }

  TEST_CASE("View: records its painting", "[UI]")
  {
    View view({200, 100});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 }</style>"
                  u8"<div style='width: 40px; height: 20px; background: red'></div>");
    WaitForLoad(view);
    const auto displayList = view.Record();
    // The view's base colour, the root's background over the canvas and the box's background, clipped to the
    // visible contents.
    REQUIRE(displayList->Items().size() == 8u);
    CHECK(displayList->AsText({}).starts_with(u8R"(
(save)
(clip
  (rect at (0,0) size 200x100)))"));
    CHECK(displayList->AsText({}).ends_with(u8R"((fill-composited-rect
  (rect at (0,0) size 40x20)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal))
(restore))"));
  }

  TEST_CASE("View: hit tests, hovers and activates", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }"
      u8"#a:hover { width: 100px } #a:active { width: 50px }</style><div id=a></div><div id=b></div>");
    WaitForLoad(view);
    auto a = view.Document().GetElementById(u8"a");
    auto b = view.Document().GetElementById(u8"b");
    CHECK(view.HitTest({10, 10}) == a.get());
    CHECK(view.HitTest({10, 60}) == b.get());

    view.MouseMoved({10, 10});
    view.UpdateRendering();
    CHECK(a->Hovered());
    CHECK(WidthOf(view, u8"a") == 100);

    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.UpdateRendering();
    CHECK(a->Active());
    CHECK(WidthOf(view, u8"a") == 50);

    // While the button is down, only the active chain stays hovered.
    view.MouseMoved({10, 60});
    CHECK_FALSE(b->Hovered());
    view.MouseReleased({10, 60}, pal::MouseButton::Left);
    CHECK_FALSE(a->Active());
    view.MouseMoved({10, 10});
    CHECK(a->Hovered());
    view.MouseMoved({10, 60});
    CHECK(b->Hovered());
    CHECK_FALSE(a->Hovered());

    view.MouseLeft();
    CHECK_FALSE(b->Hovered());
  }

  TEST_CASE("View: a click that doesn't move doesn't select", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }</style><div></div>");
    WaitForLoad(view);
    view.UpdateRendering();

    view.MouseMoved({10, 10});
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);

    // Windows reports a move with each press and release, at the same place.
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseMoved({10, 10});
    view.UpdateRendering();
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);
    view.MouseReleased({10, 10}, pal::MouseButton::Left);
    view.MouseMoved({10, 10});
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);
    CHECK_FALSE(view.Document().Selection().IsRange());
  }

  TEST_CASE("View: dragging a selection shows the I-beam only over text", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif } div { height: 50px }"
                  u8"</style><div></div><div>text</div><div></div>");
    WaitForLoad(view);
    view.UpdateRendering();

    view.MouseMoved({10, 10});
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseMoved({10, 120});
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);
    view.MouseMoved({3, 60});
    CHECK(view.CurrentCursor() == pal::CursorShape::IBeam);
    view.MouseMoved({300, 60});
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);
    view.MouseReleased({300, 60}, pal::MouseButton::Left);
    CHECK(view.Document().Selection().IsRange());
  }

  TEST_CASE("View: dispatches mouse events and chooses the cursor", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif } div { height: 50px }"
                  u8"#b { cursor: pointer }</style><div id=a>text</div><div id=b></div>");
    WaitForLoad(view);
    view.UpdateRendering();
    List<std::string> records;
    Ref<boo::dom::EventListenerCallback> recorder = CreateRef<ClickRecorder>(records);
    for (auto type : {u8"mousedown", u8"mouseup", u8"click", u8"auxclick", u8"dblclick"})
    {
      view.Document().AddEventListener(boo::dom::DOMStringAtom(type), Ref {recorder}, false);
    }

    // The cursor follows the mouse: the arrow over a box, the I-beam over text and the style's cursor.
    view.MouseMoved({300, 10});
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);
    view.MouseMoved({3, 10});
    CHECK(view.CurrentCursor() == pal::CursorShape::IBeam);
    view.MouseMoved({10, 60});
    CHECK(view.CurrentCursor() == pal::CursorShape::Hand);

    // Two quick presses in the same place are a double click; the buttons held are reported.
    view.MousePressed({10, 60}, pal::MouseButton::Left);
    view.MouseReleased({10, 60}, pal::MouseButton::Left);
    view.MousePressed({11, 61}, pal::MouseButton::Left);
    view.MouseReleased({11, 61}, pal::MouseButton::Left);
    CHECK(records
          == List<std::string> {"mousedown:b/1/1", "mouseup:b/1/0", "click:b/1/0", "mousedown:b/2/1",
                                "mouseup:b/2/0", "click:b/2/0", "dblclick:b/2/0"});
    records.clear();

    // A press far from the last starts counting again, as does another button.
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseReleased({10, 10}, pal::MouseButton::Left);
    view.MousePressed({10, 10}, pal::MouseButton::Right);
    view.MouseReleased({10, 10}, pal::MouseButton::Right);
    CHECK(records
          == List<std::string> {"mousedown:a/1/1", "mouseup:a/1/0", "click:a/1/0", "mousedown:a/1/2",
                                "mouseup:a/1/0", "auxclick:a/1/0"});
  }

  TEST_CASE("View: pointer lock", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }</style>"
                  u8"<div id=a></div><div id=b></div>");
    WaitForLoad(view);
    view.UpdateRendering();
    auto &a = *view.Document().GetElementById(u8"a");

    // Each event as `type:id`, a mouse move's with its movement, and the handler's calls as `lock` and
    // `unlock`.
    List<std::string> records;
    auto record = [&](boo::dom::Event &event)
    {
      auto type = event.Type().View();
      auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
      auto *element = node ? DynamicDowncast<boo::dom::Element>(*node) : nullptr;
      auto id = element ? element->GetAttribute(u8"id").value_or(boo::dom::DOMString())
                        : boo::dom::DOMString(u8"document");
      std::string text = std::string(type.begin(), type.end()) + ":" + std::string(id.begin(), id.end());
      if (event.Type() == boo::EventNames::MouseMove)
      {
        auto &mouse = static_cast<boo::dom::MouseEvent &>(event);
        text += "/" + std::to_string(static_cast<int>(mouse.MovementX())) + ","
                + std::to_string(static_cast<int>(mouse.MovementY()));
      }
      records.push_back(text);
    };
    List<boo::dom::EventListenerHandle> handles;
    for (auto type : {u8"pointerlockchange", u8"pointerlockerror", u8"mousemove"})
    {
      handles.push_back(view.Document().AddEventListener(boo::dom::DOMStringAtom(type),
                                                         Func<void(boo::dom::Event &)>(record)));
    }

    // A press on `a` asks to lock the pointer to it (a press is a user gesture).
    boo::dom::PointerLockOptions options;
    handles.push_back(a.AddEventListener(
      boo::dom::DOMStringAtom(u8"mousedown"),
      Func<void(boo::dom::Event &)>([&](boo::dom::Event &) { a.RequestPointerLock(options); })));
    auto click = [&]
    {
      view.MousePressed({10, 10}, pal::MouseButton::Left);
      view.MouseReleased({10, 10}, pal::MouseButton::Left);
      boo::html::EventLoop::Current().RunUntilIdle();
    };

    view.MouseMoved({10, 10});
    records.clear();

    // Without a handler, there's no pointer lock.
    click();
    CHECK(records == List<std::string> {"pointerlockerror:document"});
    CHECK_FALSE(view.PointerLocked());
    records.clear();

    bool hostLocks = true;
    view.SetPointerLockHandler(
      [&](bool locked)
      {
        records.push_back(locked ? "lock" : "unlock");
        return hostLocks;
      });

    SECTION("locking, the movement, and Escape unlocking")
    {
      click();
      CHECK(records == List<std::string> {"lock", "pointerlockchange:document"});
      CHECK(view.PointerLocked());
      CHECK(view.CurrentCursor() == pal::CursorShape::Hidden);
      CHECK(view.Document().PointerLockElement().get() == &a);
      records.clear();

      // The movement goes to `a`, with the system's acceleration unless the page asked for the device's own.
      view.LockedMouseMoved({4, -2}, {2, -1});
      CHECK(records == List<std::string> {"mousemove:a/4,-2"});
      records.clear();
      options.UnadjustedMovement = true;
      click();
      view.LockedMouseMoved({4, -2}, {2, -1});
      CHECK(records == List<std::string> {"pointerlockchange:document", "mousemove:a/2,-1"});
      records.clear();

      (void)view.KeyPressed(pal::Key::Escape);
      boo::html::EventLoop::Current().RunUntilIdle();
      CHECK(records == List<std::string> {"unlock", "pointerlockchange:document"});
      CHECK_FALSE(view.PointerLocked());
      CHECK(view.CurrentCursor() != pal::CursorShape::Hidden);
      records.clear();

      // Unlocked, moves are the mouse's and carry how far it went.
      view.LockedMouseMoved({4, -2}, {2, -1});
      view.MouseMoved({15, 12});
      CHECK(records == List<std::string> {"mousemove:a/5,2"});
    }

    SECTION("the window losing focus unlocks it, and a window without focus can't lock it")
    {
      click();
      records.clear();
      view.SetFocused(false);
      boo::html::EventLoop::Current().RunUntilIdle();
      CHECK(records == List<std::string> {"unlock", "pointerlockchange:document"});
      CHECK_FALSE(view.PointerLocked());
      records.clear();

      click();
      CHECK(records == List<std::string> {"pointerlockerror:document"});
    }

    SECTION("a host that doesn't lock it")
    {
      hostLocks = false;
      click();
      CHECK(records == List<std::string> {"lock", "pointerlockerror:document"});
      CHECK_FALSE(view.PointerLocked());
    }
  }

  TEST_CASE("View: pens and touches", "[UI]")
  {
    View view({400, 300});
    // The page takes its touches, so they make no gestures.
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px; touch-action: none }</style>"
      u8"<div id=a></div><div id=b></div>");
    WaitForLoad(view);
    view.UpdateRendering();

    // Each event as `type:id/buttons`, a pointer event's with its type, id and pressure, and a touch event's
    // with its touches.
    List<std::string> records;
    auto record = [&](boo::dom::Event &event)
    {
      auto type = event.Type().View();
      auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
      auto *element = node ? DynamicDowncast<boo::dom::Element>(*node) : nullptr;
      auto id =
        element ? element->GetAttribute(u8"id").value_or(boo::dom::DOMString()) : boo::dom::DOMString();
      std::string text = std::string(type.begin(), type.end()) + ":" + std::string(id.begin(), id.end());
      if (auto *pointer = DynamicDowncast<boo::dom::PointerEvent>(event))
      {
        auto pointerType = pointer->PointerType();
        text += "/" + std::string(pointerType.begin(), pointerType.end())
                + std::to_string(pointer->PointerId()) + "/" + std::to_string(pointer->Buttons()) + "/"
                + std::to_string(static_cast<int>(pointer->Pressure() * 100));
      }
      else if (auto *touch = DynamicDowncast<boo::dom::TouchEvent>(event))
      {
        text += "/" + std::to_string(touch->Touches()->Length()) + ","
                + std::to_string(touch->ChangedTouches()->Length());
      }
      else if (auto *mouse = DynamicDowncast<boo::dom::MouseEvent>(event))
      {
        text += "/" + std::to_string(mouse->Buttons());
      }
      records.push_back(text);
    };
    List<boo::dom::EventListenerHandle> handles;
    for (auto type : {u8"pointerdown", u8"pointermove", u8"pointerup", u8"pointercancel", u8"mousedown",
                      u8"mouseup", u8"click", u8"touchstart", u8"touchmove", u8"touchend", u8"touchcancel"})
    {
      handles.push_back(view.Document().AddEventListener(boo::dom::DOMStringAtom(type),
                                                         Func<void(boo::dom::Event &)>(record)));
    }

    auto input = [](pal::PointerType type, pal::PointerAction action, uint32 id = 7)
    {
      pal::PointerInput input;
      input.Id = id;
      input.Type = type;
      input.Action = action;
      input.InContact = action == pal::PointerAction::Pressed || action == pal::PointerAction::Moved;
      return input;
    };

    SECTION("a pen hovers, presses and releases as the mouse does, with its pressure and tilt")
    {
      RefPtr<boo::dom::PointerEvent> down;
      handles.push_back(view.Document().AddEventListener(
        boo::dom::DOMStringAtom(u8"pointerdown"),
        Func<void(boo::dom::Event &)>([&](boo::dom::Event &event)
                                      { down = ShareRefPtr(&Downcast<boo::dom::PointerEvent>(event)); })));

      auto hover = input(pal::PointerType::Pen, pal::PointerAction::Moved);
      hover.InContact = false;
      view.PenInput({10, 10}, hover);
      auto press = input(pal::PointerType::Pen, pal::PointerAction::Pressed);
      press.Pressure = 0.75f;
      press.TiltX = 20;
      press.Twist = 90;
      view.PenInput({10, 10}, press);
      view.PenInput({10, 10}, input(pal::PointerType::Pen, pal::PointerAction::Released));
      CHECK(records
            == List<std::string> {"pointermove:a/pen2/0/0", "pointerdown:a/pen2/1/75", "mousedown:a/1",
                                  "pointerup:a/pen2/0/0", "mouseup:a/0", "click:a/pen2/0/0"});
      REQUIRE(down);
      CHECK(down->TiltX() == 20);
      CHECK(down->Twist() == 90);
      records.clear();

      // The barrel button presses the right button, the eraser the eraser button; a pen without pressure
      // presses at half pressure.
      auto barrel = input(pal::PointerType::Pen, pal::PointerAction::Pressed);
      barrel.BarrelButton = true;
      view.PenInput({10, 60}, barrel);
      view.PenInput({10, 60}, input(pal::PointerType::Pen, pal::PointerAction::Released));
      auto eraser = input(pal::PointerType::Pen, pal::PointerAction::Pressed);
      eraser.Eraser = true;
      view.PenInput({10, 60}, eraser);
      CHECK(down->Button() == 5);
      view.PenInput({10, 60}, input(pal::PointerType::Pen, pal::PointerAction::Released));
      CHECK(records
            == List<std::string> {"pointerdown:b/pen2/2/50", "mousedown:b/2", "pointerup:b/pen2/0/0",
                                  "mouseup:b/0", "pointerdown:b/pen2/32/50", "mousedown:b/32",
                                  "pointerup:b/pen2/0/0", "mouseup:b/0"});
    }

    SECTION("touches: ids from the lowest free, every touch in the events, and no mouse events")
    {
      auto first = input(pal::PointerType::Touch, pal::PointerAction::Pressed, 100);
      first.Width = 10;
      first.Height = 8;
      first.Pressure = 0.5f;
      (void)view.TouchInput({10, 10}, first);
      (void)view.TouchInput({10, 60}, input(pal::PointerType::Touch, pal::PointerAction::Pressed, 200));
      CHECK(view.TouchCount() == 2);
      (void)view.TouchInput({20, 60}, input(pal::PointerType::Touch, pal::PointerAction::Moved, 200));
      (void)view.TouchInput({10, 10}, input(pal::PointerType::Touch, pal::PointerAction::Released, 100));
      // The first touch's id is free again.
      (void)view.TouchInput({10, 10}, input(pal::PointerType::Touch, pal::PointerAction::Pressed, 300));
      (void)view.TouchInput({}, input(pal::PointerType::Touch, pal::PointerAction::Cancelled, 200));
      (void)view.TouchInput({}, input(pal::PointerType::Touch, pal::PointerAction::Cancelled, 300));
      CHECK(view.TouchCount() == 0);
      CHECK(records
            == List<std::string> {"pointerdown:a/touch3/1/50", "touchstart:a/1,1",
                                  "pointermove:a/touch3/1/50", "pointerdown:b/touch4/1/50",
                                  "touchstart:b/2,1", "pointermove:a/touch3/1/50",
                                  "pointermove:b/touch4/1/50", "touchmove:b/2,1", "pointerup:a/touch3/0/0",
                                  "pointermove:b/touch4/1/50", "touchend:a/1,1", "pointermove:b/touch4/1/50",
                                  "pointerdown:a/touch3/1/50", "touchstart:a/2,1",
                                  "pointercancel:b/touch4/0/0", "pointermove:a/touch3/1/50",
                                  "touchcancel:b/0,1", "pointercancel:a/touch3/0/0", "touchcancel:a/0,1"});
    }

    SECTION("the pointing devices: media features and maxTouchPoints")
    {
      auto &window = *view.Document().Window();
      CHECK(window.MatchMedia(u8"(pointer: fine) and (hover: hover)")->Matches());
      CHECK(window.GetNavigator().MaxTouchPoints() == 0);

      view.SetPointingDevices({.Mouse = false, .Pen = false, .Touch = true, .MaxTouchPoints = 10});
      CHECK(window.MatchMedia(u8"(pointer: coarse) and (hover: none)")->Matches());
      CHECK(window.MatchMedia(u8"(any-pointer: coarse) and (any-hover: none)")->Matches());
      CHECK_FALSE(window.MatchMedia(u8"(any-pointer: fine)")->Matches());
      CHECK(window.GetNavigator().MaxTouchPoints() == 10);

      view.SetPointingDevices({.Mouse = true, .Pen = true, .Touch = true, .MaxTouchPoints = 5});
      CHECK(
        window.MatchMedia(u8"(pointer: fine) and (any-pointer: coarse) and (any-pointer: fine)")->Matches());
      CHECK(window.MatchMedia(u8"(any-hover: hover)")->Matches());
      CHECK(window.GetNavigator().MaxTouchPoints() == 5);
    }
  }

  TEST_CASE("View: gestures", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }"
      u8"#none { touch-action: none } #panx { touch-action: pan-x } #tall { width: 2000px; height: 2000px }"
      u8"</style><div id=a></div><div id=none></div><div id=panx></div><div id=tall></div>");
    WaitForLoad(view);
    view.UpdateRendering();
    auto &loop = boo::html::EventLoop::Current();
    auto now = loop.Now();
    auto frame = [&](std::chrono::milliseconds delta = std::chrono::milliseconds(16))
    {
      now += delta;
      (void)view.Update(now);
    };

    // Each event as `type:id`, a pointer event's with its type and id.
    List<std::string> records;
    auto record = [&](boo::dom::Event &event)
    {
      auto type = event.Type().View();
      auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
      auto *element = node ? DynamicDowncast<boo::dom::Element>(*node) : nullptr;
      auto id =
        element ? element->GetAttribute(u8"id").value_or(boo::dom::DOMString()) : boo::dom::DOMString();
      std::string text = std::string(type.begin(), type.end()) + ":" + std::string(id.begin(), id.end());
      if (auto *pointer = DynamicDowncast<boo::dom::PointerEvent>(event))
      {
        auto pointerType = pointer->PointerType();
        text +=
          "/" + std::string(pointerType.begin(), pointerType.end()) + std::to_string(pointer->PointerId());
      }
      records.push_back(text);
    };
    List<boo::dom::EventListenerHandle> handles;
    for (auto type : {u8"pointerdown", u8"pointerup", u8"pointercancel", u8"mousedown", u8"mouseup",
                      u8"click", u8"dblclick", u8"contextmenu"})
    {
      handles.push_back(view.Document().AddEventListener(boo::dom::DOMStringAtom(type),
                                                         Func<void(boo::dom::Event &)>(record)));
    }
    // On the document, touchmove's listeners are passive unless they say otherwise; on `tall` they aren't.
    auto cancel = [&](utf8_stringview type, bool onTall = false)
    {
      boo::dom::EventTarget &target =
        onTall ? static_cast<boo::dom::EventTarget &>(*view.Document().GetElementById(u8"tall"))
               : view.Document();
      handles.push_back(target.AddEventListener(
        boo::dom::DOMStringAtom(type),
        Func<void(boo::dom::Event &)>([](boo::dom::Event &event) { event.PreventDefault(); })));
    };

    auto touch = [&](pal::PointerAction action, boo::DoublePoint point, uint32 id = 1)
    {
      pal::PointerInput input;
      input.Id = id;
      input.Type = pal::PointerType::Touch;
      input.Action = action;
      input.InContact = action == pal::PointerAction::Pressed || action == pal::PointerAction::Moved;
      (void)view.TouchInput(point, input);
    };
    auto tap = [&](boo::DoublePoint point)
    {
      touch(pal::PointerAction::Pressed, point);
      touch(pal::PointerAction::Released, point);
    };

    SECTION("a tap clicks, with its compatibility mouse events; two quickly, a double click")
    {
      tap({10, 10});
      CHECK(records
            == List<std::string> {"pointerdown:a/touch3", "pointerup:a/touch3", "mousedown:a", "mouseup:a",
                                  "click:a/touch3"});
      records.clear();
      tap({14, 12});
      CHECK(records
            == List<std::string> {"pointerdown:a/touch3", "pointerup:a/touch3", "mousedown:a", "mouseup:a",
                                  "click:a/touch3", "dblclick:a"});
    }

    SECTION("a cancelled pointerdown stops the tap's compatibility mouse events, not its click")
    {
      cancel(u8"pointerdown");
      tap({10, 10});
      CHECK(records == List<std::string> {"pointerdown:a/touch3", "pointerup:a/touch3", "click:a/touch3"});
    }

    SECTION("a cancelled touchend stops the click too")
    {
      cancel(u8"touchend");
      tap({10, 10});
      CHECK(records == List<std::string> {"pointerdown:a/touch3", "pointerup:a/touch3"});
    }

    SECTION("a long press asks for a context menu as it lifts, and doesn't tap")
    {
      touch(pal::PointerAction::Pressed, {10, 10});
      frame(std::chrono::milliseconds(400));
      CHECK(records == List<std::string> {"pointerdown:a/touch3"});
      frame(std::chrono::milliseconds(200));
      touch(pal::PointerAction::Released, {10, 10});
      CHECK(records
            == List<std::string> {"pointerdown:a/touch3", "pointerup:a/touch3", "contextmenu:a/touch3"});
    }

    SECTION("the right button asks for a context menu too, after its press")
    {
      view.MousePressed({10, 10}, pal::MouseButton::Right);
      view.MouseReleased({10, 10}, pal::MouseButton::Right);
      CHECK(records
            == List<std::string> {"pointerdown:a/mouse1", "mousedown:a", "contextmenu:a/mouse1",
                                  "pointerup:a/mouse1", "mouseup:a"});
    }

    SECTION("a pan scrolls what's under it, after pointercancel, then flings on")
    {
      touch(pal::PointerAction::Pressed, {100, 200});
      // Within the threshold, nothing scrolls.
      touch(pal::PointerAction::Moved, {100, 195});
      CHECK(view.ScrollPosition() == boo::IntPoint());
      touch(pal::PointerAction::Moved, {100, 150});
      CHECK(records == List<std::string> {"pointerdown:tall/touch3", "pointercancel:tall/touch3"});
      CHECK(view.ScrollPosition() == boo::IntPoint(0, 50));
      frame();
      touch(pal::PointerAction::Moved, {100, 100});
      CHECK(view.ScrollPosition() == boo::IntPoint(0, 100));
      touch(pal::PointerAction::Released, {100, 100});
      // No pointerup or click follows the cancelled pointer.
      CHECK(records == List<std::string> {"pointerdown:tall/touch3", "pointercancel:tall/touch3"});

      // It flung at 100 pixels in 16ms, and slows as the frames go.
      frame();
      auto first = view.ScrollPosition().Y();
      CHECK(first > 100);
      frame();
      auto second = view.ScrollPosition().Y();
      CHECK(second - first > 0);
      CHECK(second - first < first - 100);
      frame(std::chrono::milliseconds(5'000));
      auto stopped = view.ScrollPosition();
      frame();
      CHECK(view.ScrollPosition() == stopped);
    }

    SECTION("a touch stops the fling, and doesn't tap")
    {
      touch(pal::PointerAction::Pressed, {100, 200});
      touch(pal::PointerAction::Moved, {100, 150});
      frame();
      touch(pal::PointerAction::Moved, {100, 100});
      touch(pal::PointerAction::Released, {100, 100});
      frame();
      records.clear();
      tap({100, 100});
      auto stopped = view.ScrollPosition();
      frame();
      CHECK(view.ScrollPosition() == stopped);
      CHECK(records == List<std::string> {"pointerdown:tall/touch3", "pointerup:tall/touch3"});
    }

    SECTION("touch-action: none leaves the touch to the page")
    {
      touch(pal::PointerAction::Pressed, {10, 60});
      touch(pal::PointerAction::Moved, {10, 160});
      touch(pal::PointerAction::Released, {10, 160});
      CHECK(view.ScrollPosition() == boo::IntPoint());
      CHECK(records == List<std::string> {"pointerdown:none/touch3", "pointerup:none/touch3"});
    }

    SECTION("touch-action: pan-x pans sideways only")
    {
      // Upwards, it's the page's.
      touch(pal::PointerAction::Pressed, {100, 125});
      touch(pal::PointerAction::Moved, {100, 60});
      touch(pal::PointerAction::Moved, {40, 60});
      touch(pal::PointerAction::Released, {40, 60});
      CHECK(view.ScrollPosition() == boo::IntPoint());

      // Sideways, it pans, along the one axis.
      touch(pal::PointerAction::Pressed, {100, 125});
      touch(pal::PointerAction::Moved, {40, 115});
      CHECK(view.ScrollPosition() == boo::IntPoint(60, 0));
      touch(pal::PointerAction::Released, {40, 115});
    }

    SECTION("a cancelled touchmove takes the touch from the pan")
    {
      cancel(u8"touchmove", true);
      touch(pal::PointerAction::Pressed, {100, 200});
      touch(pal::PointerAction::Moved, {100, 150});
      touch(pal::PointerAction::Released, {100, 150});
      CHECK(view.ScrollPosition() == boo::IntPoint());
      CHECK(records == List<std::string> {"pointerdown:tall/touch3", "pointerup:tall/touch3"});
    }

    SECTION("two touches pinch, scaling the page about the point between them")
    {
      touch(pal::PointerAction::Pressed, {100, 200}, 1);
      touch(pal::PointerAction::Pressed, {200, 200}, 2);
      CHECK(records
            == List<std::string> {"pointerdown:tall/touch3", "pointerdown:tall/touch4",
                                  "pointercancel:tall/touch3", "pointercancel:tall/touch4"});

      // Twice as far apart: twice the scale, with the document's (150, 200) between them at (200, 200).
      touch(pal::PointerAction::Moved, {300, 200}, 2);
      CHECK(view.GetPage().PageScaleFactor() == 2);
      CHECK(view.ScrollPosition() == boo::IntPoint(100, 200));

      // The scale has its limits.
      touch(pal::PointerAction::Moved, {1'000, 200}, 2);
      CHECK(view.GetPage().PageScaleFactor() == 5);
      touch(pal::PointerAction::Moved, {110, 200}, 2);
      CHECK(view.GetPage().PageScaleFactor() == 1);
      touch(pal::PointerAction::Released, {110, 200}, 2);
      touch(pal::PointerAction::Released, {100, 200}, 1);
      CHECK(view.GetPage().PageScaleFactor() == 1);
    }

    SECTION("touch-action: pan-x doesn't pinch")
    {
      touch(pal::PointerAction::Pressed, {100, 125}, 1);
      touch(pal::PointerAction::Pressed, {200, 125}, 2);
      touch(pal::PointerAction::Moved, {300, 125}, 2);
      CHECK(view.GetPage().PageScaleFactor() == 1);
      CHECK(records == List<std::string> {"pointerdown:panx/touch3", "pointerdown:panx/touch4"});
    }
  }

  TEST_CASE("View: a click focuses and the window's focus", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }</style>"
                  u8"<div id=a tabindex=0></div><div id=b></div>");
    WaitForLoad(view);
    view.UpdateRendering();
    auto a = view.Document().GetElementById(u8"a");
    REQUIRE(a);

    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseReleased({10, 10}, pal::MouseButton::Left);
    CHECK(view.Document().FocusedElement() == a.get());
    CHECK(view.Document().HasFocus());

    // The window losing focus keeps the focused element, but the page doesn't have focus.
    view.SetFocused(false);
    CHECK(view.Document().FocusedElement() == a.get());
    CHECK_FALSE(view.Document().HasFocus());
    // A page loaded meanwhile doesn't have focus either.
    view.LoadHTML(u8"<!DOCTYPE html><p>text</p>");
    WaitForLoad(view);
    CHECK_FALSE(view.Document().HasFocus());
    view.SetFocused(true);
    CHECK(view.Document().HasFocus());

    // A press on nothing focusable focuses the viewport.
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }</style>"
                  u8"<div id=a tabindex=0></div><div id=b></div>");
    WaitForLoad(view);
    view.UpdateRendering();
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseReleased({10, 10}, pal::MouseButton::Left);
    REQUIRE(view.Document().FocusedElement());
    view.MousePressed({10, 60}, pal::MouseButton::Left);
    view.MouseReleased({10, 60}, pal::MouseButton::Left);
    CHECK_FALSE(view.Document().FocusedElement());
  }

  TEST_CASE("View: the keyboard selects and copies", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif }</style>"
                  u8"<p id=a>The quick brown fox</p>");
    WaitForLoad(view);
    view.UpdateRendering();

    auto &strategies = boo::PlatformStrategies::Singleton();
    auto *systemPasteboard = strategies.GetPasteboardStrategy();
    MemoryPasteboard pasteboard;
    strategies.SetPasteboardStrategy(&pasteboard);

    auto &selection = view.Document().Selection();
    auto *text = view.Document().GetElementById(u8"a")->FirstChild();

    // Shift with the arrows extends a selection; without Shift, outside editable text, the arrows scroll.
    selection.MoveTo(boo::dom::MakeContainerOffsetPosition(ShareRefPtr(text), 4),
                     boo::dom::MakeContainerOffsetPosition(ShareRefPtr(text), 9), boo::Affinity::Downstream);
    CHECK(view.KeyPressed(pal::Key::RightArrow, {pal::KeyModifier::Shift}));
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 10);
    CHECK(view.KeyPressed(pal::Key::RightArrow, {pal::KeyModifier::Shift, pal::KeyModifier::Control}));
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 16);
    CHECK(view.KeyPressed(pal::Key::Home, {pal::KeyModifier::Shift}));
    CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 0);

    // Ctrl+C copies the selected text and its markup.
    selection.MoveTo(boo::dom::MakeContainerOffsetPosition(ShareRefPtr(text), 4),
                     boo::dom::MakeContainerOffsetPosition(ShareRefPtr(text), 15), boo::Affinity::Downstream);
    CHECK(view.KeyPressed(pal::Key::C, {pal::KeyModifier::Control}, {u8"", u8"c"}));
    CHECK(pasteboard.Writes == 1);
    CHECK(pasteboard.Text == u8"quick brown");
    CHECK_FALSE(pasteboard.Markup.empty());

    // Ctrl+A selects everything.
    CHECK(view.KeyPressed(pal::Key::A, {pal::KeyModifier::Control}, {u8"", u8"a"}));
    CHECK(selection.IsRange());
    CHECK(view.Document().GetEditor().SelectedText() == u8"The quick brown fox");

    strategies.SetPasteboardStrategy(systemPasteboard);
  }

  TEST_CASE("View: the keyboard edits editable text", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif }</style>"
                  u8"<div id=e contenteditable>abc</div>");
    WaitForLoad(view);
    view.UpdateRendering();

    auto &strategies = boo::PlatformStrategies::Singleton();
    auto *systemPasteboard = strategies.GetPasteboardStrategy();
    MemoryPasteboard pasteboard;
    strategies.SetPasteboardStrategy(&pasteboard);

    auto &root = *view.Document().GetElementById(u8"e");
    auto markup = [&]
    {
      auto html = boo::SerializeFragment(root, boo::SerializedNodes::SubtreesOfChildren);
      return std::string(reinterpret_cast<const char *>(html.data()), html.size());
    };
    auto caretAt = [&](uint32 offset)
    {
      view.Document().Selection().MoveTo(
        boo::dom::MakeContainerOffsetPosition(ShareRefPtr(root.FirstChild()), offset),
        boo::Affinity::Downstream);
    };

    // Typed characters are inserted, Backspace deletes, and Enter starts a paragraph.
    caretAt(3);
    (void)view.KeyPressed(pal::Key::D, {}, {u8"d", u8"d"});
    CHECK(markup() == "abcd");
    CHECK(view.KeyPressed(pal::Key::Del));
    CHECK(markup() == "abc");
    (void)view.KeyPressed(pal::Key::Enter, {}, {u8"\r", u8"\r"});
    CHECK(markup() == "abc<div><br></div>");

    // Ctrl+Z undoes the paragraph, and Ctrl+Shift+Z redoes it.
    CHECK(view.KeyPressed(pal::Key::Z, {pal::KeyModifier::Control}, {u8"", u8"z"}));
    CHECK(markup() == "abc");
    CHECK(view.KeyPressed(pal::Key::Z, {pal::KeyModifier::Control, pal::KeyModifier::Shift}, {u8"", u8"Z"}));
    CHECK(markup() == "abc<div><br></div>");
    CHECK(view.KeyPressed(pal::Key::Z, {pal::KeyModifier::Control}, {u8"", u8"z"}));

    // Ctrl+X cuts, and Ctrl+V pastes.
    view.Document().Selection().MoveTo(
      boo::dom::MakeContainerOffsetPosition(ShareRefPtr(root.FirstChild()), 0),
      boo::dom::MakeContainerOffsetPosition(ShareRefPtr(root.FirstChild()), 1), boo::Affinity::Downstream);
    CHECK(view.KeyPressed(pal::Key::X, {pal::KeyModifier::Control}, {u8"", u8"x"}));
    CHECK(pasteboard.Text == u8"a");
    CHECK(markup() == "bc");
    caretAt(2);
    CHECK(view.KeyPressed(pal::Key::V, {pal::KeyModifier::Control}, {u8"", u8"v"}));
    CHECK(markup() == "bca");

    strategies.SetPasteboardStrategy(systemPasteboard);
  }

  TEST_CASE("View: typing into text fields", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif }"
                  u8"input, textarea { display: block; width: 200px; height: 40px }</style>"
                  u8"<input id=field value=ab><textarea id=area></textarea>");
    WaitForLoad(view);
    view.UpdateRendering();

    auto &field = Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"field"));
    auto &area = Downcast<boo::html::HTMLTextAreaElement>(*view.Document().GetElementById(u8"area"));

    // A click focuses the field and puts the caret at the end of its text; typing goes into its value.
    view.MousePressed({190, 20}, pal::MouseButton::Left);
    view.MouseReleased({190, 20}, pal::MouseButton::Left);
    CHECK(view.Document().FocusedElement() == &field);
    CHECK(field.SelectionStart() == 2);
    (void)view.KeyPressed(pal::Key::C, {}, {u8"c", u8"c"});
    CHECK(field.Value() == u8"abc");
    CHECK(view.KeyPressed(pal::Key::Del));
    CHECK(field.Value() == u8"ab");

    // Enter in a field doesn't add a line; in a text area it does.
    (void)view.KeyPressed(pal::Key::Enter, {}, {u8"\r", u8"\r"});
    CHECK(field.Value() == u8"ab");
    view.MousePressed({100, 60}, pal::MouseButton::Left);
    view.MouseReleased({100, 60}, pal::MouseButton::Left);
    CHECK(view.Document().FocusedElement() == &area);
    (void)view.KeyPressed(pal::Key::X, {}, {u8"x", u8"x"});
    (void)view.KeyPressed(pal::Key::Enter, {}, {u8"\r", u8"\r"});
    (void)view.KeyPressed(pal::Key::Y, {}, {u8"y", u8"y"});
    CHECK(area.Value() == u8"x\ny");

    // Ctrl+A selects only the focused control's text.
    CHECK(view.KeyPressed(pal::Key::A, {pal::KeyModifier::Control}, {u8"", u8"a"}));
    CHECK(area.SelectionStart() == 0);
    CHECK(area.SelectionEnd() == 3);
    CHECK(view.Document().GetEditor().SelectedText() == u8"x\ny");
  }

  TEST_CASE("View: keys", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><p>text</p>");
    WaitForLoad(view);
    view.UpdateRendering();
    List<std::string> records;
    Ref<boo::dom::EventListenerCallback> recorder = CreateRef<KeyRecorder>(records);
    for (auto type : {u8"keydown", u8"keypress", u8"keyup"})
    {
      view.Document().AddEventListener(boo::dom::DOMStringAtom(type), Ref {recorder}, false);
    }

    // A key that types text: its value is the text, and its release reports the same value.
    CHECK_FALSE(view.KeyPressed(pal::Key::A, {pal::KeyModifier::Shift}, {u8"A", u8"A"}));
    CHECK_FALSE(view.KeyReleased(pal::Key::A));
    CHECK(records
          == List<std::string> {"keydown:A/KeyA/65/0/0+shift", "keypress:A/KeyA/65/65/0+shift",
                                "keyup:A/KeyA/65/0/0"});
    records.clear();

    // A shortcut types nothing, but its value is the key's text without Ctrl.
    (void)view.KeyPressed(pal::Key::A, {pal::KeyModifier::Control}, {u8"", u8"a"});
    // Named keys have their names, even when they type text; modifiers and the keypad have locations.
    (void)view.KeyPressed(pal::Key::Enter, {}, {u8"\r", u8"\r"});
    (void)view.KeyPressed(pal::Key::RightShift, {pal::KeyModifier::Shift});
    (void)view.KeyPressed(pal::Key::Keypad7, {}, {u8"7", u8"7"});
    CHECK(records
          == List<std::string> {"keydown:a/KeyA/65/0/0+ctrl", "keydown:Enter/Enter/13/0/0",
                                "keypress:Enter/Enter/13/13/0", "keydown:Shift/ShiftRight/16/0/2+shift",
                                "keydown:7/Numpad7/103/0/3", "keypress:7/Numpad7/55/55/3"});
    records.clear();

    // The page can cancel a key.
    struct Canceller : boo::dom::EventListenerCallback
    {
      void HandleEvent(boo::dom::Event &event) noexcept override
      {
        event.PreventDefault();
      }
    };
    view.Document().AddEventListener(boo::dom::DOMStringAtom(u8"keydown"), CreateRef<Canceller>(), false);
    CHECK(view.KeyPressed(pal::Key::B, {}, {u8"b", u8"b"}));
    CHECK(records == List<std::string> {"keydown:b/KeyB/66/0/0"});
  }

  TEST_CASE("View: input conversions", "[UI]")
  {
    auto tab = input::Describe(pal::Key::Tab);
    CHECK(tab.Code == u8"Tab");
    CHECK(tab.NamedKey == u8"Tab");
    CHECK(tab.KeyIdentifier == u8"U+0009");
    CHECK(tab.WindowsVirtualKeyCode == boo::vk::Tab);
    CHECK(input::Describe(pal::Key::Num4).Code == u8"Digit4");
    CHECK(input::Describe(pal::Key::Num4).WindowsVirtualKeyCode == '4');
    CHECK(input::Describe(pal::Key::Z).Code == u8"KeyZ");
    CHECK(input::Describe(pal::Key::F12).NamedKey == u8"F12");
    CHECK(input::Describe(pal::Key::F12).WindowsVirtualKeyCode == boo::vk::F1 + 11);
    CHECK(input::Describe(pal::Key::Del).NamedKey == u8"Backspace");
    CHECK(input::Describe(pal::Key::Option).WindowsVirtualKeyCode == boo::vk::LMenu);
    CHECK(input::Describe(pal::Key::KeypadEnter).IsKeypad);

    // A key that types text has a key identifier from it.
    auto a = input::KeyboardEvent(boo::PlatformEvent::Type::KeyDown, pal::Key::A, {}, {u8"a", u8"a"}, false);
    CHECK(a.KeyIdentifier() == u8"U+0041");
    CHECK(a.Key() == u8"a");

    CHECK(input::ToBooey(pal::MouseButton::Thumb1) == boo::MouseButton::Back);
    auto modifiers = input::ToBooey(pal::KeyModifiers {pal::KeyModifier::Alt, pal::KeyModifier::Meta});
    CHECK(modifiers.Contains(boo::PlatformEvent::Modifier::AltKey));
    CHECK(modifiers.Contains(boo::PlatformEvent::Modifier::MetaKey));
    CHECK_FALSE(modifiers.Contains(boo::PlatformEvent::Modifier::ShiftKey));
    CHECK(input::ToCursorShape(boo::Cursor(boo::Cursor::Type::Grab)) == pal::CursorShape::Hand);
  }

  TEST_CASE("View: Tab moves the focus", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><button id=a></button><button id=b></button>");
    WaitForLoad(view);
    view.UpdateRendering();
    auto a = view.Document().GetElementById(u8"a");
    auto b = view.Document().GetElementById(u8"b");

    CHECK(view.KeyPressed(pal::Key::Tab));
    CHECK(view.Document().FocusedElement() == a.get());
    CHECK(a->HasFocusVisible());
    CHECK(view.KeyPressed(pal::Key::Tab));
    CHECK(view.Document().FocusedElement() == b.get());
    CHECK(view.KeyPressed(pal::Key::Tab, {pal::KeyModifier::Shift}));
    CHECK(view.Document().FocusedElement() == a.get());
    CHECK_FALSE(view.KeyReleased(pal::Key::Tab));
  }

  TEST_CASE("View: scrolling", "[UI]")
  {
    View view({200, 100});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 }</style><div style='height: 500px'></div>");
    WaitForLoad(view);
    view.UpdateRendering();
    struct ScrollCounter : boo::dom::EventListenerCallback
    {
      int32 Count {0};

      void HandleEvent(boo::dom::Event &) noexcept override
      {
        ++Count;
      }
    };
    Ref<ScrollCounter> counter = CreateRef<ScrollCounter>();
    view.Document().AddEventListener(boo::dom::DOMStringAtom(u8"scroll"),
                                     Ref<boo::dom::EventListenerCallback> {counter}, false);

    CHECK(view.ScrollPosition() == boo::IntPoint());
    view.ScrollTo({0, 150});
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 150));
    // Clamped to the document, and `scroll` fires at the next rendering update.
    view.ScrollTo({0, 1'000});
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 400));
    CHECK(counter->Count == 0);
    view.UpdateRendering();
    CHECK(counter->Count == 1);

    // The recording is scrolled.
    CHECK(view.Record()->AsText({}).contains(u8R"((translate
  (x 0.00)
  (y -400.00)))"));
  }

  TEST_CASE("View: the wheel scrolls", "[UI]")
  {
    View view({200, 100});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0 }</style><div style='width: 400px; height: 500px'></div>");
    WaitForLoad(view);
    view.UpdateRendering();
    pal::WheelScrollSettings threeLines {.Lines = 3u, .Chars = 3};

    // A notch towards the user is three lines down, at 100 pixels per three lines.
    CHECK(view.MouseWheel({50, 50}, 0, -1, {}, threeLines));
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 100));
    // Fractions of a notch, from touchpads.
    CHECK(view.MouseWheel({50, 50}, 0, 0.5f, {}, threeLines));
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 50));

    // The horizontal wheel, and the vertical one with Shift, scroll sideways.
    CHECK(view.MouseWheel({50, 50}, 0.5f, 0, {}, threeLines));
    CHECK(view.ScrollPosition() == boo::IntPoint(50, 50));
    CHECK(view.MouseWheel({50, 50}, 0, -0.5f, pal::KeyModifiers {pal::KeyModifier::Shift}, threeLines));
    CHECK(view.ScrollPosition() == boo::IntPoint(100, 50));

    // A page per notch: four fifths of the 79 pixels in view, less the horizontal scrollbar.
    CHECK(view.MouseWheel({50, 50}, 0, -1, {}, {.Lines = null, .Chars = 3}));
    CHECK(view.ScrollPosition() == boo::IntPoint(100, 113));

    // No turn, no event.
    CHECK_FALSE(view.MouseWheel({50, 50}, 0, 0, {}, threeLines));
  }

  TEST_CASE("View: Update runs the event loop with the host's time", "[UI]")
  {
    using namespace std::chrono_literals;
    View view({200, 100});
    view.LoadHTML(u8"<!DOCTYPE html><div id=box style='width: 10px; height: 10px'></div>");
    WaitForLoad(view);
    auto start = boo::html::EventLoop::Current().Now() + 1s;
    view.Update(start);

    // Style and layout run on their timers.
    auto box = view.Document().GetElementById(u8"box");
    REQUIRE_FALSE(box->SetAttribute(u8"style", u8"width: 20px; height: 10px").HasException());
    CHECK(view.Document().HasPendingStyleRecalc());
    view.Update(start);
    CHECK_FALSE(view.Document().HasPendingStyleRecalc());

    // Timers follow the time the host passes.
    int32 fired = 0;
    boo::Timer timer([&] { ++fired; });
    timer.StartOneShot(100ms);
    view.Update(start + 99ms);
    CHECK(fired == 0);
    view.Update(start + 100ms);
    CHECK(fired == 1);

    // Input is followed by a microtask checkpoint.
    struct MicrotaskQueuer : boo::dom::EventListenerCallback
    {
      int32 &Ran;

      explicit MicrotaskQueuer(int32 &ran) noexcept : Ran(ran)
      {
      }

      void HandleEvent(boo::dom::Event &) noexcept override
      {
        boo::html::EventLoop::Current().QueueMicrotask([this] { ++Ran; });
      }
    };
    int32 ran = 0;
    view.Document().AddEventListener(boo::dom::DOMStringAtom(u8"mousedown"),
                                     Ref<boo::dom::EventListenerCallback> {CreateRef<MicrotaskQueuer>(ran)},
                                     false);
    view.MousePressed({5, 5}, pal::MouseButton::Left);
    CHECK(ran == 1);
    view.MouseReleased({5, 5}, pal::MouseButton::Left);
  }

  TEST_CASE("View: Update runs the rendering update when the page changes", "[UI]")
  {
    using namespace std::chrono_literals;
    View view({200, 100});
    view.LoadHTML(
      u8"<!DOCTYPE html><div id=box autofocus tabindex=0 style='width: 10px; height: 10px'></div>");
    WaitForLoad(view);
    auto now = boo::html::EventLoop::Current().Now() + 1s;

    // The new document needs one, which focuses the autofocus candidate.
    CHECK(view.Update(now));
    auto box = view.Document().GetElementById(u8"box");
    CHECK(view.Document().FocusedElement() == box.get());

    // Nothing changed, so nothing to update.
    CHECK_FALSE(view.Update(now));

    REQUIRE_FALSE(box->SetAttribute(u8"style", u8"width: 20px; height: 10px").HasException());
    CHECK(view.Update(now));
    CHECK_FALSE(view.Update(now));
  }

  TEST_CASE("View: loads file images relative to the page's file, in CSS and <img>", "[UI]")
  {
    const auto directory = std::filesystem::temp_directory_path() / "krystal-ui-image-test";
    std::filesystem::create_directories(directory / "images");
    {
      // A 2x1 24-bit BMP: red, then green.
      const unsigned char bmp[] = {
        // File header: the signature, the file's size and the offset of the pixels.
        'B', 'M', 62, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0,
        // Info header: its size, 2x1 pixels, one plane, 24 bits a pixel, no compression, 8 bytes of pixels.
        40, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        // The row, blue-green-red, padded to 4 bytes.
        0, 0, 255, 0, 255, 0, 0, 0};
      std::ofstream image(directory / "images" / "pair.bmp", std::ios::binary);
      image.write(reinterpret_cast<const char *>(bmp), sizeof(bmp));
      std::ofstream page(directory / "page.html", std::ios::binary);
      page << "<!DOCTYPE html><style>body { margin: 0 }</style>"
              "<div id=box style='width: 40px; height: 20px; background: url(images/pair.bmp)'></div>"
              "<img id=img src=images/pair.bmp>";
    }

    View view({200, 100});
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    WaitForCommit(view);
    CHECK(view.Document().GetURL().ProtocolIsFile());
    view.UpdateRendering();
    CHECK(view.Record()->AsText({}).find(u8"native-image") == utf8_string::npos);

    // The load completes as a networking task, which asks for a rendering update.
    auto now = boo::html::EventLoop::Current().Now();
    CHECK(view.Update(now));
    CHECK(view.Record()->AsText({}).find(u8"draw-pattern-native-image") != utf8_string::npos);
    // The <img> loads the same file, from the memory cache.
    CHECK(view.Record()->AsText({}).find(u8"(draw-native-image") != utf8_string::npos);
    auto &image = Downcast<boo::html::HTMLImageElement>(*view.Document().GetElementById(u8"img"));
    CHECK(image.Complete());
    CHECK(image.NaturalWidth() == 2u);

    std::filesystem::remove_all(directory);
  }

  TEST_CASE("View: loads a file page's linked and imported sheets, and reloads them", "[UI]")
  {
    const auto directory = std::filesystem::temp_directory_path() / "krystal-ui-sheet-test";
    std::filesystem::create_directories(directory / "css");
    auto write = [&](const std::filesystem::path &path, std::string_view text)
    {
      std::ofstream file(directory / path, std::ios::binary);
      file << text;
    };
    write("page.html", "<!DOCTYPE html><link rel=stylesheet href=css/page.css><div id=box></div>");
    write("css/page.css", "@import url(sizes.css); #box { height: 10px }");
    write("css/sizes.css", "#box { width: 30px }");

    View view({200, 100});
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    WaitForLoad(view);
    (void)view.Update(boo::html::EventLoop::Current().Now());
    CHECK(WidthOf(view, u8"box") == 30);

    // Loading the page again uses the cached sheets; revalidating loads the edited one.
    write("css/sizes.css", "#box { width: 50px }");
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    WaitForLoad(view);
    (void)view.Update(boo::html::EventLoop::Current().Now());
    CHECK(WidthOf(view, u8"box") == 30);
    REQUIRE(view.LoadFile(directory / "page.html", boo::CachePolicy::Revalidate).has_value());
    WaitForLoad(view);
    (void)view.Update(boo::html::EventLoop::Current().Now());
    CHECK(WidthOf(view, u8"box") == 50);

    std::filesystem::remove_all(directory);
  }

  TEST_CASE("View: hands form submissions to the host", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0 } button { display: block; width: 100px; height: 40px }"
      u8"</style><form action=\"https://example.com/order\" method=post><input name=size value=large>"
      u8"<input name=empty required><button id=go name=go value=1>Order</button></form>");
    WaitForLoad(view);
    view.UpdateRendering();

    List<NavigationRequest> requests;
    view.SetNavigationHandler(
      [&](const NavigationRequest &request)
      {
        requests.push_back(request);
        return NavigationPolicy::Ignore;
      });

    // The required field is empty, so clicking the button doesn't submit.
    auto go = view.Document().GetElementById(u8"go");
    REQUIRE(go);
    REQUIRE(view.HitTest({10, 30}) == go.get());
    view.MousePressed({10, 30}, pal::MouseButton::Left);
    view.MouseReleased({10, 30}, pal::MouseButton::Left);
    CHECK(requests.empty());

    auto empty = view.Document().QuerySelector(u8"[name=empty]");
    REQUIRE_FALSE(empty.HasException());
    REQUIRE(empty.Value());
    empty.Value()->RemoveAttribute(u8"required");
    view.MousePressed({10, 30}, pal::MouseButton::Left);
    view.MouseReleased({10, 30}, pal::MouseButton::Left);
    // The submission is scheduled: it reaches the host on a later task.
    CHECK(requests.empty());
    (void)view.Update(boo::html::EventLoop::Current().Now());
    REQUIRE(requests.size() == 1);
    CHECK(requests[0].IsFormSubmission);
    CHECK_FALSE(requests[0].IsSameDocument);
    CHECK(requests[0].SourceElement.get() == view.Document().QuerySelector(u8"form").Value().get());
    CHECK(requests[0].Method == u8"POST");
    CHECK(requests[0].URL == u8"https://example.com/order");
    CHECK(requests[0].ContentType == u8"application/x-www-form-urlencoded");
    CHECK(requests[0].Body == u8"size=large&empty=&go=1");
  }

  TEST_CASE("View: asks the host about links", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } a { display: block; height: 40px }</style>"
                  u8"<a id=away href=\"https://example.com/away\">Away</a><a id=here href=\"#here\">Here</a>",
                  boo::url::URL(u8"https://example.com/page.html"));
    WaitForLoad(view);
    view.UpdateRendering();

    List<NavigationRequest> requests;
    auto policy = NavigationPolicy::Ignore;
    view.SetNavigationHandler(
      [&](const NavigationRequest &request)
      {
        requests.push_back(request);
        return policy;
      });

    // A link to another page: the host decides, and the page stays.
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseReleased({10, 10}, pal::MouseButton::Left);
    REQUIRE(requests.size() == 1);
    CHECK(requests[0].URL == u8"https://example.com/away");
    CHECK_FALSE(requests[0].IsSameDocument);
    CHECK_FALSE(requests[0].IsFormSubmission);
    CHECK(requests[0].Button == boo::MouseButton::Left);
    CHECK(requests[0].SourceElement.get() == view.Document().GetElementById(u8"away").get());

    // A link within the page, ignored and then allowed.
    view.MousePressed({10, 50}, pal::MouseButton::Left);
    view.MouseReleased({10, 50}, pal::MouseButton::Left);
    REQUIRE(requests.size() == 2);
    CHECK(requests[1].IsSameDocument);
    CHECK(view.Document().GetURL().String() == u8"https://example.com/page.html");

    policy = NavigationPolicy::Allow;
    view.MousePressed({10, 50}, pal::MouseButton::Left);
    view.MouseReleased({10, 50}, pal::MouseButton::Left);
    CHECK(view.Document().GetURL().String() == u8"https://example.com/page.html#here");
  }

  TEST_CASE("View: a page opens at its URL's fragment", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 2000px }</style><div></div>"
                  u8"<p id=far>Far</p><div></div>",
                  boo::url::URL(u8"https://example.com/page.html#far"));
    WaitForLoad(view);
    view.UpdateRendering();

    auto far = view.Document().GetElementById(u8"far");
    REQUIRE(far);
    CHECK(view.Document().CSSTarget() == far.get());
    CHECK(view.Frame().View()->GetScrollPosition().Y() > 0);
    CHECK(far->GetBoundingClientRect()->Y() == 0);
  }

  TEST_CASE("View: a drop-down's popup", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif } select { margin: 10px }"
                  u8"</style><select id=s><option>Apple</option><option disabled>Banana</option>"
                  u8"<optgroup label=Fruit><option>Cherry</option><option>Date</option></optgroup></select>"
                  u8"<p id=after>After</p>");
    WaitForLoad(view);
    view.UpdateRendering();

    auto &select = Downcast<boo::html::HTMLSelectElement>(*view.Document().GetElementById(u8"s"));
    auto box = select.GetBoundingClientRect();
    boo::IntPoint inside(static_cast<int32>(box->X()) + 5, static_cast<int32>(box->Y()) + 5);
    auto changes = 0;
    struct Counter : public boo::dom::EventListenerCallback
    {
      int32 &Count;
      explicit Counter(int32 &count) noexcept : Count(count)
      {
      }
      void HandleEvent(boo::dom::Event &) noexcept override
      {
        ++Count;
      }
    };
    select.AddEventListener(boo::dom::DOMStringAtom(u8"change"), CreateRef<Counter>(changes), false);

    // Pressing the drop-down opens its popup below it, with the selected item highlighted; its release goes
    // to the page and leaves the popup open.
    view.MousePressed(inside, pal::MouseButton::Left);
    view.MouseReleased(inside, pal::MouseButton::Left);
    auto *popup = static_cast<PopupMenuOverlay *>(view.ShownOverlay());
    REQUIRE(popup);
    CHECK(popup->Rect().Y() == static_cast<int32>(box->Bottom()));
    CHECK(popup->Rect().X() == static_cast<int32>(box->X()));
    CHECK(popup->Rect().Width() >= static_cast<int32>(box->Width()));
    CHECK(popup->HighlightedIndex() == 0);

    // The popup's page lists the items: the disabled one and the optgroup's label can't be chosen.
    auto &items = popup->ContentView().Document();
    auto labels = items.QuerySelectorAll(u8".label");
    REQUIRE_FALSE(labels.HasException());
    CHECK(labels.Value()->Length() == 1uz);
    auto disabled = items.QuerySelectorAll(u8".disabled");
    REQUIRE_FALSE(disabled.HasException());
    CHECK(disabled.Value()->Length() == 1uz);

    // Down skips them; Enter chooses, firing change, and closes the popup.
    CHECK(view.KeyPressed(pal::Key::DownArrow));
    CHECK(popup->HighlightedIndex() == 3);
    CHECK(view.KeyPressed(pal::Key::Enter));
    CHECK_FALSE(view.ShownOverlay());
    CHECK(select.SelectedIndex() == 2);
    CHECK(changes == 1);

    // Escape closes it without choosing.
    view.MousePressed(inside, pal::MouseButton::Left);
    view.MouseReleased(inside, pal::MouseButton::Left);
    REQUIRE(view.ShownOverlay());
    CHECK(view.KeyPressed(pal::Key::Escape));
    CHECK_FALSE(view.ShownOverlay());
    CHECK(select.SelectedIndex() == 2);

    // Clicking an item chooses it.
    view.MousePressed(inside, pal::MouseButton::Left);
    view.MouseReleased(inside, pal::MouseButton::Left);
    popup = static_cast<PopupMenuOverlay *>(view.ShownOverlay());
    REQUIRE(popup);
    auto date = popup->ContentView().Document().QuerySelector(u8"[data-index='4']");
    REQUIRE_FALSE(date.HasException());
    REQUIRE(date.Value());
    auto dateBox = date.Value()->GetBoundingClientRect();
    boo::IntPoint onDate(popup->Rect().X() + static_cast<int32>(dateBox->X()) + 5,
                         popup->Rect().Y() + static_cast<int32>(dateBox->Y()) + 3);
    view.MouseMoved(onDate);
    CHECK(popup->HighlightedIndex() == 4);
    view.MousePressed(onDate, pal::MouseButton::Left);
    view.MouseReleased(onDate, pal::MouseButton::Left);
    CHECK_FALSE(view.ShownOverlay());
    CHECK(select.SelectedIndex() == 3);
    CHECK(changes == 2);

    // Pressing outside closes it, and the press goes no further.
    view.MousePressed(inside, pal::MouseButton::Left);
    view.MouseReleased(inside, pal::MouseButton::Left);
    REQUIRE(view.ShownOverlay());
    view.MousePressed({390, 290}, pal::MouseButton::Left);
    view.MouseReleased({390, 290}, pal::MouseButton::Left);
    CHECK_FALSE(view.ShownOverlay());
    CHECK(view.Document().FocusedElement() == &select);

    // The recording shows the popup over the page.
    view.MousePressed(inside, pal::MouseButton::Left);
    view.MouseReleased(inside, pal::MouseButton::Left);
    REQUIRE(view.ShownOverlay());
    CHECK(view.Record()->Items().size() > 0uz);
  }

  namespace
  {
    /// @brief The middle of @p element, in its view's viewport offset by @p origin.
    KRYS_NODISCARD boo::IntPoint CentreOf(boo::dom::Element &element, boo::IntPoint origin = {}) noexcept
    {
      auto box = element.GetBoundingClientRect();
      return boo::IntPoint(origin.X() + static_cast<int32>(box->X() + box->Width() / 2),
                           origin.Y() + static_cast<int32>(box->Y() + box->Height() / 2));
    }

    /// @brief The days since 1970-01-01 of the date @p date, as the date picker's cells name them.
    KRYS_NODISCARD utf8_string DayValue(utf8_stringview date) noexcept
    {
      auto components = boo::DateComponents::FromParsingDate(date);
      auto days = static_cast<int64>(components->MillisecondsSinceEpoch() / 86'400'000.0);
      auto text = std::to_string(days);
      return utf8_string(text.begin(), text.end());
    }

    struct EventCounter : public boo::dom::EventListenerCallback
    {
      int32 Count {0};

      void HandleEvent(boo::dom::Event &) noexcept override
      {
        ++Count;
      }
    };
  }

  TEST_CASE("View: a date input's picker", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif } input { margin: 10px }"
                  u8"</style><input type=date id=d value=2026-03-07 min=2026-03-02><p id=after>After</p>");
    WaitForLoad(view);
    view.UpdateRendering();

    auto &input = Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"d"));
    auto box = input.GetBoundingClientRect();
    boo::IntPoint inside(static_cast<int32>(box->X()) + 5, static_cast<int32>(box->Y()) + 5);
    auto changes = CreateRef<EventCounter>();
    input.AddEventListener(boo::dom::DOMStringAtom(u8"change"),
                           Ref<boo::dom::EventListenerCallback> {changes}, false);
    auto open = [&]
    {
      view.MousePressed(inside, pal::MouseButton::Left);
      view.MouseReleased(inside, pal::MouseButton::Left);
      return static_cast<DateTimeChooserOverlay *>(view.ShownOverlay());
    };

    // A click opens the picker below the input, on the value's month with its day highlighted.
    auto *picker = open();
    REQUIRE(picker);
    CHECK(picker->GetMode() == DateTimeChooserOverlay::Mode::Days);
    CHECK(picker->Rect().Y() == static_cast<int32>(box->Bottom()));
    CHECK(picker->HighlightedValue() == u8"2026-03-07");
    CHECK(input.IsPresentingAttachedView());

    // The arrow keys move the highlight, Page Down turns the page; Enter chooses, firing change, and closes
    // it.
    CHECK(view.KeyPressed(pal::Key::RightArrow));
    CHECK(view.KeyPressed(pal::Key::DownArrow));
    CHECK(picker->HighlightedValue() == u8"2026-03-15");
    CHECK(view.KeyPressed(pal::Key::PageDown));
    CHECK(picker->HighlightedValue() == u8"2026-04-15");
    CHECK(view.KeyPressed(pal::Key::Enter));
    CHECK_FALSE(view.ShownOverlay());
    CHECK_FALSE(input.IsPresentingAttachedView());
    CHECK(input.Value() == u8"2026-04-15");
    CHECK(changes->Count == 1);

    // Clicking a day chooses it; days before the minimum can't be chosen.
    (void)input.SetValue(u8"2026-03-07");
    picker = open();
    REQUIRE(picker);
    auto &days = picker->ContentView().Document();
    auto early = days.QuerySelector(u8"[data-value='" + DayValue(u8"2026-03-01") + u8"']");
    REQUIRE_FALSE(early.HasException());
    REQUIRE(early.Value());
    CHECK(early.Value()->GetAttribute(u8"class").value_or(utf8_string()).contains(u8"disabled"));
    auto onEarly = CentreOf(*early.Value(), picker->Rect().Location());
    view.MousePressed(onEarly, pal::MouseButton::Left);
    view.MouseReleased(onEarly, pal::MouseButton::Left);
    REQUIRE(view.ShownOverlay() == picker);
    // Hovering a day of the next month highlights it without turning the page.
    auto next = days.QuerySelector(u8"[data-value='" + DayValue(u8"2026-04-02") + u8"']");
    REQUIRE_FALSE(next.HasException());
    REQUIRE(next.Value());
    view.MouseMoved(CentreOf(*next.Value(), picker->Rect().Location()));
    CHECK(picker->HighlightedValue() == u8"2026-04-02");
    CHECK(days.QuerySelector(u8"[data-value='" + DayValue(u8"2026-03-01") + u8"']").Value());
    auto day = days.QuerySelector(u8"[data-value='" + DayValue(u8"2026-03-20") + u8"']");
    REQUIRE_FALSE(day.HasException());
    REQUIRE(day.Value());
    auto onDay = CentreOf(*day.Value(), picker->Rect().Location());
    view.MouseMoved(onDay);
    CHECK(picker->HighlightedValue() == u8"2026-03-20");
    view.MousePressed(onDay, pal::MouseButton::Left);
    view.MouseReleased(onDay, pal::MouseButton::Left);
    CHECK_FALSE(view.ShownOverlay());
    CHECK(input.Value() == u8"2026-03-20");

    // The previous month's button turns the page; Escape closes the picker without choosing.
    picker = open();
    REQUIRE(picker);
    auto previous = picker->ContentView().Document().QuerySelector(u8"[data-action='-1']");
    REQUIRE_FALSE(previous.HasException());
    REQUIRE(previous.Value());
    auto onPrevious = CentreOf(*previous.Value(), picker->Rect().Location());
    view.MousePressed(onPrevious, pal::MouseButton::Left);
    view.MouseReleased(onPrevious, pal::MouseButton::Left);
    CHECK(picker->HighlightedValue() == u8"2026-02-20");
    CHECK(view.KeyPressed(pal::Key::Escape));
    CHECK_FALSE(view.ShownOverlay());
    CHECK(input.Value() == u8"2026-03-20");

    // Pressing outside closes it; so does the window losing focus.
    REQUIRE(open());
    view.MousePressed({390, 290}, pal::MouseButton::Left);
    view.MouseReleased({390, 290}, pal::MouseButton::Left);
    CHECK_FALSE(view.ShownOverlay());
    REQUIRE(open());
    view.SetFocused(false);
    CHECK_FALSE(view.ShownOverlay());
    view.SetFocused(true);

    // Month inputs choose from a year's months.
    view.LoadHTML(u8"<!DOCTYPE html><input type=month id=m value=2026-09>");
    WaitForLoad(view);
    view.UpdateRendering();
    auto &month = Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"m"));
    month.Click();
    auto *months = static_cast<DateTimeChooserOverlay *>(view.ShownOverlay());
    REQUIRE(months);
    CHECK(months->GetMode() == DateTimeChooserOverlay::Mode::Months);
    CHECK(months->HighlightedValue() == u8"2026-09");
    CHECK(view.KeyPressed(pal::Key::DownArrow));
    CHECK(view.KeyPressed(pal::Key::Enter));
    CHECK(month.Value() == u8"2026-12");

    // Week inputs choose a day's week.
    view.LoadHTML(u8"<!DOCTYPE html><input type=week id=w value=2026-W10>");
    WaitForLoad(view);
    view.UpdateRendering();
    auto &week = Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"w"));
    week.Click();
    auto *weeks = static_cast<DateTimeChooserOverlay *>(view.ShownOverlay());
    REQUIRE(weeks);
    CHECK(weeks->GetMode() == DateTimeChooserOverlay::Mode::Weeks);
    CHECK(weeks->HighlightedValue() == u8"2026-W10");
    CHECK(view.KeyPressed(pal::Key::DownArrow));
    CHECK(view.KeyPressed(pal::Key::Space));
    CHECK(week.Value() == u8"2026-W11");
  }

  TEST_CASE("View: a colour input's picker", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } input { margin: 10px }</style>"
                  u8"<input type=color id=c value=#336699>");
    WaitForLoad(view);
    view.UpdateRendering();

    auto &input = Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"c"));
    auto box = input.GetBoundingClientRect();
    boo::IntPoint inside(static_cast<int32>(box->X()) + 5, static_cast<int32>(box->Y()) + 5);
    auto changes = CreateRef<EventCounter>();
    input.AddEventListener(boo::dom::DOMStringAtom(u8"change"),
                           Ref<boo::dom::EventListenerCallback> {changes}, false);
    auto open = [&]
    {
      view.MousePressed(inside, pal::MouseButton::Left);
      view.MouseReleased(inside, pal::MouseButton::Left);
      return static_cast<ColorChooserOverlay *>(view.ShownOverlay());
    };

    // A click opens the palette below the input; the value isn't in it, so the first colour is highlighted.
    auto *picker = open();
    REQUIRE(picker);
    CHECK(picker->Colors().size() == 40uz);
    CHECK(picker->Rect().Y() == static_cast<int32>(box->Bottom()));
    CHECK(picker->HighlightedIndex() == 0);

    // The arrow keys move the highlight; Enter chooses, firing change, and closes it.
    CHECK(view.KeyPressed(pal::Key::DownArrow));
    CHECK(view.KeyPressed(pal::Key::RightArrow));
    CHECK(picker->HighlightedIndex() == 9);
    CHECK(view.KeyPressed(pal::Key::Enter));
    CHECK_FALSE(view.ShownOverlay());
    CHECK(input.Value() == u8"#804000");
    CHECK(changes->Count == 1);

    // Opening it again highlights the value; clicking a colour chooses it.
    picker = open();
    REQUIRE(picker);
    CHECK(picker->HighlightedIndex() == 9);
    auto red = picker->ContentView().Document().QuerySelector(u8"[data-index='16']");
    REQUIRE_FALSE(red.HasException());
    REQUIRE(red.Value());
    auto onRed = CentreOf(*red.Value(), picker->Rect().Location());
    view.MouseMoved(onRed);
    CHECK(picker->HighlightedIndex() == 16);
    view.MousePressed(onRed, pal::MouseButton::Left);
    view.MouseReleased(onRed, pal::MouseButton::Left);
    CHECK_FALSE(view.ShownOverlay());
    CHECK(input.Value() == u8"#ff0000");
    CHECK(changes->Count == 2);

    // Escape closes it without choosing.
    REQUIRE(open());
    CHECK(view.KeyPressed(pal::Key::Escape));
    CHECK_FALSE(view.ShownOverlay());
    CHECK(input.Value() == u8"#ff0000");
  }

  TEST_CASE("FileLoaderStrategy: MIME types by extension", "[UI]")
  {
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".PNG") == u8"image/png");
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".jpeg") == u8"image/jpeg");
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".webp") == u8"image/webp");
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".woff2") == u8"font/woff2");
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".TTF") == u8"font/ttf");
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".unknown").empty());
  }

  TEST_CASE("View: animations run on the host's frames", "[UI]")
  {
    using namespace std::chrono_literals;
    // As a host's page: its style sheet loads from a file, as a networking task.
    const auto directory = std::filesystem::temp_directory_path() / "krystal-ui-animation-test";
    std::filesystem::create_directories(directory);
    {
      std::ofstream page(directory / "page.html", std::ios::binary);
      page << "<!DOCTYPE html><link rel=stylesheet href=page.css><div id=box></div>";
      std::ofstream sheet(directory / "page.css", std::ios::binary);
      sheet << "@keyframes fade { from { opacity: 0 } to { opacity: 1 } }"
               "#box { width: 10px; height: 10px; animation: fade 1s linear infinite }";
    }
    View view({200, 100});
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    WaitForLoad(view);
    // The animation runs on the box's layer, so its value is the renderer's animated style.
    auto opacity = [&]
    {
      auto box = view.Document().GetElementById(u8"box");
      REQUIRE(box);
      REQUIRE(box->Renderer());
      return box->Renderer()->AnimatedStyle()->GetOpacity().Value.Value;
    };

    // Frames at 60 Hz, longer than the timelines controller keeps its cached time: the animation still gets
    // its start time and runs.
    constexpr auto frame =
      std::chrono::duration_cast<MonotonicTime::duration>(std::chrono::microseconds {16'667});
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    for (int32 i = 0; i < 4; ++i)
    {
      time += frame;
      (void)view.Update(time);
    }
    auto &animation = *view.Document().GetElementById(u8"box")->Animations(null)->first();
    CHECK_FALSE(animation.Pending());
    auto before = opacity();
    for (int32 i = 0; i < 15; ++i)
    {
      time += frame;
      // The page's style doesn't change while the animation runs on its layer.
      CHECK_FALSE(view.Update(time));
    }
    CHECK(opacity() == Catch::Approx(before + 0.25f).margin(0.02f));
    std::filesystem::remove_all(directory);
  }

  TEST_CASE("View: the page's window asks the host to focus and close it", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html>");
    WaitForLoad(view);

    uint32 focusRequests = 0;
    uint32 closeRequests = 0;
    auto &window = *view.Document().Window();

    // Without a handler, nothing happens.
    window.Focus();

    view.SetFocusRequestHandler([&] { ++focusRequests; });
    view.SetCloseRequestHandler([&] { ++closeRequests; });

    view.Document().Window()->Focus();
    CHECK(focusRequests == 1);
    view.Document().Window()->Close();
    CHECK(closeRequests == 0);
    boo::html::EventLoop::Current().RunUntilIdle();
    CHECK(closeRequests == 1);
  }

  TEST_CASE("View: the page's window geometry is the host's", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html>");
    WaitForLoad(view);
    auto &window = *view.Document().Window();

    // Until the host says, the window is the viewport at the screen's origin.
    CHECK(window.OuterWidth() == 400);
    CHECK(window.OuterHeight() == 300);
    CHECK(window.ScreenX() == 0);

    view.SetWindowRect(boo::FloatRect(10, 20, 800, 600));
    CHECK(window.ScreenX() == 10);
    CHECK(window.ScreenY() == 20);
    CHECK(window.OuterWidth() == 800);

    auto environment = view.Frame().GetMediaEnvironment();
    environment.ScreenSize = boo::FloatSize(1'920, 1'080);
    view.Frame().SetMediaEnvironment(environment);

    List<boo::FloatRect> requests;
    view.SetWindowRectRequestHandler([&](const boo::FloatRect &rect) { requests.push_back(rect); });
    window.MoveTo(100, 200);
    REQUIRE(requests.size() == 1);
    CHECK(requests[0] == boo::FloatRect(100, 200, 800, 600));
  }

  TEST_CASE("View: the page's prompts and console messages reach the host's handlers", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html>", boo::url::URL(u8"https://example.com/page.html"));
    WaitForLoad(view);
    auto &window = *view.Document().Window();

    // Without handlers, prompts show nothing and are cancelled.
    window.Alert(u8"Hi");
    CHECK_FALSE(window.Confirm(u8"Sure?"));
    CHECK_FALSE(window.Prompt(u8"Name?"));

    List<std::string> shown;
    view.SetAlertHandler([&](const utf8_string &message)
                         { shown.push_back(std::string(message.begin(), message.end())); });
    view.SetConfirmHandler([](const utf8_string &) { return true; });
    view.SetPromptHandler([](const utf8_string &, const utf8_string &defaultValue)
                          { return Maybe<utf8_string>(defaultValue + u8"!"); });
    List<ConsoleMessage> messages;
    view.SetConsoleMessageHandler([&](const ConsoleMessage &message) { messages.push_back(message); });

    window.Alert(u8"Hi");
    CHECK(shown == List<std::string> {"Hi"});
    CHECK(window.Confirm(u8"Sure?"));
    CHECK(window.Prompt(u8"Name?", u8"Ada") == boo::dom::DOMString(u8"Ada!"));

    window.ReportError(u8"Something broke");
    REQUIRE(messages.size() == 1);
    CHECK(messages[0].Level == boo::MessageLevel::Error);
    CHECK(messages[0].Message == u8"Something broke");
    CHECK(messages[0].SourceURL == u8"https://example.com/page.html");

    view.SetUserAgent(u8"KrystalGame/1.0");
    CHECK(window.GetNavigator().UserAgent() == u8"KrystalGame/1.0");
  }

  TEST_CASE("View: local storage kept in files", "[UI]")
  {
    auto directory = std::filesystem::temp_directory_path() / "KrystalStorageTest";
    std::filesystem::remove_all(directory);

    {
      FileStorageProvider provider(directory);
      auto area = provider.LocalStorageArea(u8"https://example.com");
      Maybe<boo::dom::DOMString> oldValue;
      REQUIRE(area->SetItem(u8"name", u8"Ada \u00e9", oldValue));
      REQUIRE(area->SetItem(u8"level", u8"3", oldValue));
      CHECK(provider.PathForOrigin(u8"https://example.com").filename() == "https___example_com.storage");
      CHECK(std::filesystem::exists(provider.PathForOrigin(u8"https://example.com")));
    }

    // Another provider reads the items back, in order.
    FileStorageProvider provider(directory);
    auto area = provider.LocalStorageArea(u8"https://example.com");
    CHECK(area->Length() == 2);
    CHECK(area->Key(0) == boo::dom::DOMString(u8"name"));
    CHECK(area->Item(u8"name") == boo::dom::DOMString(u8"Ada \u00e9"));
    CHECK(area->Item(u8"level") == boo::dom::DOMString(u8"3"));
    (void)area->RemoveItem(u8"level");

    FileStorageProvider again(directory);
    CHECK(again.LocalStorageArea(u8"https://example.com")->Length() == 1);
    std::filesystem::remove_all(directory);
  }

  TEST_CASE("View: loads pages on a later task, and tells the host", "[UI]")
  {
    View view({400, 300});
    // A view starts with its initial empty document.
    CHECK(view.Document().GetURL().String() == u8"about:blank");
    CHECK_FALSE(view.IsLoading());

    List<std::string> events;
    view.SetLoadHandler(
      [&](const LoadEvent &event)
      {
        switch (event.Type)
        {
          case LoadEventType::Started:   events.push_back("started"); break;
          case LoadEventType::Committed: events.push_back("committed"); break;
          case LoadEventType::Finished:  events.push_back("finished"); break;
          case LoadEventType::Failed:    events.push_back("failed"); break;
        }
      });

    view.LoadHTML(u8"<!DOCTYPE html><a id=next href=next.html>Next</a>",
                  boo::url::URL(u8"https://example.com/page.html"));
    CHECK(view.IsLoading());
    CHECK(view.Document().GetURL().String() == u8"about:blank");
    WaitForLoad(view);
    CHECK(view.Document().GetURL().String() == u8"https://example.com/page.html");
    CHECK(events == List<std::string> {"started", "committed", "finished"});

    SECTION("stopping keeps the page")
    {
      events.clear();
      view.LoadHTML(u8"<!DOCTYPE html><p>Other</p>");
      view.Stop();
      WaitForLoad(view);
      CHECK(view.Document().GetURL().String() == u8"https://example.com/page.html");
      CHECK(events == List<std::string> {"started", "failed"});
    }

    SECTION("a link the handler allows loads its page")
    {
      events.clear();
      List<std::string> asked;
      view.SetNavigationHandler(
        [&](const NavigationRequest &request)
        {
          asked.push_back(std::string(request.URL.begin(), request.URL.end()));
          return NavigationPolicy::Allow;
        });
      view.Document().GetElementById(u8"next")->DispatchSimulatedClick(nullptr);
      CHECK(asked == List<std::string> {"https://example.com/next.html"});
      // The file loader strategy can't load https: pages, so the load fails and the page stays.
      WaitForLoad(view);
      CHECK(events == List<std::string> {"started", "failed"});
      CHECK(view.Document().GetURL().String() == u8"https://example.com/page.html");
    }
  }

  TEST_CASE("View: goes back and forward through its pages", "[UI]")
  {
    View view({400, 300});
    CHECK_FALSE(view.CanGoBack());
    CHECK_FALSE(view.CanGoForward());

    view.LoadHTML(u8"<!DOCTYPE html><p id=first>First</p><input id=field>",
                  boo::url::URL(u8"https://example.com/first.html"));
    WaitForLoad(view);
    auto *field =
      DynamicDowncast<boo::html::HTMLInputElement>(view.Document().GetElementById(u8"field").get());
    REQUIRE(field);
    (void)field->SetValue(u8"typed");
    RefPtr<boo::dom::Document> first = ShareRefPtr<boo::dom::Document>(&view.Document());
    view.LoadHTML(u8"<!DOCTYPE html><p id=second>Second</p>",
                  boo::url::URL(u8"https://example.com/second.html"));
    WaitForLoad(view);

    CHECK(view.CanGoBack());
    CHECK_FALSE(view.CanGoForward());
    REQUIRE(view.GetBackForwardList().Entries().size() == 2);

    auto shows = [&](utf8_stringview id)
    {
      return view.Document().GetElementById(id) != nullptr;
    };

    SECTION("GoBack and GoForward")
    {
      // A page from the back/forward cache commits once it's the view's again.
      List<bool> committedFirst;
      view.SetLoadHandler(
        [&](const LoadEvent &event)
        {
          if (event.Type == LoadEventType::Committed)
          {
            committedFirst.push_back(&view.Document() == first.get() && event.FromBackForwardCache);
          }
        });
      REQUIRE(view.GoBack());
      WaitForLoad(view);
      CHECK(committedFirst == List<bool> {true});
      CHECK(shows(u8"first"));
      // The back/forward cache kept the page, with its form filled in.
      CHECK(&view.Document() == first.get());
      field = DynamicDowncast<boo::html::HTMLInputElement>(view.Document().GetElementById(u8"field").get());
      REQUIRE(field);
      CHECK(field->Value() == u8"typed");
      CHECK(view.CanGoForward());

      REQUIRE(view.GoForward());
      WaitForLoad(view);
      CHECK(shows(u8"second"));
      CHECK_FALSE(view.GoForward());
    }

    SECTION("the mouse's back and forward buttons")
    {
      view.MousePressed({10, 10}, pal::MouseButton::Thumb1);
      view.MouseReleased({10, 10}, pal::MouseButton::Thumb1);
      WaitForLoad(view);
      CHECK(shows(u8"first"));

      view.MousePressed({10, 10}, pal::MouseButton::Thumb2);
      view.MouseReleased({10, 10}, pal::MouseButton::Thumb2);
      WaitForLoad(view);
      CHECK(shows(u8"second"));
    }

    SECTION("Alt+Left and Alt+Right")
    {
      CHECK(view.KeyPressed(pal::Key::LeftArrow, {pal::KeyModifier::Alt}));
      (void)view.KeyReleased(pal::Key::LeftArrow, {pal::KeyModifier::Alt});
      WaitForLoad(view);
      CHECK(shows(u8"first"));

      CHECK(view.KeyPressed(pal::Key::RightArrow, {pal::KeyModifier::Alt}));
      (void)view.KeyReleased(pal::Key::RightArrow, {pal::KeyModifier::Alt});
      WaitForLoad(view);
      CHECK(shows(u8"second"));
    }

    SECTION("a page that handles the key keeps it")
    {
      auto listener =
        view.Document().AddEventListener(u8"keydown", [](boo::dom::Event &event) { event.PreventDefault(); });
      CHECK(view.KeyPressed(pal::Key::LeftArrow, {pal::KeyModifier::Alt}));
      CHECK_FALSE(view.IsLoading());
      CHECK(shows(u8"second"));
    }

    SECTION("going to an item of the list")
    {
      view.GoToItem(*view.GetBackForwardList().Entries()[0]);
      WaitForLoad(view);
      CHECK(shows(u8"first"));
    }

    SECTION("reloading keeps the item")
    {
      RefPtr<boo::dom::Document> before = ShareRefPtr<boo::dom::Document>(&view.Document());
      view.Reload();
      WaitForLoad(view);
      CHECK(&view.Document() != before.get());
      CHECK(shows(u8"second"));
      CHECK(view.GetBackForwardList().Entries().size() == 2);
    }
  }
}
