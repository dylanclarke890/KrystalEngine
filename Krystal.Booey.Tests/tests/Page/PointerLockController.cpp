#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/PlatformWheelEvent.hpp"
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    using Strings = List<std::string>;
    using Type = PlatformEvent::Type;

    /// @brief A host that answers requests to lock the pointer with @p Result and to unlock it with
    /// @p UnlockResult, counting them.
    class LockChromeClient final : public ChromeClient
    {
    public:
      PointerLockRequestResult Result {PointerLockRequestResult::Success};
      bool UnlockResult {true};
      bool Mouse {true};
      uint32 Locks {0};
      uint32 Unlocks {0};

      void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
      {
      }

      KRYS_NODISCARD bool HasAccessoryMousePointingDevice() const noexcept final
      {
        return Mouse;
      }

      void RequestPointerLock(Func<void(PointerLockRequestResult)> &&completionHandler) noexcept final
      {
        ++Locks;
        completionHandler(Result);
      }

      void RequestPointerUnlock(Func<void(bool)> &&completionHandler) noexcept final
      {
        ++Unlocks;
        completionHandler(UnlockResult);
      }
    };

    KRYS_NODISCARD std::string Name(const dom::EventTarget *target)
    {
      auto *node = DynamicDowncast<dom::Node>(target);
      auto *element = node ? DynamicDowncast<dom::Element>(*node) : nullptr;
      if (!element)
      {
        return node ? "document" : "null";
      }
      auto id = element->GetAttribute(u8"id").value_or(dom::DOMString());
      return std::string(id.begin(), id.end());
    }

    /// @brief Records the events it handles as `type:target`, with a mouse move's movement after its type
    /// (`mousemove(5,-3)`).
    struct Recorder : public dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit Recorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto type = std::string(event.Type().View().begin(), event.Type().View().end());
        if (event.Type() == EventNames::MouseMove)
        {
          auto &mouse = static_cast<dom::MouseEvent &>(event);
          type += "(" + std::to_string(static_cast<int>(mouse.MovementX())) + ","
                  + std::to_string(static_cast<int>(mouse.MovementY())) + ")";
        }
        Records.push_back(type + ":" + Name(event.Target().get()));
      }
    };

    /// @brief Two 100x50 boxes, `a` above `b`, in a page whose host can lock the pointer, recording the
    /// events at its document.
    struct LockFixture : style::tests::ResolverFixture
    {
      LockChromeClient *Chrome {nullptr};
      RefPtr<Page> TestPage;
      List<std::string> Records;

      LockFixture() noexcept
          : ResolverFixture(u8"<!DOCTYPE html><style>body{margin:0}div{width:100px;height:50px}</style>"
                            u8"<div id=a></div><div id=b></div><div id=host></div>",
                            style::tests::WithView::No)
      {
        auto chrome = CreateUnique<LockChromeClient>();
        Chrome = chrome.get();
        TestPage = Page::Create(krys::move(chrome));
        Frame = ShareRefPtr(&TestPage->MainFrame());
        Frame->CreateView({200, 200});
        Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
        (void)Document->UpdateLayout();

        // Capturing at the document sees the events that don't bubble too.
        Ref<dom::EventListenerCallback> recorder = CreateRef<Recorder>(Records);
        for (auto type : {u8"pointerlockchange", u8"pointerlockerror", u8"pointerdown", u8"pointermove",
                          u8"pointerup", u8"mousedown", u8"mousemove", u8"mouseup", u8"click", u8"wheel"})
        {
          Document->AddEventListener(dom::DOMStringAtom(type), Ref {recorder}, true);
        }
      }

      KRYS_NODISCARD PointerLockController &Controller() noexcept
      {
        return TestPage->GetPointerLockController();
      }

      KRYS_NODISCARD EventHandler &Handler() noexcept
      {
        return Frame->GetEventHandler();
      }

      /// @brief A user gesture: the window gets transient activation.
      void Activate() noexcept
      {
        Document->Window()->NotifyActivated();
      }

      /// @brief Ends the window's transient activation, as time passes.
      void Deactivate() noexcept
      {
        (void)Document->Window()->ConsumeTransientActivation();
      }

      /// @brief Asks to lock the pointer to @p element, then runs the tasks and microtasks that follow.
      /// @return How the request's promise settled: `ok`, or the exception's code.
      std::string Lock(dom::Element &element, dom::PointerLockOptions options = {}) noexcept
      {
        std::string settled = "pending";
        element.RequestPointerLock(
          options,
          [&](dom::ExceptionOr<void> &&result)
          {
            settled =
              !result.HasException()                                                   ? "ok"
              : result.GetException().Code() == dom::ExceptionCode::NotAllowedError    ? "NotAllowedError"
              : result.GetException().Code() == dom::ExceptionCode::WrongDocumentError ? "WrongDocumentError"
              : result.GetException().Code() == dom::ExceptionCode::NotSupportedError  ? "NotSupportedError"
              : result.GetException().Code() == dom::ExceptionCode::InvalidStateError  ? "InvalidStateError"
                                                                                       : "other";
          });
        html::EventLoop::Current().RunUntilIdle();
        return settled;
      }

      void Run() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
      }

      KRYS_NODISCARD static PlatformMouseEvent Mouse(Type type, double x, double y, DoublePoint movement = {},
                                                     uint16 buttons = 0) noexcept
      {
        PlatformMouseEvent event(DoublePoint(x, y), DoublePoint(x, y),
                                 type == Type::MouseMoved ? MouseButton::None : MouseButton::Left, type,
                                 type == Type::MouseMoved ? 0 : 1, {}, MonotonicTime::Now(), buttons,
                                 movement);
        if (type == Type::MouseMoved)
        {
          event.SetCoalescedEvents({event});
        }
        return event;
      }

      /// @brief The records since the last call at the boxes and the document (not the body or the root).
      KRYS_NODISCARD Strings Take() noexcept
      {
        Strings records;
        for (auto &record : std::exchange(Records, {}))
        {
          if (record.ends_with(":a") || record.ends_with(":b") || record.ends_with(":document")
              || record.ends_with(":host"))
          {
            records.push_back(record);
          }
        }
        return records;
      }
    };
  }

  TEST_CASE("PointerLockController: locking, the locked element's events and unlocking",
            "[Page][PointerLock]")
  {
    LockFixture fixture;
    auto &a = fixture.ById(u8"a");
    auto &b = fixture.ById(u8"b");

    // Without a user gesture, the request fails and the host isn't asked.
    CHECK(fixture.Lock(a) == "NotAllowedError");
    CHECK(fixture.Chrome->Locks == 0);
    CHECK(fixture.Take() == Strings {"pointerlockerror:document"});
    CHECK(fixture.Document->PointerLockElement() == nullptr);

    // With one, the host locks the pointer.
    fixture.Activate();
    CHECK(fixture.Lock(a) == "ok");
    CHECK(fixture.Chrome->Locks == 1);
    CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
    CHECK(fixture.Controller().IsLocked());
    CHECK(fixture.Document->PointerLockElement().get() == &a);

    // The mouse's events go to the locked element, wherever the mouse is, with the movement.
    (void)fixture.Handler().MouseMoved(LockFixture::Mouse(Type::MouseMoved, 50, 75, {5, -3}));
    (void)fixture.Handler().HandleMousePressEvent(LockFixture::Mouse(Type::MousePressed, 50, 75, {}, 1));
    (void)fixture.Handler().HandleMouseReleaseEvent(LockFixture::Mouse(Type::MouseReleased, 50, 75));
    (void)fixture.Handler().HandleWheelEvent(
      PlatformWheelEvent(IntPoint(50, 75), IntPoint(50, 75), 0, -40, 0, -1,
                         PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, MonotonicTime::Now()));
    CHECK(fixture.Take()
          == Strings {"pointermove:a", "mousemove(5,-3):a", "pointerdown:a", "mousedown:a", "pointerup:a",
                      "mouseup:a", "click:a", "wheel:a"});

    // Pointers can't be captured while it's locked.
    CHECK(b.SetPointerCapture(1).GetException().Code() == dom::ExceptionCode::InvalidStateError);

    // Moving the lock to another element of the document needs no host.
    CHECK(fixture.Lock(b) == "ok");
    CHECK(fixture.Chrome->Locks == 1);
    CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
    CHECK(fixture.Document->PointerLockElement().get() == &b);

    SECTION("exitPointerLock() unlocks it, and the document may lock it again without a gesture")
    {
      fixture.Document->ExitPointerLock();
      fixture.Run();
      CHECK(fixture.Chrome->Unlocks == 1);
      CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
      CHECK_FALSE(fixture.Controller().IsLocked());
      CHECK(fixture.Document->PointerLockElement() == nullptr);

      fixture.Deactivate();
      CHECK(fixture.Lock(a) == "ok");
      CHECK(fixture.Chrome->Locks == 2);
      CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
    }

    SECTION("Escape unlocks it, and locking it again needs a gesture")
    {
      fixture.Deactivate();
      (void)fixture.Handler().KeyEvent(PlatformKeyboardEvent(Type::KeyDown, u8"", u8"", u8"Escape",
                                                             u8"Escape", u8"U+001B", 27, false, false, false,
                                                             {}, MonotonicTime::Now()));
      fixture.Run();
      CHECK(fixture.Chrome->Unlocks == 1);
      CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
      CHECK_FALSE(fixture.Controller().IsLocked());

      CHECK(fixture.Lock(a) == "NotAllowedError");
      CHECK(fixture.Take() == Strings {"pointerlockerror:document"});
    }

    SECTION("removing the locked element unlocks it")
    {
      b.Remove();
      fixture.Run();
      CHECK(fixture.Chrome->Unlocks == 1);
      CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
      CHECK_FALSE(fixture.Controller().IsLocked());
      CHECK(fixture.Controller().GetElement() == nullptr);
    }

    SECTION("the mouse's events go where the mouse is once it's unlocked")
    {
      fixture.Document->ExitPointerLock();
      fixture.Run();
      (void)fixture.Take();
      (void)fixture.Handler().MouseMoved(LockFixture::Mouse(Type::MouseMoved, 50, 75, {1, 1}));
      CHECK(fixture.Take() == Strings {"pointermove:b", "mousemove(1,1):b"});
    }
  }

  TEST_CASE("PointerLockController: hosts that refuse, unadjusted movement and shadow trees",
            "[Page][PointerLock]")
  {
    LockFixture fixture;
    auto &a = fixture.ById(u8"a");

    SECTION("a host that doesn't lock the pointer")
    {
      fixture.Chrome->Result = PointerLockRequestResult::Failure;
      fixture.Activate();
      CHECK(fixture.Lock(a) == "WrongDocumentError");
      CHECK(fixture.Take() == Strings {"pointerlockerror:document"});
      CHECK(fixture.Controller().GetElement() == nullptr);

      fixture.Chrome->Result = PointerLockRequestResult::Unsupported;
      CHECK(fixture.Lock(a) == "NotSupportedError");
      CHECK(fixture.Take() == Strings {"pointerlockerror:document"});
      CHECK(fixture.Controller().GetElement() == nullptr);
    }

    SECTION("an element that isn't connected")
    {
      fixture.Activate();
      auto element = fixture.Document->CreateElement(u8"div").ReleaseValue();
      CHECK(fixture.Lock(*element) == "WrongDocumentError");
      CHECK(fixture.Take() == Strings {"pointerlockerror:document"});
      CHECK(fixture.Chrome->Locks == 0);
    }

    SECTION("unadjusted movement, with a mouse")
    {
      fixture.Activate();
      fixture.Chrome->Mouse = false;
      CHECK(fixture.Lock(a, {.UnadjustedMovement = true}) == "NotSupportedError");
      CHECK(fixture.Take() == Strings {"pointerlockerror:document"});

      fixture.Chrome->Mouse = true;
      CHECK(fixture.Lock(a, {.UnadjustedMovement = true}) == "ok");
      (void)fixture.Take();
      auto move = LockFixture::Mouse(Type::MouseMoved, 0, 0, {6, 4});
      move.SetUnadjustedMovementDelta({3, 2});
      (void)fixture.Handler().MouseMoved(move);
      CHECK(fixture.Take() == Strings {"pointermove:a", "mousemove(3,2):a"});
    }

    SECTION("an element in a shadow tree is its host to the document")
    {
      auto &host = fixture.ById(u8"host");
      auto shadow = host.AttachShadow({}).ReleaseValue();
      auto inner = fixture.Document->CreateElement(u8"span").ReleaseValue();
      (void)shadow->AppendChild(*inner);

      fixture.Activate();
      CHECK(fixture.Lock(*inner) == "ok");
      CHECK(fixture.Document->PointerLockElement().get() == &host);
      CHECK(shadow->PointerLockElement().get() == inner.get());
    }
  }
}
