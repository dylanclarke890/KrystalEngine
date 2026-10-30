#pragma once

#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief The contents of a pasteboard: text, markup, a URL and a URI list (with the files it names),
  /// buffers by type and a page's custom data.
  // FIXME: Images (setImage, image, hasImage, clearImage), with images on the pasteboard.
  class SelectionData
  {
  private:
    utf8_string _text;
    utf8_string _markup;
    url::URL _url;
    utf8_string _uriList;
    List<utf8_string> _filenames;
    bool _canSmartReplace {false};
    RefPtr<SharedBuffer> _customData;
    HashMap<utf8_string, Ref<SharedBuffer>> _buffers;

  public:
    SelectionData() noexcept = default;
    SelectionData(utf8_stringview text, utf8_stringview markup, const url::URL &url, utf8_stringview uriList,
                  RefPtr<SharedBuffer> &&customData, bool canSmartReplace) noexcept;

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
    void SetURL(const url::URL &url, utf8_stringview label) noexcept;
    KRYS_NODISCARD const url::URL &GetURL() const noexcept
    {
      return _url;
    }
    KRYS_NODISCARD const utf8_string &URLLabel() const noexcept;
    KRYS_NODISCARD bool HasURL() const noexcept
    {
      return !_url.IsEmpty() && _url.IsValid();
    }
    void ClearURL() noexcept
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

    void SetCanSmartReplace(bool canSmartReplace) noexcept
    {
      _canSmartReplace = canSmartReplace;
    }
    KRYS_NODISCARD bool CanSmartReplace() const noexcept
    {
      return _canSmartReplace;
    }

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
    void ClearAllExceptFilenames() noexcept;
  };
}
