#pragma once

#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
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

  /// @brief A navigation as the host was asked about it.
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

  /// @brief A frame loader client that records the navigations it's asked about and answers each with
  /// `Policy`.
  class RecordingFrameLoaderClient final : public LocalFrameLoaderClient
  {
  public:
    List<ReceivedNavigation> Navigations;
    PolicyAction Policy {PolicyAction::Use};
    uint32 DidNavigateWithinPageCount {0};
    uint32 DidChangeLocationWithinPageCount {0};
    /// @brief Called with each navigation as it's recorded.
    Func<void(const ReceivedNavigation &)> OnRecord;

    KRYS_NODISCARD PolicyAction
      DispatchDecidePolicyForNavigationAction(const NavigationAction &action, const ResourceRequest &request,
                                              const FormSubmission *formSubmission) noexcept final
    {
      Record(action, request, formSubmission, false, {});
      return Policy;
    }

    KRYS_NODISCARD PolicyAction DispatchDecidePolicyForNewWindowAction(
      const NavigationAction &action, const ResourceRequest &request, const FormSubmission *formSubmission,
      dom::DOMStringView frameName) noexcept final
    {
      Record(action, request, formSubmission, true, frameName);
      return Policy;
    }

    void DispatchDidNavigateWithinPage() noexcept final
    {
      ++DidNavigateWithinPageCount;
    }

    void DispatchDidChangeLocationWithinPage() noexcept final
    {
      ++DidChangeLocationWithinPageCount;
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

  /// @brief A document at `https://example.com/dir/page.html`, in a page whose host records navigations and
  /// answers them with its `Policy` (`Use` to begin with).
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

    KRYS_NODISCARD const List<ReceivedNavigation> &Navigations() const noexcept
    {
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
