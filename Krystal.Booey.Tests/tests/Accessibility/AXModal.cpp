#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AXModal: an aria-modal dialog", "[Accessibility][AXModal]")
  {
    AccessibilityFixture page(u8"<button id=outside>Outside</button>"
                              u8"<div id=dialog role=dialog aria-modal=true><button id=inside "
                              u8"aria-controls=popup>Inside</button></div>"
                              u8"<div id=popup role=listbox>Popup</div>");
    auto &outside = page.Object(u8"outside");
    auto &inside = page.Object(u8"inside");

    SECTION("Without the focus in it, it isn't modal")
    {
      CHECK(page.Cache().ModalNode() == nullptr);
      CHECK_FALSE(outside.IsIgnored());
    }

    SECTION("With the focus in it, everything outside is ignored")
    {
      Downcast<html::HTMLElement>(page.ById(u8"inside")).Focus();
      (void)page.TakeNotifications();
      CHECK(page.Cache().ModalNode() == &page.ById(u8"dialog"));
      CHECK(outside.IsIgnored());
      CHECK_FALSE(inside.IsIgnored());
      // Something in the dialog controls it.
      CHECK_FALSE(page.Object(u8"popup").IsIgnored());

      SECTION("Removing aria-modal ends it")
      {
        page.ById(u8"dialog").RemoveAttribute(dom::DOMStringAtom(u8"aria-modal"));
        (void)page.TakeNotifications();
        CHECK(page.Cache().ModalNode() == nullptr);
        CHECK_FALSE(outside.IsIgnored());
      }

      SECTION("Hiding it ends it")
      {
        REQUIRE_FALSE(page.ById(u8"dialog").SetAttribute(u8"style", u8"display: none").HasException());
        (void)page.Document->UpdateLayout();
        (void)page.TakeNotifications();
        CHECK(page.Cache().ModalNode() == nullptr);
      }
    }
  }

  TEST_CASE("AXModal: an empty dialog doesn't trap the user", "[Accessibility][AXModal]")
  {
    AccessibilityFixture page(u8"<button id=outside>Outside</button>"
                              u8"<div id=dialog role=dialog aria-modal=true tabindex=0></div>");
    (void)page.Object(u8"outside");
    Downcast<html::HTMLElement>(page.ById(u8"dialog")).Focus();
    (void)page.TakeNotifications();
    CHECK(page.Cache().ModalNode() == nullptr);
    CHECK_FALSE(page.Object(u8"outside").IsIgnored());
  }

  TEST_CASE("AXModal: a modal dialog element", "[Accessibility][AXModal]")
  {
    AccessibilityFixture page(u8"<p id=text>Text</p>"
                              u8"<dialog id=dialog><button id=inside>Inside</button></dialog>");
    auto &text = page.Object(u8"text");
    CHECK(page.Cache().ModalNode() == nullptr);

    auto &dialog = Downcast<html::HTMLDialogElement>(page.ById(u8"dialog"));
    REQUIRE_FALSE(dialog.ShowModal().HasException());
    (void)page.Document->UpdateLayout();
    (void)page.TakeNotifications();
    CHECK(page.Cache().ModalNode() == &dialog);
    CHECK(text.IsIgnored());
    CHECK_FALSE(page.Object(u8"inside").IsIgnored());

    dialog.Close();
    (void)page.Document->UpdateLayout();
    (void)page.TakeNotifications();
    CHECK(page.Cache().ModalNode() == nullptr);
    CHECK_FALSE(text.IsIgnored());
  }
}
