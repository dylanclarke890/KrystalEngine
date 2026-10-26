#include "Krystal.Booey/Page/DragController.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/DragEvent.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/DragClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/DragData.hpp"
#include <algorithm>

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Runs the drags that start in the page as the host would, keeping the data dragged.
    struct TestDragClient final : DragClient
    {
      EnumSet<DragSourceAction> Mask = AnyDragSourceAction();
      int32 Starts {0};
      int32 Ended {0};
      Maybe<DragItem> LastItem;
      SelectionData Data;
      EnumSet<DragOperation> SourceMask;
      List<DragDestinationAction> DestinationActions;
      List<DragSourceAction> SourceActions;

      void WillPerformDragDestinationAction(DragDestinationAction action, const DragData &) noexcept override
      {
        DestinationActions.push_back(action);
      }

      void WillPerformDragSourceAction(DragSourceAction action, const IntPoint &,
                                       dom::DataTransfer &) noexcept override
      {
        SourceActions.push_back(action);
      }

      KRYS_NODISCARD EnumSet<DragSourceAction>
        DragSourceActionMaskForPoint(const IntPoint &) noexcept override
      {
        return Mask;
      }

      void StartDrag(DragItem dragItem, dom::DataTransfer &dataTransfer, LocalFrame &) noexcept override
      {
        ++Starts;
        LastItem = krys::move(dragItem);
        Data = dataTransfer.GetPasteboard().GetSelectionData();
        SourceMask = dataTransfer.SourceOperationMask();
      }

      void DragEnded() noexcept override
      {
        ++Ended;
      }
    };

    struct FunctionListener final : dom::EventListenerCallback
    {
      Func<void(dom::Event &)> Function;

      explicit FunctionListener(Func<void(dom::Event &)> &&function) noexcept : Function(krys::move(function))
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        Function(event);
      }
    };

    /// @brief A 400x300 view of @p markup in a page whose drags a `TestDragClient` runs, with no body margin,
    /// 10px text on 20px lines and 40px tall divs.
    struct DragFixture : style::tests::ResolverFixture
    {
      RefPtr<Page> TestPage;
      TestDragClient *Client {nullptr};
      RecordingPasteboardStrategy Pasteboard;
      List<utf8_string> Log;

      explicit DragFixture(utf8_stringview markup) noexcept
          : ResolverFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}div{height:40px}"
                            u8"</style>"
                              + utf8_string(markup),
                            style::tests::WithView::No)
      {
        auto client = CreateUnique<TestDragClient>();
        Client = client.get();
        TestPage =
          Page::Create(nullptr, CreateUnique<TestEditorClient>(), nullptr, nullptr, krys::move(client));
        Frame = ShareRefPtr(&TestPage->MainFrame());
        Frame->CreateView({400, 300});
        Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
        (void)Document->UpdateLayout();
        PlatformStrategies::Singleton().SetPasteboardStrategy(&Pasteboard);
      }

      ~DragFixture() noexcept
      {
        PlatformStrategies::Singleton().SetPasteboardStrategy(nullptr);
      }

      KRYS_NODISCARD EventHandler &Handler() noexcept
      {
        return Frame->GetEventHandler();
      }

      KRYS_NODISCARD DragController &Controller() noexcept
      {
        return TestPage->GetDragController();
      }

      KRYS_NODISCARD FrameSelection &Selection() noexcept
      {
        return Document->Selection();
      }

      KRYS_NODISCARD dom::Text &TextIn(dom::DOMStringView id) noexcept
      {
        return Downcast<dom::Text>(*ById(id).FirstChild());
      }

      void Select(dom::Node &node, uint32 start, uint32 end) noexcept
      {
        Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&node), start),
                           dom::MakeContainerOffsetPosition(ShareRefPtr(&node), end), Affinity::Downstream);
      }

      /// @brief Calls @p function for each event of @p type.
      void ListenWith(utf8_stringview type, Func<void(dom::Event &)> &&function) noexcept
      {
        Document->AddEventListener(
          dom::DOMStringAtom(type),
          Ref<dom::EventListenerCallback>(CreateRef<FunctionListener>(krys::move(function))), true);
      }

      /// @brief Logs each drag event as its type and its target's id.
      void LogDragEvents() noexcept
      {
        for (auto type : {EventNames::DragStart, EventNames::Drag, EventNames::DragEnter,
                          EventNames::DragOver, EventNames::DragLeave, EventNames::Drop, EventNames::DragEnd})
        {
          ListenWith(type,
                     [this](dom::Event &event)
                     {
                       auto *element =
                         DynamicDowncast<dom::Element>(DynamicDowncast<dom::Node>(event.Target().get()));
                       utf8_string id =
                         element ? element->GetAttribute(u8"id").value_or(dom::DOMString()) : utf8_string();
                       Log.push_back(utf8_string(event.Type().View()) + u8"@" + id);
                     });
        }
      }

      KRYS_NODISCARD std::string LogText() const noexcept
      {
        std::string text;
        for (auto &entry : Log)
        {
          if (!text.empty())
          {
            text += ' ';
          }
          text += Narrow(entry);
        }
        return text;
      }

      KRYS_NODISCARD static PlatformMouseEvent Event(PlatformEvent::Type type, double x, double y,
                                                     MouseButton button, int32 clickCount) noexcept
      {
        return PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), button, type, clickCount, {},
                                  MonotonicTime::Now());
      }

      void Press(double x, double y) noexcept
      {
        (void)Handler().HandleMousePressEvent(
          Event(PlatformEvent::Type::MousePressed, x, y, MouseButton::Left, 1));
      }

      void Move(double x, double y) noexcept
      {
        (void)Handler().MouseMoved(Event(PlatformEvent::Type::MouseMoved, x, y, MouseButton::Left, 0));
      }

      void Release(double x, double y) noexcept
      {
        (void)Handler().HandleMouseReleaseEvent(
          Event(PlatformEvent::Type::MouseReleased, x, y, MouseButton::Left, 1));
      }

      KRYS_NODISCARD DragData Data(int32 x, int32 y, EnumSet<DragApplicationFlags> flags = {}) noexcept
      {
        return DragData(&Client->Data, IntPoint(x, y), IntPoint(x, y), Client->SourceMask, flags);
      }

      /// @brief Moves the drag to (@p x, @p y), as the host does as the cursor moves.
      Maybe<DragOperation> DragTo(int32 x, int32 y, EnumSet<DragApplicationFlags> flags = {}) noexcept
      {
        return Controller().DragEnteredOrUpdated(*Frame, Data(x, y, flags));
      }

      /// @brief Drops the drag at (@p x, @p y).
      bool Drop(int32 x, int32 y, EnumSet<DragApplicationFlags> flags = {}) noexcept
      {
        return Controller().PerformDragOperation(Data(x, y, flags));
      }

      /// @brief The drag left the view at (@p x, @p y).
      void Exit(int32 x, int32 y) noexcept
      {
        Controller().DragExited(*Frame, Data(x, y));
      }

      /// @brief Ends the drag that started in the page, having done @p operation.
      void End(double x, double y, EnumSet<DragOperation> operation) noexcept
      {
        Controller().DragEnded();
        Handler().DragSourceEndedAt(Event(PlatformEvent::Type::MouseMoved, x, y, MouseButton::Left, 0),
                                    operation);
      }

      KRYS_NODISCARD std::string InnerHTML(const dom::Node &node) noexcept
      {
        return Narrow(SerializeFragment(node, SerializedNodes::SubtreesOfChildren));
      }
    };

    KRYS_NODISCARD dom::DataTransfer &DataTransferOf(dom::Event &event) noexcept
    {
      auto *dataTransfer = Downcast<dom::DragEvent>(event).GetDataTransfer();
      krys_assert(dataTransfer);
      return *dataTransfer;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragController: dragging a draggable element",
                   "[Page][DragController]")
  {
    DragFixture fixture(u8"<div id=src draggable=true>source</div><div id=dst>target</div>");
    auto &document = *fixture.Document;
    fixture.LogDragEvents();

    std::string readInDragOver = "unread";
    bool dragOverSawType = false;
    std::string readInDrop;
    std::string dropEffectAtEnd;
    fixture.ListenWith(EventNames::DragStart,
                       [&](dom::Event &event)
                       {
                         auto &dataTransfer = DataTransferOf(event);
                         CHECK(dataTransfer.CanWriteData());
                         dataTransfer.SetData(document, u8"text/plain", u8"payload");
                       });
    auto accept = [&](dom::Event &event)
    {
      auto *target = DynamicDowncast<dom::Element>(DynamicDowncast<dom::Node>(event.Target().get()));
      if (!target || target->GetAttribute(u8"id") != dom::DOMString(u8"dst"))
      {
        return;
      }
      auto &dataTransfer = DataTransferOf(event);
      if (event.Type() == EventNames::DragOver)
      {
        // Drop targets see the types, not the data.
        readInDragOver = Narrow(dataTransfer.GetData(document, u8"text/plain"));
        auto types = dataTransfer.Types(document);
        dragOverSawType = std::ranges::find(types, dom::DOMString(u8"text/plain")) != types.end();
      }
      dataTransfer.SetDropEffect(u8"move");
      event.PreventDefault();
    };
    fixture.ListenWith(EventNames::DragEnter, accept);
    fixture.ListenWith(EventNames::DragOver, accept);
    fixture.ListenWith(EventNames::Drop,
                       [&](dom::Event &event)
                       {
                         readInDrop = Narrow(DataTransferOf(event).GetData(document, u8"text/plain"));
                         event.PreventDefault();
                       });
    fixture.ListenWith(EventNames::DragEnd, [&](dom::Event &event)
                       { dropEffectAtEnd = Narrow(DataTransferOf(event).DropEffect()); });

    // A press on the element and a move within the hysteresis don't start a drag.
    fixture.Press(10, 10);
    fixture.Move(10, 12);
    CHECK(fixture.Client->Starts == 0);
    CHECK(fixture.LogText().empty());

    // Moving past it does: dragstart at the source, then the client runs the drag.
    fixture.Move(10, 20);
    CHECK(fixture.Client->Starts == 1);
    CHECK(fixture.LogText() == "dragstart@src");
    REQUIRE(fixture.Client->LastItem);
    CHECK(fixture.Client->LastItem->SourceAction == DragSourceAction::DHTML);
    CHECK(EventHandler::DraggedElement() == &fixture.ById(u8"src"));
    CHECK(fixture.Client->Data.Text() == u8"payload");
    CHECK(fixture.Client->SourceMask == AnyDragOperation());

    // Over the target: drag at the source, then dragenter; the next move fires only dragover (drag fired with
    // dragenter), and the ones after drag and dragover.
    fixture.Log.clear();
    CHECK(fixture.DragTo(10, 60) == DragOperation::Move);
    CHECK(fixture.LogText() == "drag@src dragenter@dst");
    fixture.Log.clear();
    CHECK(fixture.DragTo(10, 62) == DragOperation::Move);
    CHECK(fixture.LogText() == "dragover@dst");
    fixture.Log.clear();
    CHECK(fixture.DragTo(10, 64) == DragOperation::Move);
    CHECK(fixture.LogText() == "drag@src dragover@dst");
    CHECK(readInDragOver.empty());
    CHECK(dragOverSawType);
    CHECK(fixture.Controller().GetDragHandlingMethod() == DragHandlingMethod::NonDefault);

    // The drop reads the data; dragend at the source has the operation done.
    fixture.Log.clear();
    CHECK(fixture.Drop(10, 62));
    CHECK(readInDrop == "payload");
    fixture.End(10, 62, {DragOperation::Generic, DragOperation::Move});
    CHECK(fixture.LogText() == "drop@dst dragend@src");
    CHECK(dropEffectAtEnd == "move");
    CHECK(fixture.Client->Ended == 1);
    CHECK(!EventHandler::DraggedElement());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragController: what doesn't start a drag",
                   "[Page][DragController]")
  {
    DragFixture fixture(u8"<div id=src draggable=true>source</div><p id=p>plain text</p>");
    fixture.LogDragEvents();

    SECTION("Text that isn't selected is selected instead")
    {
      fixture.Press(1, 50);
      fixture.Move(40, 50);
      fixture.Release(40, 50);
      CHECK(fixture.Client->Starts == 0);
      CHECK(fixture.Selection().IsRange());
    }

    SECTION("Nothing is draggable where the client allows no drags")
    {
      fixture.Client->Mask = {};
      fixture.Press(10, 10);
      fixture.Move(10, 30);
      CHECK(fixture.Client->Starts == 0);
      CHECK(fixture.LogText().empty());
    }

    SECTION("A cancelled dragstart")
    {
      fixture.ListenWith(EventNames::DragStart, [](dom::Event &event) { event.PreventDefault(); });
      fixture.Press(10, 10);
      fixture.Move(10, 30);
      CHECK(fixture.Client->Starts == 0);
      CHECK(fixture.LogText() == "dragstart@src");
      CHECK(!EventHandler::DraggedElement());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragController: dragging a link", "[Page][DragController]")
  {
    DragFixture fixture(u8"<a id=link href=\"https://example.com/x\">example link</a>");

    // Links have a larger hysteresis.
    fixture.Press(5, 10);
    fixture.Move(5, 30);
    CHECK(fixture.Client->Starts == 0);
    fixture.Move(5, 60);
    REQUIRE(fixture.Client->Starts == 1);
    CHECK(fixture.Client->LastItem->SourceAction == DragSourceAction::Link);
    CHECK(fixture.Client->LastItem->Url.String() == u8"https://example.com/x");
    CHECK(fixture.Client->Data.HasURL());
    CHECK(fixture.Client->Data.GetURL().String() == u8"https://example.com/x");
    CHECK(fixture.Client->Data.Markup() == u8"<a href=\"https://example.com/x\">example link</a>");
    CHECK(fixture.Client->SourceActions == List<DragSourceAction> {DragSourceAction::Link});
    fixture.End(5, 60, {});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragController: refusing and cancelling drags",
                   "[Page][DragController]")
  {
    DragFixture fixture(u8"<div id=src draggable=true>source</div><div id=dst>target</div>");
    fixture.LogDragEvents();
    std::string dropEffectAtEnd;
    fixture.ListenWith(EventNames::DragEnd, [&](dom::Event &event)
                       { dropEffectAtEnd = Narrow(DataTransferOf(event).DropEffect()); });
    fixture.Press(10, 10);
    fixture.Move(10, 20);
    REQUIRE(fixture.Client->Starts == 1);
    fixture.Log.clear();

    SECTION("A target that doesn't cancel dragover refuses the drop")
    {
      CHECK(!fixture.DragTo(10, 60));
      CHECK(!fixture.DragTo(10, 62));
      CHECK(!fixture.Drop(10, 62));
      fixture.End(10, 62, {});
      CHECK(fixture.LogText() == "drag@src dragenter@dst dragover@dst dragend@src");
      CHECK(dropEffectAtEnd == "none");
    }

    SECTION("Leaving the view")
    {
      (void)fixture.DragTo(10, 60);
      fixture.Exit(10, 60);
      fixture.End(10, 60, {});
      CHECK(fixture.LogText() == "drag@src dragenter@dst drag@src dragleave@dst dragend@src");
      CHECK(dropEffectAtEnd == "none");
    }

    SECTION("An operation the source doesn't allow")
    {
      fixture.Client->SourceMask = {DragOperation::Copy};
      fixture.ListenWith(EventNames::DragOver,
                         [](dom::Event &event)
                         {
                           DataTransferOf(event).SetDropEffect(u8"move");
                           event.PreventDefault();
                         });
      (void)fixture.DragTo(10, 60);
      CHECK(!fixture.DragTo(10, 62));
      fixture.End(10, 62, {});
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragController: the source's effects", "[Page][DragController]")
  {
    DragFixture fixture(u8"<div id=src draggable=true>source</div><div id=dst>target</div>");
    fixture.ListenWith(EventNames::DragStart,
                       [](dom::Event &event)
                       {
                         auto &dataTransfer = DataTransferOf(event);
                         dataTransfer.SetEffectAllowed(u8"bogus");
                         CHECK(dataTransfer.EffectAllowed() == u8"uninitialized");
                         dataTransfer.SetEffectAllowed(u8"copyLink");
                       });
    fixture.Press(10, 10);
    fixture.Move(10, 20);
    REQUIRE(fixture.Client->Starts == 1);
    CHECK(fixture.Client->SourceMask == EnumSet<DragOperation> {DragOperation::Copy, DragOperation::Link});

    // A target that accepts without choosing gets the source's first allowed operation.
    fixture.ListenWith(EventNames::DragOver, [](dom::Event &event) { event.PreventDefault(); });
    fixture.ListenWith(EventNames::DragEnter, [](dom::Event &event) { event.PreventDefault(); });
    CHECK(fixture.DragTo(10, 60) == DragOperation::Copy);
    fixture.End(10, 60, {});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragController: dropping into editable content",
                   "[Page][DragController]")
  {
    DragFixture fixture(
      u8"<div id=a contenteditable>abc def</div><div id=b contenteditable>xyz</div><div id=c>"
      u8"static</div>");
    auto &a = fixture.ById(u8"a");
    auto &b = fixture.ById(u8"b");

    SECTION("Moving the selection within the document")
    {
      fixture.Select(fixture.TextIn(u8"a"), 0, 3);
      fixture.Press(5, 10);
      fixture.Move(5, 20);
      REQUIRE(fixture.Client->Starts == 1);
      CHECK(fixture.Client->LastItem->SourceAction == DragSourceAction::Selection);
      CHECK(fixture.Client->Data.Text() == u8"abc");

      CHECK(fixture.DragTo(200, 50) == DragOperation::Move);
      CHECK(fixture.TestPage->GetDragCaretController().HasCaret());
      CHECK(fixture.Controller().GetDragHandlingMethod() == DragHandlingMethod::EditRichText);
      CHECK(fixture.Drop(200, 50));
      fixture.End(200, 50, {DragOperation::Move});
      CHECK(!fixture.TestPage->GetDragCaretController().HasCaret());
      CHECK(fixture.InnerHTML(a) == "&nbsp;def");
      CHECK(fixture.InnerHTML(b) == "xyzabc");
    }

    SECTION("Copying it with the copy key")
    {
      fixture.Select(fixture.TextIn(u8"a"), 0, 3);
      fixture.Press(5, 10);
      fixture.Move(5, 20);
      REQUIRE(fixture.Client->Starts == 1);

      CHECK(fixture.DragTo(200, 50, {DragApplicationFlags::IsCopyKeyDown}) == DragOperation::Copy);
      CHECK(fixture.Drop(200, 50, {DragApplicationFlags::IsCopyKeyDown}));
      fixture.End(200, 50, {DragOperation::Copy});
      CHECK(fixture.InnerHTML(a) == "abc def");
      CHECK(fixture.InnerHTML(b) == "xyzabc");
    }

    SECTION("Text from elsewhere")
    {
      fixture.Client->Data.SetText(u8"dropped");
      fixture.Client->SourceMask = AnyDragOperation();
      CHECK(fixture.DragTo(200, 50) == DragOperation::Copy);
      CHECK(fixture.Drop(200, 50));
      CHECK(fixture.InnerHTML(b) == "xyzdropped");
      // The page's drop handlers had their chance first.
      CHECK(fixture.Client->DestinationActions
            == List<DragDestinationAction> {DragDestinationAction::DHTML, DragDestinationAction::Edit});
    }

    SECTION("Not over editable content, the drag caret goes")
    {
      fixture.Client->Data.SetText(u8"dropped");
      fixture.Client->SourceMask = AnyDragOperation();
      (void)fixture.DragTo(200, 50);
      auto &caret = fixture.TestPage->GetDragCaretController();
      CHECK(caret.HasCaret());
      CHECK(caret.IsContentEditable());
      // The block it's in paints it.
      CHECK(caret.CaretRenderer() == fixture.ById(u8"b").Renderer());
      CHECK(!fixture.DragTo(200, 90));
      CHECK(!fixture.TestPage->GetDragCaretController().HasCaret());
      fixture.Exit(200, 90);
    }

    SECTION("A cancelled textInput stops the drop")
    {
      fixture.Client->Data.SetText(u8"dropped");
      fixture.Client->SourceMask = AnyDragOperation();
      fixture.ListenWith(EventNames::TextInput, [](dom::Event &event) { event.PreventDefault(); });
      (void)fixture.DragTo(200, 50);
      CHECK(fixture.Drop(200, 50));
      CHECK(fixture.InnerHTML(b) == "xyz");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragController: dropping into form controls",
                   "[Page][DragController]")
  {
    DragFixture fixture(u8"<input id=t><br><input id=f type=file multiple>");
    auto &text = Downcast<html::HTMLInputElement>(fixture.ById(u8"t"));
    auto &file = Downcast<html::HTMLInputElement>(fixture.ById(u8"f"));
    fixture.Client->SourceMask = AnyDragOperation();

    SECTION("Text into a text field")
    {
      int32 inputs = 0;
      fixture.ListenWith(EventNames::Input, [&](dom::Event &) { ++inputs; });
      fixture.Client->Data.SetText(u8"dropped");
      CHECK(fixture.DragTo(10, 8) == DragOperation::Copy);
      CHECK(fixture.Controller().GetDragHandlingMethod() == DragHandlingMethod::EditPlainText);
      CHECK(fixture.Drop(10, 8));
      CHECK(text.Value() == u8"dropped");
      CHECK(inputs == 1);
    }

    SECTION("Files into a file input")
    {
      int32 changes = 0;
      fixture.ListenWith(EventNames::Change, [&](dom::Event &) { ++changes; });
      fixture.Client->Data.SetURIList(u8"file:///C:/drops/a.txt\r\nfile:///C:/drops/b.txt");
      auto rect = file.BoundingClientRect();
      auto x = static_cast<int32>(rect.X() + 5);
      auto y = static_cast<int32>(rect.Y() + 5);
      CHECK(fixture.DragTo(x, y) == DragOperation::Copy);
      CHECK(fixture.Controller().GetDragHandlingMethod() == DragHandlingMethod::UploadFile);
      CHECK(fixture.Controller().NumberOfItemsToBeAccepted() == 2);
      CHECK(file.CanReceiveDroppedFiles());
      CHECK(fixture.Drop(x, y));
      CHECK(!file.CanReceiveDroppedFiles());
      REQUIRE(file.Files());
      CHECK(file.Files()->Length() == 2);
      CHECK(changes == 1);
    }

    SECTION("Too many files for a file input that takes one")
    {
      file.RemoveAttribute(u8"multiple");
      (void)fixture.Document->UpdateLayout();
      fixture.Client->Data.SetURIList(u8"file:///C:/drops/a.txt\r\nfile:///C:/drops/b.txt");
      auto rect = file.BoundingClientRect();
      CHECK(!fixture.DragTo(static_cast<int32>(rect.X() + 5), static_cast<int32>(rect.Y() + 5)));
      CHECK(fixture.Controller().NumberOfItemsToBeAccepted() == 0);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragController: URLs dropped on a page",
                   "[Page][DragController]")
  {
    DragFixture fixture(u8"<div id=c>static</div><div id=e contenteditable>editable</div>");
    fixture.Client->Data.SetURL(url::URL(u8"https://example.com/"), u8"Example");
    fixture.Client->SourceMask = AnyDragOperation();

    // Over content that isn't editable, the page would load it.
    CHECK(fixture.DragTo(10, 10) == DragOperation::Copy);
    CHECK(fixture.Controller().GetDragHandlingMethod() == DragHandlingMethod::PageLoad);

    // Editable content takes it as a link.
    CHECK(fixture.DragTo(100, 50) == DragOperation::Copy);
    CHECK(fixture.Drop(100, 50));
    CHECK(fixture.InnerHTML(fixture.ById(u8"e")) == "editable<a href=\"https://example.com/\">Example</a>");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DragController: draggable elements that can't be selected",
                   "[Page][DragController]")
  {
    DragFixture fixture(u8"<div id=src draggable=true style=\"-webkit-user-select:none\">source</div>");
    CHECK(!fixture.ById(u8"src").FirstChild()->CanStartSelection());
    fixture.ById(u8"src").RemoveAttribute(u8"draggable");
    (void)fixture.Document->UpdateLayout();
    CHECK(fixture.ById(u8"src").FirstChild()->CanStartSelection());
  }
  TEST_CASE_METHOD(css::tests::ValueFixture, "DragController: drag images", "[Page][DragController]")
  {
    SECTION("A draggable element is snapshotted, under the cursor where it was pressed")
    {
      DragFixture fixture(u8"<div id=src draggable=true style=\"width:100px\">source</div>");
      fixture.Press(10, 15);
      fixture.Move(10, 30);
      REQUIRE(fixture.Client->LastItem);
      auto &item = *fixture.Client->LastItem;
      REQUIRE(item.Image);
      CHECK(DragImageSize(item.Image.Get()) == IntSize(100, 40));
      CHECK(item.EventPositionInContentCoordinates == IntPoint(10, 15));
      CHECK(item.DragLocationInContentCoordinates == IntPoint(0, 0));
      CHECK(item.DragPreviewFrameInRootViewCoordinates == IntRect(0, 0, 100, 40));
      fixture.End(10, 30, {});
    }

    SECTION("The page's own image, from dragstart")
    {
      DragFixture fixture(u8"<div id=src draggable=true>source</div>"
                          u8"<div id=image style=\"width:30px;height:20px\">image</div>");
      fixture.ListenWith(EventNames::DragStart, [&](dom::Event &event)
                         { DataTransferOf(event).SetDragImage(ShareRef(fixture.ById(u8"image")), 5, 6); });
      fixture.Press(10, 15);
      fixture.Move(10, 30);
      REQUIRE(fixture.Client->LastItem);
      auto &item = *fixture.Client->LastItem;
      REQUIRE(item.Image);
      CHECK(DragImageSize(item.Image.Get()) == IntSize(30, 20));
      CHECK(item.DragLocationInContentCoordinates == IntPoint(5, 9));
      CHECK(item.DragPreviewFrameInRootViewCoordinates.Size() == IntSize(30, 20));
      fixture.End(10, 30, {});
    }

    SECTION("A link's label, centred below the cursor")
    {
      DragFixture fixture(u8"<a id=link href=\"https://example.com/x\">example link</a>");
      fixture.Press(5, 10);
      fixture.Move(5, 60);
      REQUIRE(fixture.Client->LastItem);
      auto &item = *fixture.Client->LastItem;
      REQUIRE(item.Image);
      auto size = DragImageSize(item.Image.Get());
      CHECK(size.Width() <= 200);
      CHECK(item.DragLocationInContentCoordinates == IntPoint(5 - size.Width() / 2, 8));
      fixture.End(5, 60, {});
    }

    SECTION("A selection, faded")
    {
      DragFixture fixture(u8"<p id=p>drag this text</p>");
      fixture.Select(fixture.TextIn(u8"p"), 0, 4);
      fixture.Press(5, 10);
      fixture.Move(5, 30);
      REQUIRE(fixture.Client->LastItem);
      CHECK(fixture.Client->LastItem->SourceAction == DragSourceAction::Selection);
      CHECK(fixture.Client->LastItem->Image);
      fixture.End(5, 30, {});
    }
  }
}
