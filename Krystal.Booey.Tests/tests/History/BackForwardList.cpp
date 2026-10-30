#include "Krystal.Booey/History/BackForwardList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<HistoryItem> Item(utf8_stringview url) noexcept
    {
      return HistoryItem::Create(url::URL(url));
    }
  }

  TEST_CASE("HistoryItem", "[History][HistoryItem]")
  {
    auto item = Item(u8"https://example.com/a.html#top");
    CHECK(item->URL().String() == u8"https://example.com/a.html#top");
    CHECK(item->OriginalURL() == item->URL());
    CHECK(item->ShouldRestoreScrollPosition());

    SECTION("each item has its own identifier and sequence numbers")
    {
      auto other = Item(u8"https://example.com/a.html#top");
      CHECK(item->ItemID() != other->ItemID());
      CHECK(item->ItemSequenceNumber() != other->ItemSequenceNumber());
      CHECK(item->DocumentSequenceNumber() != other->DocumentSequenceNumber());
      CHECK_FALSE(*item == *other);
    }

    SECTION("a copy keeps the identifier")
    {
      item->SetScrollPosition(IntPoint(0, 30));
      item->SetSubstituteData(
        SubstituteData(u8"<p>", u8"text/html", url::URL(u8"https://example.com/a.html")));
      auto copy = item->Copy();
      CHECK(*copy == *item);
      CHECK(copy->ScrollPosition() == IntPoint(0, 30));
      CHECK(copy->DocumentSequenceNumber() == item->DocumentSequenceNumber());
      REQUIRE(copy->GetSubstituteData());
      CHECK(copy->GetSubstituteData()->Content() == u8"<p>");
    }

    SECTION("items for the same document navigate within it")
    {
      auto fragment = Item(u8"https://example.com/a.html#more");
      CHECK_FALSE(item->ShouldDoSameDocumentNavigationTo(*fragment));
      fragment->SetDocumentSequenceNumber(item->DocumentSequenceNumber());
      CHECK(item->ShouldDoSameDocumentNavigationTo(*fragment));
      CHECK_FALSE(item->ShouldDoSameDocumentNavigationTo(*item));
    }

    SECTION("setting the URL drops the document state; setting the URL string keeps it")
    {
      item->SetDocumentState({dom::DOMString(u8"state")});
      item->SetURLString(url::URL(u8"https://example.com/b.html"));
      CHECK(item->DocumentState().size() == 1);
      item->SetURL(url::URL(u8"https://example.com/c.html"));
      CHECK(item->DocumentState().empty());
      CHECK(item->URL().String() == u8"https://example.com/c.html");
    }

    SECTION("resetting gives new sequence numbers")
    {
      auto documentSequenceNumber = item->DocumentSequenceNumber();
      auto id = item->ItemID();
      item->Reset();
      CHECK(item->URL().IsEmpty());
      CHECK(item->DocumentSequenceNumber() != documentSequenceNumber);
      CHECK(item->ItemID() == id);
    }
  }

  TEST_CASE("BackForwardList", "[History][BackForwardList]")
  {
    auto list = BackForwardList::Create();
    CHECK_FALSE(list->CurrentItem());
    CHECK_FALSE(list->ItemAtIndex(0));
    CHECK(list->BackListCount() == 0);
    CHECK(list->ForwardListCount() == 0);

    auto a = Item(u8"https://example.com/a.html");
    auto b = Item(u8"https://example.com/b.html");
    auto c = Item(u8"https://example.com/c.html");
    list->AddItem(Ref<HistoryItem>(a));
    list->AddItem(Ref<HistoryItem>(b));
    list->AddItem(Ref<HistoryItem>(c));

    CHECK(list->CurrentItem() == c.get());
    CHECK(list->BackItem() == b.get());
    CHECK_FALSE(list->ForwardItem());
    CHECK(list->BackListCount() == 2);
    CHECK(list->ItemAtIndex(-2) == a.get());
    CHECK_FALSE(list->ItemAtIndex(-3));
    CHECK_FALSE(list->ItemAtIndex(1));
    CHECK(list->ContainsItem(*b));

    SECTION("going back and forward moves the current item")
    {
      list->GoBack();
      list->GoBack();
      CHECK(list->CurrentItem() == a.get());
      CHECK(list->ForwardListCount() == 2);
      list->GoForward();
      CHECK(list->CurrentItem() == b.get());
      list->GoToItem(*c);
      CHECK(list->CurrentItem() == c.get());
    }

    SECTION("adding an item drops the items after the current one")
    {
      list->GoToItem(*a);
      auto d = Item(u8"https://example.com/d.html");
      list->AddItem(Ref<HistoryItem>(d));
      CHECK(list->Entries().size() == 2);
      CHECK(list->CurrentItem() == d.get());
      CHECK_FALSE(list->ContainsItem(*b));
      CHECK_FALSE(list->ContainsItem(*c));
    }

    SECTION("the oldest item goes past the capacity")
    {
      list->SetCapacity(3);
      auto d = Item(u8"https://example.com/d.html");
      list->AddItem(Ref<HistoryItem>(d));
      CHECK(list->Entries().size() == 3);
      CHECK_FALSE(list->ContainsItem(*a));
      CHECK(list->CurrentItem() == d.get());

      list->SetCapacity(1);
      CHECK(list->Entries().size() == 1);
      CHECK(list->CurrentItem() == b.get());
    }

    SECTION("back and forward lists with limits")
    {
      list->GoToItem(*b);
      auto back = list->BackListWithLimit(5);
      REQUIRE(back.size() == 1);
      CHECK(back[0].get() == a.get());
      auto forward = list->ForwardListWithLimit(5);
      REQUIRE(forward.size() == 1);
      CHECK(forward[0].get() == c.get());
    }

    SECTION("removing items")
    {
      list->RemoveItem(*c);
      CHECK(list->CurrentItem() == b.get());
      list->RemoveItem(*a);
      CHECK(list->CurrentItem() == b.get());
      CHECK(list->BackListCount() == 0);
    }

    SECTION("disabling clears the list, and adds nothing")
    {
      list->SetEnabled(false);
      CHECK(list->Entries().empty());
      CHECK_FALSE(list->CurrentItem());
      list->AddItem(Item(u8"https://example.com/d.html"));
      CHECK(list->Entries().empty());
    }

    SECTION("closing empties the list")
    {
      list->Close();
      CHECK(list->Closed());
      CHECK_FALSE(list->CurrentItem());
      CHECK(list->ForwardListCount() == 0);
    }
  }
}
