#pragma once

#include "Krystal.Booey/Loader/DocumentWriter.hpp"
#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/Loader/SubstituteData.hpp"
#include "Krystal.Booey/Platform/Network/ResourceError.hpp"
#include "Krystal.Booey/Platform/Network/ResourceRequest.hpp"
#include "Krystal.Booey/Platform/Network/ResourceResponse.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LocalFrame;
  class SharedBuffer;

  /// @brief Loads one document into a frame: its main resource (from the host's loader strategy, a `data:`
  /// URL, or the host's substitute data), which, once it arrives, commits the load and is written into the
  /// frame as its document.
  /// @note The main resource arrives whole, on a networking task after the load starts.
  // FIXME: Redirects, responses that aren't documents (downloads), archives and the application cache,
  // streamed data, and the subresource loaders a document loader owns, once there are those.
  class DocumentLoader final : public RefCounted<DocumentLoader>, public CanMakeWeakPtr<DocumentLoader>
  {
    KRYS_NON_COPY_MOVABLE(DocumentLoader);

  private:
    WeakPtr<LocalFrame> _frame;
    ResourceRequest _request;
    Maybe<SubstituteData> _substituteData;
    NavigationAction _triggeringAction;
    ResourceResponse _response;
    ResourceError _mainDocumentError;
    DocumentWriter _writer;
    bool _isLoadingMainResource {false};
    bool _committed {false};
    bool _isStopping {false};
    bool _gotFirstByte {false};
    bool _isClientRedirect {false};

    DocumentLoader(ResourceRequest &&request, Maybe<SubstituteData> &&substituteData) noexcept;

  public:
    ~DocumentLoader() noexcept;

    KRYS_NODISCARD static Ref<DocumentLoader> Create(ResourceRequest &&request,
                                                     Maybe<SubstituteData> &&substituteData = null) noexcept;

    /// @brief Attaches the loader to the frame it loads into.
    void AttachToFrame(LocalFrame &frame) noexcept;
    void DetachFromFrame() noexcept;

    KRYS_NODISCARD LocalFrame *Frame() const noexcept
    {
      return _frame.get();
    }

    KRYS_NODISCARD const ResourceRequest &Request() const noexcept
    {
      return _request;
    }

    /// @brief The document's URL: the substitute data's, the response's, or the request's.
    KRYS_NODISCARD const url::URL &DocumentURL() const noexcept;

    /// @brief The URL the document's history item has: its URL, also for the host's markup, which the item
    /// keeps.
    KRYS_NODISCARD const url::URL &URLForHistory() const noexcept
    {
      return DocumentURL();
    }

    /// @brief Whether the load replaces the document that started it in the session history (a redirect, or a
    /// location change before it loaded).
    KRYS_NODISCARD bool IsClientRedirect() const noexcept
    {
      return _isClientRedirect;
    }
    void SetIsClientRedirect(bool isClientRedirect) noexcept
    {
      _isClientRedirect = isClientRedirect;
    }

    KRYS_NODISCARD const Maybe<SubstituteData> &GetSubstituteData() const noexcept
    {
      return _substituteData;
    }

    KRYS_NODISCARD const ResourceResponse &Response() const noexcept
    {
      return _response;
    }

    KRYS_NODISCARD const ResourceError &MainDocumentError() const noexcept
    {
      return _mainDocumentError;
    }

    KRYS_NODISCARD const NavigationAction &TriggeringAction() const noexcept
    {
      return _triggeringAction;
    }
    void SetTriggeringAction(NavigationAction &&action) noexcept
    {
      _triggeringAction = krys::move(action);
    }

    KRYS_NODISCARD DocumentWriter &Writer() noexcept
    {
      return _writer;
    }

    /// @brief Whether the main resource is still to arrive.
    KRYS_NODISCARD bool IsLoadingMainResource() const noexcept
    {
      return _isLoadingMainResource;
    }

    /// @brief Whether the loader has a document in the frame.
    KRYS_NODISCARD bool IsCommitted() const noexcept
    {
      return _committed;
    }

    /// @brief Starts loading the main resource.
    void StartLoadingMainResource() noexcept;

    /// @brief Stops loading the main resource: its data, if it arrives later, is dropped.
    void StopLoading() noexcept;

    /// @brief Makes the initial empty document (`about:blank`) the frame's at once, without a load.
    void CommitInitialEmptyDocument() noexcept;

  private:
    /// @brief The main resource arrived: commits the load and writes the document.
    void ContinueAfterMainResourceLoad(ResourceResponse &&response, Span<const uint8> data) noexcept;

    /// @brief The main resource couldn't be loaded.
    void MainReceivedError(ResourceError &&error) noexcept;

    /// @brief Commits the load, making the frame show the new document.
    void CommitLoad(Span<const uint8> data) noexcept;
  };
}
