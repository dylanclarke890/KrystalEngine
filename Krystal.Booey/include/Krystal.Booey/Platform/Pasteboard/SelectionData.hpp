#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "Krystal.Net/Url.hpp"

namespace krys::boo
{
  /// @brief What's on a pasteboard, or being dragged, in the forms Booey reads and writes: plain text,
  /// markup, a URL, a URI list (and the files it names), other data by type, and a page's own data.
  // FIXME: Images (setImage, image, hasImage, clearImage), with images on the pasteboard.
  class SelectionData
  {
  private:
    utf8_string _text;
    utf8_string _markup;
    net::Url _url;
    utf8_string _uriList;
    List<utf8_string> _filenames;
    bool _canSmartReplace {false};
    RefPtr<SharedBuffer> _customData;
    HashMap<utf8_string, Ref<SharedBuffer>> _buffers;

  public:
    SelectionData() noexcept = default;

    /// @brief Sets the text, with no-break spaces made spaces.
    void SetText(utf8_stringview text) noexcept;

    KRYS_NODISCARD const utf8_string &Text() const noexcept
    {
      return _text;
    }

    KRYS_NODISCARD bool HasText() const noexcept
    {
      return !_text.empty();
    }

    void ClearText() noexcept
    {
      _text.clear();
    }

    void SetMarkup(utf8_stringview markup) noexcept
    {
      _markup = utf8_string(markup);
    }

    KRYS_NODISCARD const utf8_string &Markup() const noexcept
    {
      return _markup;
    }

    KRYS_NODISCARD bool HasMarkup() const noexcept
    {
      return !_markup.empty();
    }

    void ClearMarkup() noexcept
    {
      _markup.clear();
    }

    /// @brief Sets the URL, and the URI list, text and markup (a link labelled @p label) if they're empty.
    void SetUrl(const net::Url &url, utf8_stringview label) noexcept;

    KRYS_NODISCARD const net::Url &GetUrl() const noexcept
    {
      return _url;
    }

    /// @brief What to show for the URL: the text if there is any, otherwise the URL itself.
    KRYS_NODISCARD const utf8_string &UrlLabel() const noexcept;

    KRYS_NODISCARD bool HasUrl() const noexcept
    {
      return !_url.IsEmpty() && _url.IsValid();
    }

    void ClearUrl() noexcept
    {
      _url = {};
    }

    /// @brief Sets the URI list, the URL to its first valid URL if there's none, and the file names to the
    /// paths of its file URLs.
    void SetURIList(utf8_stringview uriList) noexcept;

    KRYS_NODISCARD const utf8_string &URIList() const noexcept
    {
      return _uriList;
    }

    KRYS_NODISCARD const List<utf8_string> &Filenames() const noexcept
    {
      return _filenames;
    }

    KRYS_NODISCARD bool HasURIList() const noexcept
    {
      return !_uriList.empty();
    }

    KRYS_NODISCARD bool HasFilenames() const noexcept
    {
      return !_filenames.empty();
    }

    void ClearURIList() noexcept
    {
      _uriList.clear();
    }

    /// @brief Whether pasting it may adjust the spaces around it, as when it was copied from a word
    /// selection.
    void SetCanSmartReplace(bool canSmartReplace) noexcept
    {
      _canSmartReplace = canSmartReplace;
    }

    KRYS_NODISCARD bool CanSmartReplace() const noexcept
    {
      return _canSmartReplace;
    }

    /// @brief Adds data of MIME @p type, unless there's already some of that type.
    void AddBuffer(utf8_stringview type, const Ref<SharedBuffer> &buffer) noexcept
    {
      _buffers.try_emplace(utf8_string(type), buffer);
    }

    KRYS_NODISCARD const HashMap<utf8_string, Ref<SharedBuffer>> &Buffers() const noexcept
    {
      return _buffers;
    }

    KRYS_NODISCARD SharedBuffer *Buffer(utf8_stringview type) const noexcept
    {
      auto it = _buffers.find(utf8_string(type));
      return it == _buffers.end() ? nullptr : it->second.get();
    }

    void ClearBuffers() noexcept
    {
      _buffers.clear();
    }

    /// @brief Sets the data a page wrote in types of its own (PasteboardCustomData), serialised as one
    /// buffer.
    void SetCustomData(Ref<SharedBuffer> &&buffer) noexcept
    {
      _customData = krys::move(buffer);
    }

    KRYS_NODISCARD const RefPtr<SharedBuffer> &CustomData() const noexcept
    {
      return _customData;
    }

    KRYS_NODISCARD bool HasCustomData() const noexcept
    {
      return static_cast<bool>(_customData);
    }

    void ClearCustomData() noexcept
    {
      _customData = nullptr;
    }

    void ClearAll() noexcept;

    /// @brief Clears everything but the file names, and turns smart replace off.
    void ClearAllExceptFilenames() noexcept;
  };
}
