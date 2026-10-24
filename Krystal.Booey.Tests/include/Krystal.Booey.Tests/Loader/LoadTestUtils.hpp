#pragma once

#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Loader/TestLoader.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/Loader/SubstituteData.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/Network/ResourceError.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <string>

namespace krys::boo::tests
{
  KRYS_NODISCARD inline std::string Narrow(utf8_stringview text) noexcept
  {
    return std::string(reinterpret_cast<const char *>(text.data()), text.size());
  }

  /// @brief Records what the loader tells the host, and allows every navigation.
  class LoadRecordingClient final : public LocalFrameLoaderClient
  {
  public:
    List<std::string> &Events;
    PolicyAction Policy {PolicyAction::Use};
    /// @brief Whether pages may enter the back/forward cache.
    bool CachePages {true};

    explicit LoadRecordingClient(List<std::string> &events) noexcept : Events(events)
    {
    }

    KRYS_NODISCARD PolicyAction DispatchDecidePolicyForNavigationAction(
      const NavigationAction &action, const ResourceRequest &, const FormSubmission *) noexcept override
    {
      Events.push_back(action.IsRequestFromClientOrUserInput() ? "policy (host)" : "policy");
      return Policy;
    }

    KRYS_NODISCARD PolicyAction DispatchDecidePolicyForNewWindowAction(const NavigationAction &,
                                                                       const ResourceRequest &,
                                                                       const FormSubmission *,
                                                                       dom::DOMStringView) noexcept override
    {
      return PolicyAction::Ignore;
    }

    KRYS_NODISCARD bool CanCachePage() const noexcept override
    {
      return CachePages;
    }

    void DispatchDidStartProvisionalLoad() noexcept override
    {
      Events.push_back("start");
    }

    void DispatchDidFailProvisionalLoad(const ResourceError &error) noexcept override
    {
      Events.push_back(error.IsCancellation() ? "fail provisional (cancelled)" : "fail provisional");
    }

    void DispatchDidCommitLoad() noexcept override
    {
      Events.push_back("commit");
    }

    /// @brief Called with each new document, before it's parsed.
    Func<void()> OnClearWindowObject;

    void DispatchDidClearWindowObject() noexcept override
    {
      Events.push_back("window");
      if (OnClearWindowObject)
      {
        OnClearWindowObject();
      }
    }

    void DispatchDidFinishDocumentLoad() noexcept override
    {
      Events.push_back("document loaded");
    }

    void DispatchDidFinishLoad() noexcept override
    {
      Events.push_back("finish");
    }
  };

  /// @brief Answers whether to leave a page, and records the alerts shown.
  struct LeavingChromeClient final : ChromeClient
  {
    bool Leave {true};
    uint32 ConfirmPanels {0};
    List<std::string> Alerts;

    void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept override
    {
    }

    KRYS_NODISCARD bool RunBeforeUnloadConfirmPanel(const dom::DOMString &, LocalFrame &) noexcept override
    {
      ++ConfirmPanels;
      return Leave;
    }

    void RunJavaScriptAlert(LocalFrame &, const dom::DOMString &message) noexcept override
    {
      Alerts.push_back(Narrow(message));
    }
  };

  /// @brief A page whose main frame starts with its initial empty document, and whose loads come from a
  /// test strategy.
  struct LoadFixture : css::tests::ValueFixture, LoaderFixture
  {
    List<std::string> Events;
    LoadRecordingClient *Client {nullptr};
    LeavingChromeClient *Chrome {nullptr};
    Ref<Page> TestPage {CreatePage()};

    LoadFixture() noexcept
    {
      Frame().CreateView({200, 100});
      Frame().Loader().Init();
      Events.clear();
    }

    KRYS_NODISCARD Ref<Page> CreatePage() noexcept
    {
      auto client = CreateUnique<LoadRecordingClient>(Events);
      Client = client.get();
      auto chrome = CreateUnique<LeavingChromeClient>();
      Chrome = chrome.get();
      return Page::Create(krys::move(chrome), nullptr, krys::move(client));
    }

    KRYS_NODISCARD LocalFrame &Frame() const noexcept
    {
      return TestPage->MainFrame();
    }

    KRYS_NODISCARD dom::Document &Document() const noexcept
    {
      return *Frame().Document();
    }

    /// @brief Loads @p markup at @p url, as the host does.
    void LoadMarkup(utf8_string &&markup, utf8_stringview url) noexcept
    {
      FrameLoadRequest request(ShareRef(Document()), ResourceRequest(url::URL(url)),
                               dom::DOMStringAtom::Null());
      request.SetSubstituteData(SubstituteData(krys::move(markup), u8"text/html", url::URL(url)));
      Frame().Loader().Load(krys::move(request));
    }

    /// @brief Loads @p url through the strategy, as the host does.
    void LoadURL(utf8_stringview url) noexcept
    {
      Frame().Loader().Load(
        FrameLoadRequest(ShareRef(Document()), ResourceRequest(url::URL(url)), dom::DOMStringAtom::Null()));
    }
  };
}
