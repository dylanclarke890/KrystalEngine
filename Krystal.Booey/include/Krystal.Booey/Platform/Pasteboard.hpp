#pragma once

#include "Krystal.Booey/Platform/PasteboardCustomData.hpp"
#include "Krystal.Booey/Platform/SelectionData.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class DragData;

  enum class PlainTextURLReadingPolicy : bool
  {
    IgnoreURL,
    AllowURL
  };

  enum class WebContentReadingPolicy : bool
  {
    AnyType,
    OnlyRichTextTypes
  };

  // For writing to the pasteboard. Generally sorted with the richest formats on top.

  struct PasteboardWebContent
  {
    utf8_string ContentOrigin;
    bool CanSmartCopyOrDelete {false};
    utf8_string Text;
    utf8_string Markup;
  };

  struct PasteboardURL
  {
    url::URL Url;
    utf8_string Title;
    utf8_string Markup;
  };

  // FIXME: The image itself (image), with images on the pasteboard.
  struct PasteboardImage
  {
    PasteboardURL Url;
    utf8_string SuggestedName;
  };

  // For reading from the pasteboard.

  /// @brief Reads a pasteboard's richest content it can: each read returns whether it took the content.
  // FIXME: Images (readImage), with blob URLs.
  class PasteboardWebContentReader
  {
  private:
    utf8_string _contentOrigin;

  public:
    virtual ~PasteboardWebContentReader() noexcept = default;

    virtual bool ReadFilePath(utf8_stringview path, utf8_stringview contentType = {}) noexcept = 0;
    virtual bool ReadFilePaths(const List<utf8_string> &paths) noexcept = 0;
    virtual bool ReadHTML(utf8_stringview markup) noexcept = 0;
    virtual bool ReadURL(const url::URL &url, utf8_stringview title) noexcept = 0;
    virtual bool ReadPlainText(utf8_stringview text) noexcept = 0;

    /// @brief The identifier of the origin that wrote the content, or empty if it wasn't a page.
    KRYS_NODISCARD const utf8_string &ContentOrigin() const noexcept
    {
      return _contentOrigin;
    }
    void SetContentOrigin(utf8_stringview contentOrigin) noexcept
    {
      _contentOrigin = utf8_string(contentOrigin);
    }
  };

  struct PasteboardPlainText
  {
    utf8_string Text;
  };

  /// @brief Reads the files on a pasteboard: by path, or as buffers named and typed by the pasteboard.
  struct PasteboardFileReader
  {
    virtual ~PasteboardFileReader() noexcept = default;
    virtual void ReadFilename(utf8_stringview filename) noexcept = 0;
    KRYS_NODISCARD virtual bool ShouldReadBuffer(utf8_stringview /* type */) const noexcept
    {
      return true;
    }
    virtual void ReadBuffer(utf8_stringview filename, utf8_stringview type,
                            Ref<SharedBuffer> &&buffer) noexcept = 0;
  };

  /// @brief A pasteboard: the system clipboard, read and written through the host's pasteboard strategy, or
  /// the data of a drag.
  // FIXME: The drag image (setDragImage), with drag images; writing images, colours and buffers, with images
  // on the pasteboard; the smart paste type, with writing it; reading items by index, with the async
  // clipboard API.
  // FIXME: Trustworthy web URLs (canWriteTrustworthyWebURLsPboardType), which are Cocoa's.
  class Pasteboard
  {
    KRYS_NON_COPY_MOVABLE(Pasteboard);

  private:
    Maybe<SelectionData> _selectionData;
    utf8_string _name;
    int64 _changeCount {0};

  public:
    enum SmartReplaceOption : bool
    {
      CanSmartReplace,
      CannotSmartReplace
    };

    enum class FileContentState : uint8
    {
      NoFileOrImageData,
      InMemoryImage,
      MayContainFilePaths
    };

    Pasteboard() noexcept = default;
    /// @brief The system clipboard named @p name.
    explicit Pasteboard(utf8_stringview name) noexcept;
    /// @brief A pasteboard holding @p selectionData.
    explicit Pasteboard(SelectionData &&selectionData) noexcept;
    explicit Pasteboard(const SelectionData &selectionData) noexcept;
    virtual ~Pasteboard() noexcept = default;

    KRYS_NODISCARD static UniquePtr<Pasteboard> CreateForCopyAndPaste() noexcept;
    /// @brief An empty pasteboard for a drag that starts in a page.
    KRYS_NODISCARD static UniquePtr<Pasteboard> CreateForDragAndDrop() noexcept;
    /// @brief A pasteboard holding the data of @p dragData.
    KRYS_NODISCARD static UniquePtr<Pasteboard> Create(const DragData &dragData) noexcept;

    KRYS_NODISCARD static bool IsSafeTypeForDOMToReadAndWrite(utf8_stringview type) noexcept;
    KRYS_NODISCARD static bool
      CanExposeURLToDOMWhenPasteboardContainsFiles(utf8_stringview urlString) noexcept;

    KRYS_NODISCARD virtual bool IsStatic() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool HasData() noexcept;
    KRYS_NODISCARD virtual List<utf8_string> TypesSafeForBindings(utf8_stringview origin) noexcept;
    KRYS_NODISCARD virtual utf8_string ReadOrigin() noexcept;
    KRYS_NODISCARD virtual utf8_string ReadString(utf8_stringview type) noexcept;
    KRYS_NODISCARD virtual Maybe<utf8_string> ReadStringInCustomData(utf8_stringview type) noexcept;
    KRYS_NODISCARD virtual List<utf8_string> ReadAllStrings(utf8_stringview type) noexcept;

    virtual void WriteString(utf8_stringview type, utf8_stringview data) noexcept;
    virtual void Clear() noexcept;
    virtual void Clear(utf8_stringview type) noexcept;

    virtual void Read(PasteboardPlainText &text,
                      PlainTextURLReadingPolicy policy = PlainTextURLReadingPolicy::AllowURL) noexcept;
    virtual void Read(PasteboardWebContentReader &reader,
                      WebContentReadingPolicy policy = WebContentReadingPolicy::AnyType) noexcept;
    virtual void Read(PasteboardFileReader &reader) noexcept;

    virtual void Write(const PasteboardURL &pasteboardURL) noexcept;
    virtual void Write(const PasteboardImage &pasteboardImage) noexcept;
    virtual void Write(const PasteboardWebContent &pasteboardContent) noexcept;

    virtual void WriteCustomData(const List<PasteboardCustomData> &data) noexcept;

    KRYS_NODISCARD virtual FileContentState GetFileContentState() noexcept;
    /// @brief Whether the pasteboard's contents were copied as whole words, so pasting them adds spaces
    /// around them.
    KRYS_NODISCARD virtual bool CanSmartReplaceContents() noexcept;

    virtual void WriteMarkup(utf8_stringview markup) noexcept;
    virtual void WritePlainText(utf8_stringview text, SmartReplaceOption smartReplaceOption) noexcept;

    KRYS_NODISCARD int64 ChangeCount() const noexcept;
    KRYS_NODISCARD const utf8_string &Name() const noexcept
    {
      return _name;
    }

    /// @brief The data of a pasteboard made from it.
    KRYS_NODISCARD const SelectionData &GetSelectionData() const noexcept
    {
      krys_debug_assert(_selectionData);
      return *_selectionData;
    }

    /// @brief The pasteboard's @p type as bytes, or null.
    KRYS_NODISCARD RefPtr<SharedBuffer> ReadBuffer(utf8_stringview type) noexcept;
  };
}
