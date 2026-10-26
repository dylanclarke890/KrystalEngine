#pragma once

#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <utility>

namespace krys::boo::dom
{
  class DocumentFragment;
}

namespace krys::boo
{
  class LocalFrame;
  enum class MSOListQuirks : bool;

  /// @brief Reads a pasteboard's content for a frame's document.
  class FrameWebContentReader : public PasteboardWebContentReader
  {
  private:
    WeakPtr<LocalFrame> _frame;

  public:
    explicit FrameWebContentReader(LocalFrame &frame) noexcept;

    KRYS_NODISCARD LocalFrame &Frame() const noexcept
    {
      return *_frame;
    }

  protected:
    /// @brief Whether the content came from another origin than the frame's document, so its markup must be
    /// sanitised.
    KRYS_NODISCARD bool ShouldSanitize() const noexcept;
    KRYS_NODISCARD MSOListQuirks MSOListQuirksForMarkup() const noexcept;
  };

  /// @brief Reads a pasteboard's richest content as a fragment to paste at a range.
  // FIXME: Images (readImage), with blob URLs.
  class WebContentReader final : public FrameWebContentReader
  {
  private:
    const dom::SimpleRange _context;
    const bool _allowPlainText;

    RefPtr<dom::DocumentFragment> _fragment;
    bool _madeFragmentFromPlainText {false};

  public:
    WebContentReader(LocalFrame &frame, const dom::SimpleRange &context, bool allowPlainText) noexcept;
    ~WebContentReader() noexcept;

    void AddFragment(Ref<dom::DocumentFragment> &&fragment) noexcept;
    KRYS_NODISCARD RefPtr<dom::DocumentFragment> TakeFragment() noexcept;
    KRYS_NODISCARD dom::DocumentFragment *Fragment() const noexcept
    {
      return _fragment.get();
    }

    KRYS_NODISCARD bool MadeFragmentFromPlainText() const noexcept
    {
      return _madeFragmentFromPlainText;
    }

  private:
    bool ReadFilePath(utf8_stringview path, utf8_stringview contentType = {}) noexcept override;
    bool ReadFilePaths(const List<utf8_string> &paths) noexcept override;
    bool ReadHTML(utf8_stringview markup) noexcept override;
    bool ReadURL(const url::URL &url, utf8_stringview title) noexcept override;
    bool ReadPlainText(utf8_stringview text) noexcept override;
  };

  /// @brief Reads a pasteboard's markup, sanitised if it came from another origin.
  class WebContentMarkupReader final : public FrameWebContentReader
  {
  private:
    utf8_string _markup;

  public:
    explicit WebContentMarkupReader(LocalFrame &frame) noexcept : FrameWebContentReader(frame)
    {
    }

    KRYS_NODISCARD utf8_string TakeMarkup() noexcept
    {
      return std::exchange(_markup, {});
    }

  private:
    bool ReadFilePath(utf8_stringview, utf8_stringview = {}) noexcept override
    {
      return false;
    }
    bool ReadFilePaths(const List<utf8_string> &) noexcept override
    {
      return false;
    }
    bool ReadHTML(utf8_stringview markup) noexcept override;
    bool ReadURL(const url::URL &, utf8_stringview) noexcept override
    {
      return false;
    }
    bool ReadPlainText(utf8_stringview) noexcept override
    {
      return false;
    }
  };
}
