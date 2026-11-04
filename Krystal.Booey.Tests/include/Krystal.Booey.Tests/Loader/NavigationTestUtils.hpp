#pragma once

#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/Network/ResourceRequest.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <string>

namespace krys::boo::tests
{
  /// @brief @p text as a `std::string`, which Catch2 can print.
  KRYS_NODISCARD inline std::string Str(utf8_stringview text) noexcept
  {
    return std::string(reinterpret_cast<const char *>(text.data()), text.size());
  }

  /// @brief A navigation the page asked the host for.
  struct ReceivedNavigation
  {
    NavigationAction Action;
    std::string URL;
    std::string HTTPMethod;
    /// @brief The browsing context named, for a new window; empty otherwise.
    std::string NewWindowName;
    bool IsNewWindow {false};
    RefPtr<FormSubmission> Submission;
  };

  /// @brief A frame loader client that records the navigations the page asks for.
  class RecordingFrameLoaderClient final : public LocalFrameLoaderClient
  {
  public:
    List<ReceivedNavigation> Navigations;
    uint32 DidNavigateWithinPageCount {0};
    /// @brief Called with each navigation as it's recorded.
    Func<void(const ReceivedNavigation &)> OnRecord;

    void DispatchNavigationRequested(const NavigationAction &action, const ResourceRequest &request,
                                     const FormSubmission *formSubmission) noexcept final
    {
      Record(action, request, formSubmission, false, {});
    }

    void DispatchNewWindowRequested(const NavigationAction &action, const ResourceRequest &request,
                                    const FormSubmission *formSubmission,
                                    dom::DOMStringView frameName) noexcept final
    {
      Record(action, request, formSubmission, true, frameName);
    }

    void DispatchDidNavigateWithinPage() noexcept final
    {
      ++DidNavigateWithinPageCount;
    }

  private:
    void Record(const NavigationAction &action, const ResourceRequest &request,
                const FormSubmission *formSubmission, bool isNewWindow, dom::DOMStringView frameName) noexcept
    {
      auto narrow = [](utf8_stringview text)
      {
        return std::string(reinterpret_cast<const char *>(text.data()), text.size());
      };
      ReceivedNavigation received {.Action = action,
                                   .URL = narrow(request.URL().String()),
                                   .HTTPMethod = narrow(request.HTTPMethod()),
                                   .NewWindowName = narrow(frameName),
                                   .IsNewWindow = isNewWindow,
                                   .Submission = ShareRefPtr(const_cast<FormSubmission *>(formSubmission))};
      Navigations.push_back(krys::move(received));
      if (OnRecord)
      {
        OnRecord(Navigations.back());
      }
    }
  };

  /// @brief A document at `https://example.com/dir/page.html`, in a page whose host records the navigations
  /// it asks for.
  struct NavigationFixture : style::tests::ResolverFixture
  {
    RecordingFrameLoaderClient *Client {nullptr};
    RefPtr<Page> TestPage;

    explicit NavigationFixture(utf8_stringview markup) noexcept
        : ResolverFixture(u8"<!DOCTYPE html>" + utf8_string(markup), style::tests::WithView::No)
    {
      auto client = CreateUnique<RecordingFrameLoaderClient>();
      Client = client.get();
      TestPage = Page::Create(nullptr, nullptr, krys::move(client));
      Frame = ShareRefPtr(&TestPage->MainFrame());
      Frame->CreateView({400, 300});
      Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
      Document->SetURL(url::URL(u8"https://example.com/dir/page.html"));
    }

    template <typename T>
    KRYS_NODISCARD T &Get(dom::DOMStringView id) noexcept
    {
      return Downcast<T>(Downcast<html::HTMLElement>(ById(id)));
    }

    /// @brief The navigations the page asked for, after running the tasks that start them (form submissions
    /// are planned for a later task).
    KRYS_NODISCARD const List<ReceivedNavigation> &Navigations() const noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
      return Client->Navigations;
    }

    /// @brief The document's URL, as a `std::string`.
    KRYS_NODISCARD std::string DocumentURL() const noexcept
    {
      auto &url = Document->GetURL().String();
      return std::string(reinterpret_cast<const char *>(url.data()), url.size());
    }
  };
}
