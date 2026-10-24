#include "Krystal.Booey/HTML/HTMLDetailsElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/ToggleEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLSummaryElement.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::NavigationFixture;

  namespace
  {
    KRYS_NODISCARD std::string Narrow(dom::DOMStringView string)
    {
      return std::string(reinterpret_cast<const char *>(string.data()), string.size());
    }

    /// @brief Records toggle events, as `"<id>:<old>><new>"`.
    struct ToggleLog
    {
      List<std::string> Events;
      List<dom::EventListenerHandle> Handles;

      void Watch(dom::Element &element) noexcept
      {
        auto id = Narrow(element.GetAttribute(u8"id").value_or(dom::DOMString()));
        Handles.push_back(element.AddEventListener(u8"toggle",
                                                   [this, id](dom::Event &event)
                                                   {
                                                     auto &toggle = Downcast<dom::ToggleEvent>(event);
                                                     Events.push_back(id + ":" + Narrow(toggle.OldState())
                                                                      + ">" + Narrow(toggle.NewState()));
                                                   }));
      }

      /// @brief The events so far, after running the queued tasks (toggle is queued).
      KRYS_NODISCARD List<std::string> Take() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
        return std::exchange(Events, {});
      }
    };

    KRYS_NODISCARD HTMLSlotElement &ContentSlot(HTMLDetailsElement &details) noexcept
    {
      for (auto *child = details.UserAgentShadowRoot()->FirstChild(); child; child = child->NextSibling())
      {
        if (auto *slot = DynamicDowncast<HTMLSlotElement>(*child); slot && slot->Name().empty())
        {
          return *slot;
        }
      }
      krys_unreachable();
    }

    void Navigate(NavigationFixture &fixture, utf8_stringview url) noexcept
    {
      fixture.Frame->Loader().ChangeLocation(
        url::URL(fixture.Document->GetURL(), url), dom::DOMStringAtom::Empty(), nullptr,
        ReferrerPolicy::EmptyString, std::nullopt, dom::DOMStringAtom::Null());
      html::EventLoop::Current().RunUntilIdle();
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLDetailsElement: the shadow tree",
                   "[HTML][HTMLDetailsElement]")
  {
    NavigationFixture fixture(
      u8"<style>body { margin: 0 } details::details-content { margin-left: 30px }</style>"
      u8"<details id=d><p id=content>Text</p><summary id=s>Caption</summary>"
      u8"<summary id=second>Not the caption</summary></details>"
      u8"<details id=bare>Only text</details><summary id=loose>Loose</summary>");
    auto &document = *fixture.Document;
    auto &details = fixture.Get<HTMLDetailsElement>(u8"d");
    auto &summary = fixture.Get<HTMLSummaryElement>(u8"s");
    auto &second = fixture.Get<HTMLSummaryElement>(u8"second");
    auto &content = fixture.ById(u8"content");
    (void)document.UpdateLayoutIgnorePendingStylesheets();

    SECTION("The first summary child is the caption; the rest goes in the content slot")
    {
      REQUIRE(details.UserAgentShadowRoot());
      CHECK(summary.IsActiveSummary());
      CHECK_FALSE(second.IsActiveSummary());
      CHECK_FALSE(fixture.Get<HTMLSummaryElement>(u8"loose").IsActiveSummary());
      CHECK(dom::SlotAlgorithms::GetAssignedSlot(summary)->Name() == u8"summarySlot");
      CHECK(dom::SlotAlgorithms::GetAssignedSlot(second) == &ContentSlot(details));
      CHECK(dom::SlotAlgorithms::GetAssignedSlot(content) == &ContentSlot(details));

      // The caption is a list item with a disclosure marker.
      REQUIRE(summary.Renderer());
      CHECK(summary.Renderer()->Style().Display() == DisplayType::ListItem);
      REQUIRE(second.Renderer());
      CHECK(second.Renderer()->Style().Display() == DisplayType::Block);
    }

    SECTION("Inserting a summary before the caption makes it the caption; removing it restores the old one")
    {
      auto first = document.CreateElement(u8"summary").ReleaseValue();
      REQUIRE_FALSE(details.InsertBefore(*first, details.FirstChild()).HasException());
      CHECK(Downcast<HTMLSummaryElement>(*first).IsActiveSummary());
      CHECK_FALSE(summary.IsActiveSummary());
      CHECK(dom::SlotAlgorithms::GetAssignedSlot(summary) == &ContentSlot(details));

      REQUIRE_FALSE(first->Remove().HasException());
      CHECK(summary.IsActiveSummary());
      CHECK(dom::SlotAlgorithms::GetAssignedSlot(summary)->Name() == u8"summarySlot");

      REQUIRE_FALSE(summary.Remove().HasException());
      CHECK(second.IsActiveSummary());
    }

    SECTION("Without a summary child, the default summary is the caption")
    {
      auto &bare = fixture.Get<HTMLDetailsElement>(u8"bare");
      auto *summarySlot = Downcast<HTMLSlotElement>(bare.UserAgentShadowRoot()->FirstChild());
      CHECK_FALSE(summarySlot->AssignedNodes());
      auto &defaultSummary = Downcast<HTMLSummaryElement>(*summarySlot->FirstChild());
      CHECK(defaultSummary.IsActiveSummary());
      CHECK(defaultSummary.TextContent() == u8"Details");
      CHECK(FocusAlgorithms::IsFocusableArea(defaultSummary));
      defaultSummary.Click();
      CHECK(bare.Open());
    }

    SECTION("The content is skipped while closed, and styled by ::details-content")
    {
      REQUIRE(content.Renderer());
      CHECK(content.Renderer()->IsSkippedContent());
      CHECK(ContentSlot(details).GetRenderStyle()->UsedContentVisibility() == ContentVisibility::Hidden);

      details.Open(true);
      (void)document.UpdateLayoutIgnorePendingStylesheets();
      REQUIRE(content.Renderer());
      CHECK_FALSE(content.Renderer()->IsSkippedContent());
      CHECK(content.GetBoundingClientRect()->X() == 30);
    }

    SECTION("The caption is focusable and in the tab order; other summaries aren't")
    {
      CHECK(summary.TabIndex() == 0);
      CHECK(second.TabIndex() == -1);
      CHECK(FocusAlgorithms::IsFocusableArea(summary));
      CHECK_FALSE(FocusAlgorithms::IsFocusableArea(second));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLDetailsElement: toggling", "[HTML][HTMLDetailsElement]")
  {
    NavigationFixture fixture(u8"<details id=d><summary id=s>Caption <span id=span>text</span> "
                              u8"<a id=link href=#x>link</a> <input id=field></summary>"
                              u8"<summary id=second>Second</summary>Content</details>");
    auto &details = fixture.Get<HTMLDetailsElement>(u8"d");
    auto &summary = fixture.Get<HTMLSummaryElement>(u8"s");
    ToggleLog log;
    log.Watch(details);

    SECTION("Setting open fires toggle; changes before it fires coalesce")
    {
      details.Open(true);
      CHECK(details.HasAttribute(u8"open"));
      CHECK(log.Take() == List<std::string> {"d:closed>open"});
      (void)details.SetAttribute(u8"open", u8"again");
      CHECK(log.Take().empty());

      details.Open(false);
      details.Open(true);
      details.Open(false);
      CHECK(log.Take() == List<std::string> {"d:open>closed"});
    }

    SECTION("Activating the caption toggles it, unless the activation is for interactive content in it")
    {
      summary.Click();
      CHECK(details.Open());
      Downcast<HTMLElement>(fixture.ById(u8"span")).Click();
      CHECK_FALSE(details.Open());
      Downcast<HTMLElement>(fixture.ById(u8"field")).Click();
      CHECK_FALSE(details.Open());
      fixture.Get<HTMLSummaryElement>(u8"second").Click();
      CHECK_FALSE(details.Open());
      CHECK(log.Take() == List<std::string> {"d:closed>closed"});
    }

    SECTION("A link in the caption is activated instead")
    {
      Downcast<HTMLElement>(fixture.ById(u8"link")).Click();
      html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(details.Open());
      CHECK(fixture.DocumentURL() == "https://example.com/dir/page.html#x");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLDetailsElement: exclusive accordions",
                   "[HTML][HTMLDetailsElement]")
  {
    NavigationFixture fixture(u8"<details id=a name=faq open>A</details><details id=b name=faq>B</details>"
                              u8"<details id=c name=FAQ open>C</details><details id=d open>D</details>"
                              u8"<details id=e open>E</details>");
    auto &document = *fixture.Document;
    auto &a = fixture.Get<HTMLDetailsElement>(u8"a");
    auto &b = fixture.Get<HTMLDetailsElement>(u8"b");
    auto &c = fixture.Get<HTMLDetailsElement>(u8"c");
    auto &d = fixture.Get<HTMLDetailsElement>(u8"d");
    auto &e = fixture.Get<HTMLDetailsElement>(u8"e");

    SECTION("Opening one closes the others with its name, which is case-sensitive")
    {
      CHECK(a.Open());
      b.Open(true);
      CHECK_FALSE(a.Open());
      CHECK(b.Open());
      CHECK(c.Open());
      // Without a name there's no group.
      CHECK(d.Open());
      CHECK(e.Open());
    }

    SECTION("Joining a group with an open element closes the joining one")
    {
      d.Name(dom::DOMString(u8"faq"));
      CHECK_FALSE(d.Open());
      CHECK(a.Open());
    }

    SECTION("Inserting an open element into a group with an open one closes it")
    {
      auto inserted = document.CreateElement(u8"details").ReleaseValue();
      (void)inserted->SetAttribute(u8"name", u8"faq");
      (void)inserted->SetAttribute(u8"open", u8"");
      CHECK(inserted->HasAttribute(u8"open"));
      REQUIRE_FALSE(document.Body()->AppendChild(*inserted).HasException());
      CHECK_FALSE(inserted->HasAttribute(u8"open"));
      CHECK(a.Open());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLDetailsElement: revealing", "[HTML][HTMLDetailsElement]")
  {
    NavigationFixture fixture(
      u8"<details id=d><summary>Q</summary><p id=answer>The answer is here</p></details>"
      u8"<div id=hidden hidden=until-found><p id=found>Found by name</p></div>"
      u8"<div id=plain hidden><p id=never>Never found</p></div>");
    auto &document = *fixture.Document;
    (void)document.UpdateLayoutIgnorePendingStylesheets();
    auto &details = fixture.Get<HTMLDetailsElement>(u8"d");
    auto &hidden = Downcast<HTMLElement>(fixture.ById(u8"hidden"));
    List<std::string> events;
    auto handle = document.AddEventListener(
      u8"beforematch",
      [&](dom::Event &event)
      {
        CHECK(event.Bubbles());
        CHECK_FALSE(event.Cancelable());
        events.push_back(Narrow(Downcast<dom::Element>(Downcast<dom::Node>(*event.Target()))
                                  .GetAttribute(u8"id")
                                  .value_or(dom::DOMString())));
      });

    SECTION("hidden=until-found reflects as such, whatever its case")
    {
      CHECK(hidden.IsHiddenUntilFound());
      hidden.Hidden(dom::DOMString(u8"UNTIL-FOUND"));
      CHECK(hidden.GetAttribute(u8"hidden") == u8"until-found");
    }

    SECTION("A fragment opens its target's closed details")
    {
      Navigate(fixture, u8"#answer");
      CHECK(details.Open());
      CHECK(events.empty());
    }

    SECTION("A fragment fires beforematch at its target's hidden=until-found ancestor, then shows it")
    {
      Navigate(fixture, u8"#found");
      CHECK(events == List<std::string> {"hidden"});
      CHECK_FALSE(hidden.HasAttribute(u8"hidden"));
    }

    SECTION("Plain hidden content isn't revealed")
    {
      Navigate(fixture, u8"#never");
      CHECK(Downcast<HTMLElement>(fixture.ById(u8"plain")).HasAttribute(u8"hidden"));
      CHECK(events.empty());
    }

    SECTION("A text fragment opens the details its text is in")
    {
      Navigate(fixture, u8"#:~:text=answer%20is%20here");
      CHECK(document.CSSTarget() == &fixture.ById(u8"answer"));
      CHECK(details.Open());
    }
  }
}
