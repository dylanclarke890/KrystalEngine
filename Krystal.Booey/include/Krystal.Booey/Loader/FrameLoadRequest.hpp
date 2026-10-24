#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/FetchOptions.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Booey/Loader/SubstituteData.hpp"
#include "Krystal.Booey/Page/WindowFeatures.hpp"
#include "Krystal.Booey/Platform/Network/ResourceRequest.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo
{
  /// @brief What a request to load a frame and the navigation action it becomes have in common.
  class FrameLoadRequestBase
  {
  private:
    NewFrameOpenerPolicy _newFrameOpenerPolicy {NewFrameOpenerPolicy::Allow};
    dom::DOMStringAtom _downloadAttribute {dom::DOMStringAtom::Null()};
    RefPtr<dom::Element> _sourceElement;
    Maybe<WindowFeatures> _windowFeatures;
    bool _isRequestFromClientOrUserInput {false};
    bool _isFromNavigationAPI {false};

  public:
    /// @brief Whether the Navigation API started the navigation (`navigation.navigate()`, `reload()`).
    KRYS_NODISCARD bool IsFromNavigationAPI() const noexcept
    {
      return _isFromNavigationAPI;
    }
    void SetIsFromNavigationAPI(bool isFromNavigationAPI) noexcept
    {
      _isFromNavigationAPI = isFromNavigationAPI;
    }

    KRYS_NODISCARD NewFrameOpenerPolicy GetNewFrameOpenerPolicy() const noexcept
    {
      return _newFrameOpenerPolicy;
    }
    void SetNewFrameOpenerPolicy(NewFrameOpenerPolicy policy) noexcept
    {
      _newFrameOpenerPolicy = policy;
    }

    /// @brief The `download` attribute of the link followed; null if it has none.
    KRYS_NODISCARD dom::DOMStringAtom DownloadAttribute() const noexcept
    {
      return _downloadAttribute;
    }

    /// @brief The element that started the navigation (a link or a form), if any.
    KRYS_NODISCARD dom::Element *SourceElement() const noexcept
    {
      return _sourceElement.get();
    }
    void SetSourceElement(dom::Element *sourceElement) noexcept;

    /// @brief The features `window.open()` asked a new window to have, if it opened one.
    KRYS_NODISCARD const Maybe<WindowFeatures> &GetWindowFeatures() const noexcept
    {
      return _windowFeatures;
    }
    void SetWindowFeatures(Maybe<WindowFeatures> &&features) noexcept
    {
      _windowFeatures = krys::move(features);
    }

    /// @brief Whether the host or the user asked for the navigation, rather than the page.
    KRYS_NODISCARD bool IsRequestFromClientOrUserInput() const noexcept
    {
      return _isRequestFromClientOrUserInput;
    }
    void SetIsRequestFromClientOrUserInput(bool value) noexcept
    {
      _isRequestFromClientOrUserInput = value;
    }

  protected:
    FrameLoadRequestBase() noexcept;
    FrameLoadRequestBase(const FrameLoadRequestBase &) noexcept;
    FrameLoadRequestBase &operator=(const FrameLoadRequestBase &) noexcept;
    FrameLoadRequestBase(FrameLoadRequestBase &&) noexcept;
    FrameLoadRequestBase &operator=(FrameLoadRequestBase &&) noexcept;
    ~FrameLoadRequestBase() noexcept;

    void SetDownloadAttribute(dom::DOMStringAtom downloadAttribute) noexcept
    {
      _downloadAttribute = downloadAttribute;
    }
  };

  /// @brief A request to load a URL into a frame: what's loaded, from which document, into which browsing
  /// context.
  // FIXME: History locking, client redirects and the security origin, once there's history.
  class FrameLoadRequest : public FrameLoadRequestBase
  {
  private:
    Ref<dom::Document> _requester;
    ResourceRequest _resourceRequest;
    dom::DOMStringAtom _frameName;
    ReferrerPolicy _referrerPolicy {ReferrerPolicy::EmptyString};
    NavigationHistoryBehavior _navigationHistoryBehavior {NavigationHistoryBehavior::Auto};
    Maybe<SubstituteData> _substituteData;

  public:
    /// @param requester The document asking for the load.
    /// @param frameName The browsing context to load into (`_self`, `_blank` and the others, or a name).
    /// @param downloadAttribute The `download` attribute of the link followed; null if it has none.
    FrameLoadRequest(Ref<dom::Document> &&requester, ResourceRequest &&resourceRequest,
                     dom::DOMStringAtom frameName,
                     dom::DOMStringAtom downloadAttribute = dom::DOMStringAtom::Null()) noexcept;
    ~FrameLoadRequest() noexcept;
    FrameLoadRequest(const FrameLoadRequest &) noexcept;
    FrameLoadRequest &operator=(const FrameLoadRequest &) noexcept;
    FrameLoadRequest(FrameLoadRequest &&) noexcept;
    FrameLoadRequest &operator=(FrameLoadRequest &&) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _resourceRequest.IsNull();
    }

    KRYS_NODISCARD dom::Document &Requester() const noexcept
    {
      return *_requester;
    }

    KRYS_NODISCARD ResourceRequest &GetResourceRequest() noexcept
    {
      return _resourceRequest;
    }
    KRYS_NODISCARD const ResourceRequest &GetResourceRequest() const noexcept
    {
      return _resourceRequest;
    }

    KRYS_NODISCARD dom::DOMStringAtom FrameName() const noexcept
    {
      return _frameName;
    }

    /// @brief The document's content, which the host gives in place of loading the request's URL.
    KRYS_NODISCARD const Maybe<SubstituteData> &GetSubstituteData() const noexcept
    {
      return _substituteData;
    }
    void SetSubstituteData(SubstituteData &&substituteData) noexcept
    {
      _substituteData = krys::move(substituteData);
    }
    void SetFrameName(dom::DOMStringAtom frameName) noexcept
    {
      _frameName = frameName;
    }

    KRYS_NODISCARD ReferrerPolicy GetReferrerPolicy() const noexcept
    {
      return _referrerPolicy;
    }
    void SetReferrerPolicy(ReferrerPolicy referrerPolicy) noexcept
    {
      _referrerPolicy = referrerPolicy;
    }

    KRYS_NODISCARD NavigationHistoryBehavior GetNavigationHistoryBehavior() const noexcept
    {
      return _navigationHistoryBehavior;
    }
    void SetNavigationHistoryBehavior(NavigationHistoryBehavior historyHandling) noexcept
    {
      _navigationHistoryBehavior = historyHandling;
    }
  };
}
