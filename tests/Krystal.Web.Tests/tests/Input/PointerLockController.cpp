#include "Krystal.Web/Input/PointerLockController.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Web/Dom/Events/EventListener.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Html/Window.hpp"
#include "Krystal.Web/Input/EventHandler.hpp"
#include "Krystal.Web/Input/PlatformKeyboardEvent.hpp"
#include "Krystal.Web/Input/PlatformWheelEvent.hpp"
#include "Krystal.Web/Input/PointerCaptureController.hpp"
#include "Krystal.Web/Page/Clients/WebClient.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include <string>

namespace krys::web::tests
{
  namespace
  {
    using Strings = List<std::string>;
    using Type = PlatformEvent::Type;

    /// @brief A host that answers requests to lock the pointer with @p Result and to unlock it with
    /// @p UnlockResult, counting them.
    class LockWebClient final : public WebClient
    {
    public:
      PointerLockRequestResult Result {PointerLockRequestResult::Success};
      bool UnlockResult {true};
      bool Mouse {true};
      uint32 Locks {0};
      uint32 Unlocks {0};

      void AttachRootGraphicsLayer(web::Frame &, GraphicsLayer *) noexcept final
      {
      }

      KRYS_NODISCARD bool HasAccessoryMousePointingDevice() const noexcept final
      {
        return Mouse;
      }

      void RequestPointerLock(Func<void(PointerLockRequestResult)> completionHandler) noexcept final
      {
        ++Locks;
        completionHandler(Result);
      }

      void RequestPointerUnlock(Func<void(bool)> completionHandler) noexcept final
      {
        ++Unlocks;
        completionHandler(UnlockResult);
      }
    };

    KRYS_NODISCARD std::string TargetId(const EventTarget *target)
    {
      auto *node = DynamicDowncast<Node>(target);
      auto *element = node ? DynamicDowncast<Element>(*node) : nullptr;
      if (!element)
      {
        return node ? "document" : "null";
      }
      auto id = element->GetAttribute(u8"id").value_or(DOMString());
      return std::string(id.begin(), id.end());
    }

    /// @brief Records the events it handles as `type:target`, with a mouse move's movement after its type
    /// (`mousemove(5,-3)`).
    struct PointerLockRecorder : public EventListenerCallback
    {
      List<std::string> &Records;

      explicit PointerLockRecorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(Event &event) noexcept override
      {
        auto type = std::string(event.Type().View().begin(), event.Type().View().end());
        if (event.Type() == event_names::MouseMove)
        {
          auto &mouse = static_cast<MouseEvent &>(event);
          type += "(" + std::to_string(static_cast<int>(mouse.MovementX())) + ","
                  + std::to_string(static_cast<int>(mouse.MovementY())) + ")";
        }
        Records.push_back(type + ":" + TargetId(event.Target().get()));
      }
    };

    /// @brief Two 100x50 boxes, `a` above `b`, in a page whose host can lock the pointer, recording the
    /// events at its document.
    struct LockFixture : style::tests::ResolverFixture
    {
      LockWebClient *Client {nullptr};
      List<std::string> Records;

      LockFixture() noexcept
          : ResolverFixture(u8"<!DOCTYPE html><style>body{margin:0}div{width:100px;height:50px}</style>"
                            u8"<div id=a></div><div id=b></div><div id=host></div>",
                            style::tests::WithView::No)
      {
        auto client = CreateUnique<LockWebClient>();
        Client = client.get();
        Frame = web::Frame::Create(krys::move(client));
        Frame->CreateView({200, 200});
        Frame->SetDocument(ShareRefPtr<web::Document>(Document.get()));
        (void)Document->UpdateLayout();

        // Capturing at the document sees the events that don't bubble too.
        Ref<EventListenerCallback> recorder = CreateRef<PointerLockRecorder>(Records);
        for (auto type : {u8"pointerlockchange", u8"pointerlockerror", u8"pointerdown", u8"pointermove",
                          u8"pointerup", u8"mousedown", u8"mousemove", u8"mouseup", u8"click", u8"wheel",
                          u8"gotpointercapture", u8"lostpointercapture"})
        {
          Document->AddEventListener(DOMStringAtom(type), Ref {recorder}, true);
        }
      }

      KRYS_NODISCARD PointerLockController &Controller() const noexcept
      {
        return Frame->GetPointerLockController();
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
      static std::string Lock(Element &element, PointerLockOptions options = {}) noexcept
      {
        std::string settled = "pending";
        element.RequestPointerLock(
          options,
          [&](const ExceptionOr<void> &result)
          {
            settled = !result.HasException()                                           ? "ok"
                      : result.GetException().Code() == ExceptionCode::NotAllowedError ? "NotAllowedError"
                      : result.GetException().Code() == ExceptionCode::WrongDocumentError
                        ? "WrongDocumentError"
                      : result.GetException().Code() == ExceptionCode::NotSupportedError ? "NotSupportedError"
                      : result.GetException().Code() == ExceptionCode::InvalidStateError ? "InvalidStateError"
                                                                                         : "other";
          });
        html::EventLoop::Current().RunUntilIdle();
        return settled;
      }

      static void Run() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
      }

      KRYS_NODISCARD static PlatformMouseEvent Mouse(Type type, double x, double y, PointD movement = {},
                                                     uint16 buttons = 0) noexcept
      {
        PlatformMouseEvent event(
          PointD(x, y), PointD(x, y), type == Type::MouseMoved ? MouseButton::None : MouseButton::Left, type,
          type == Type::MouseMoved ? 0 : 1, {}, MonotonicTime::Now(), buttons, movement);
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
    CHECK(fixture.Client->Locks == 0);
    CHECK(fixture.Take() == Strings {"pointerlockerror:document"});
    CHECK(fixture.Document->PointerLockElement() == nullptr);

    // With one, the host locks the pointer.
    fixture.Activate();
    CHECK(fixture.Lock(a) == "ok");
    CHECK(fixture.Client->Locks == 1);
    CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
    CHECK(fixture.Controller().IsLocked());
    CHECK(fixture.Document->PointerLockElement().get() == &a);

    // The mouse's events go to the locked element, wherever the mouse is, with the movement.
    (void)fixture.Handler().MouseMoved(LockFixture::Mouse(Type::MouseMoved, 50, 75, {5, -3}));
    (void)fixture.Handler().HandleMousePressEvent(LockFixture::Mouse(Type::MousePressed, 50, 75, {}, 1));
    (void)fixture.Handler().HandleMouseReleaseEvent(LockFixture::Mouse(Type::MouseReleased, 50, 75));
    (void)fixture.Handler().HandleWheelEvent(
      PlatformWheelEvent(PointI(50, 75), PointI(50, 75), 0, -40, 0, -1,
                         PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, MonotonicTime::Now()));
    CHECK(fixture.Take()
          == Strings {"pointermove:a", "mousemove(5,-3):a", "pointerdown:a", "mousedown:a", "pointerup:a",
                      "mouseup:a", "click:a", "wheel:a"});

    // Pointers can't be captured while it's locked.
    CHECK(b.SetPointerCapture(1).GetException().Code() == ExceptionCode::InvalidStateError);

    // Moving the lock to another element of the document needs no host.
    CHECK(fixture.Lock(b) == "ok");
    CHECK(fixture.Client->Locks == 1);
    CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
    CHECK(fixture.Document->PointerLockElement().get() == &b);

    SECTION("exitPointerLock() unlocks it, and the document may lock it again without a gesture")
    {
      fixture.Document->ExitPointerLock();
      fixture.Run();
      CHECK(fixture.Client->Unlocks == 1);
      CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
      CHECK_FALSE(fixture.Controller().IsLocked());
      CHECK(fixture.Document->PointerLockElement() == nullptr);

      fixture.Deactivate();
      CHECK(fixture.Lock(a) == "ok");
      CHECK(fixture.Client->Locks == 2);
      CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
    }

    SECTION("Escape unlocks it, and locking it again needs a gesture")
    {
      fixture.Deactivate();
      (void)fixture.Handler().KeyEvent(PlatformKeyboardEvent(Type::KeyDown, u8"", u8"", u8"Escape",
                                                             u8"Escape", 27, false, false, false, {},
                                                             MonotonicTime::Now()));
      fixture.Run();
      CHECK(fixture.Client->Unlocks == 1);
      CHECK(fixture.Take() == Strings {"pointerlockchange:document"});
      CHECK_FALSE(fixture.Controller().IsLocked());

      CHECK(fixture.Lock(a) == "NotAllowedError");
      CHECK(fixture.Take() == Strings {"pointerlockerror:document"});
    }

    SECTION("removing the locked element unlocks it")
    {
      b.Remove();
      fixture.Run();
      CHECK(fixture.Client->Unlocks == 1);
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

    SECTION("locking releases pointer capture")
    {
      auto &b = fixture.ById(u8"b");
      (void)fixture.Handler().HandleMousePressEvent(LockFixture::Mouse(Type::MousePressed, 50, 75, {}, 1));
      REQUIRE_FALSE(b.SetPointerCapture(MousePointerId).HasException());
      (void)fixture.Handler().MouseMoved(LockFixture::Mouse(Type::MouseMoved, 50, 25, {}, 1));
      (void)fixture.Take();
      CHECK(fixture.Frame->GetPointerCaptureController().PointerCaptureElement(fixture.Document.get(),
                                                                               MousePointerId)
            == &b);

      // As if releasePointerCapture() were called, but the locked pointer's events don't process the release,
      // so lostpointercapture fires as the lock is applied.
      fixture.Activate();
      CHECK(fixture.Lock(a) == "ok");
      CHECK(fixture.Take() == Strings {"lostpointercapture:b", "pointerlockchange:document"});
      CHECK_FALSE(b.HasPointerCapture(MousePointerId));
      CHECK_FALSE(fixture.Frame->GetPointerCaptureController().PointerCaptureElement(fixture.Document.get(),
                                                                                     MousePointerId));
    }

    SECTION("a host that doesn't lock the pointer")
    {
      fixture.Client->Result = PointerLockRequestResult::Failure;
      fixture.Activate();
      CHECK(fixture.Lock(a) == "WrongDocumentError");
      CHECK(fixture.Take() == Strings {"pointerlockerror:document"});
      CHECK(fixture.Controller().GetElement() == nullptr);

      fixture.Client->Result = PointerLockRequestResult::Unsupported;
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
      CHECK(fixture.Client->Locks == 0);
    }

    SECTION("unadjusted movement, with a mouse")
    {
      fixture.Activate();
      fixture.Client->Mouse = false;
      CHECK(fixture.Lock(a, {.UnadjustedMovement = true}) == "NotSupportedError");
      CHECK(fixture.Take() == Strings {"pointerlockerror:document"});

      fixture.Client->Mouse = true;
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
