#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include <string>

namespace krys::boo::tests
{
  /// @brief A form submission as the host received it.
  struct ReceivedSubmission
  {
    FormSubmission::Method Method {FormSubmission::Method::Get};
    std::string URL;
    std::string Target;
    std::string EncodingType;
    /// @brief The entries, unencoded, as `name=value` joined by `&` (a file by its name).
    std::string Entries;
    std::string Submitter;
  };

  /// @brief @p submission as the host received it.
  KRYS_NODISCARD inline ReceivedSubmission ToReceivedSubmission(const FormSubmission &submission) noexcept
  {
    auto narrow = [](utf8_stringview text)
    {
      return std::string(reinterpret_cast<const char *>(text.data()), text.size());
    };
    ReceivedSubmission received;
    received.Method = submission.GetMethod();
    received.URL = narrow(submission.Action().String());
    received.Target = narrow(submission.Target());
    received.EncodingType = narrow(submission.EncodingType());
    if (auto *entries = submission.EntryList())
    {
      for (auto &item : entries->Items())
      {
        auto *value = std::get_if<dom::DOMString>(&item.Data);
        auto *file = std::get_if<RefPtr<File>>(&item.Data);
        received.Entries += (received.Entries.empty() ? "" : "&") + narrow(item.Name) + "="
                            + (value   ? narrow(*value)
                               : *file ? narrow((*file)->Name())
                                       : std::string());
      }
    }
    if (auto *submitter = submission.Submitter())
    {
      received.Submitter = narrow(submitter->GetAttribute(u8"id").value_or(dom::DOMString()));
    }
    return received;
  }

  /// @brief A document in a page whose host records form submissions.
  struct FormFixture : style::tests::ResolverFixture
  {
    RecordingFrameLoaderClient *LoaderClient {nullptr};
    RefPtr<Page> TestPage;
    List<ReceivedSubmission> SubmissionList;

    explicit FormFixture(utf8_stringview markup) noexcept
        : ResolverFixture(u8"<!DOCTYPE html>" + utf8_string(markup), style::tests::WithView::No)
    {
      auto loaderClient = CreateUnique<RecordingFrameLoaderClient>();
      loaderClient->OnRecord = [this](const ReceivedNavigation &navigation)
      {
        if (navigation.Submission)
        {
          SubmissionList.push_back(ToReceivedSubmission(*navigation.Submission));
        }
      };
      LoaderClient = loaderClient.get();
      TestPage = Page::Create(nullptr, nullptr, krys::move(loaderClient));
      Frame = ShareRefPtr(&TestPage->MainFrame());
      Frame->CreateView({200, 100});
      Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
    }

    template <typename T>
    KRYS_NODISCARD T &Get(dom::DOMStringView id) noexcept
    {
      return Downcast<T>(Downcast<html::HTMLElement>(ById(id)));
    }

    /// @brief The submissions the host was asked about, after running the tasks that start them (they're
    /// scheduled).
    KRYS_NODISCARD const List<ReceivedSubmission> &Submissions() const noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
      return SubmissionList;
    }
  };

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
        target.AddEventListener(dom::DOMStringAtom(type), log, false);
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
