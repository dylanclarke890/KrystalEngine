#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/Network/FormData.hpp"
#include <string>

namespace krys::boo::tests
{
  /// @brief A form submission as the host received it.
  struct ReceivedSubmission
  {
    FormSubmission::Method Method {FormSubmission::Method::Get};
    std::string URL;
    std::string Target;
    std::string ContentType;
    std::string Boundary;
    std::string Body;
    std::string Submitter;
  };

  /// @brief A host that records the form submissions it's asked to navigate to.
  class FormChromeClient final : public ChromeClient
  {
  public:
    List<ReceivedSubmission> Submissions;

    void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
    {
    }

    void SubmitForm(LocalFrame &, FormSubmission &submission) noexcept final
    {
      auto narrow = [](utf8_stringview text)
      {
        return std::string(reinterpret_cast<const char *>(text.data()), text.size());
      };
      ReceivedSubmission received;
      received.Method = submission.GetMethod();
      received.URL = narrow(submission.RequestURL().String());
      received.Target = narrow(submission.Target());
      received.ContentType = narrow(submission.ContentType());
      received.Boundary = narrow(submission.Boundary());
      if (auto *data = submission.Data(); data && submission.GetMethod() == FormSubmission::Method::Post)
      {
        received.Body = narrow(data->FlattenToString());
      }
      if (auto *submitter = submission.Submitter())
      {
        received.Submitter = narrow(submitter->GetAttribute(u8"id").value_or(dom::DOMString()));
      }
      Submissions.push_back(krys::move(received));
    }
  };

  /// @brief A document in a page whose host records form submissions.
  struct FormFixture : style::tests::ResolverFixture
  {
    FormChromeClient *Chrome {nullptr};
    RefPtr<Page> TestPage;

    explicit FormFixture(utf8_stringview markup) noexcept
        : ResolverFixture(u8"<!DOCTYPE html>" + utf8_string(markup), style::tests::WithView::No)
    {
      auto chrome = CreateUnique<FormChromeClient>();
      Chrome = chrome.get();
      TestPage = Page::Create(krys::move(chrome));
      Frame = ShareRefPtr(&TestPage->MainFrame());
      Frame->CreateView({200, 100});
      Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
    }

    template <typename T>
    KRYS_NODISCARD T &Get(dom::DOMStringView id) noexcept
    {
      return Downcast<T>(Downcast<html::HTMLElement>(ById(id)));
    }

    KRYS_NODISCARD const List<ReceivedSubmission> &Submissions() const noexcept
    {
      return Chrome->Submissions;
    }
  };

  /// @brief @p text as a `std::string`, which Catch2 can print.
  KRYS_NODISCARD inline std::string Str(utf8_stringview text) noexcept
  {
    return std::string(reinterpret_cast<const char *>(text.data()), text.size());
  }

  /// @brief Records the types of the events it hears, and cancels them if asked.
  struct EventLog : dom::EventListenerCallback
  {
    List<std::string> Types;
    bool PreventDefault {false};

    void HandleEvent(dom::Event &event) noexcept override
    {
      Types.push_back(Str(event.Type().View()));
      if (PreventDefault)
      {
        event.PreventDefault();
      }
    }

    KRYS_NODISCARD static Ref<EventLog> ListenTo(dom::EventTarget &target,
                                                 std::initializer_list<utf8_stringview> types) noexcept
    {
      auto log = CreateRef<EventLog>();
      for (auto type : types)
      {
        target.AddEventListener(dom::DOMStringAtom(type), Ref<dom::EventListenerCallback> {log}, false);
      }
      return log;
    }

    KRYS_NODISCARD std::string Joined() const noexcept
    {
      std::string result;
      for (auto &type : Types)
      {
        result += result.empty() ? type : " " + type;
      }
      return result;
    }
  };
}
