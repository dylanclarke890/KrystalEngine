#pragma once

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DOMRectReadOnly.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/Page/IntersectionObserver.hpp"
#include "Krystal.Booey/Page/IntersectionObserverEntry.hpp"
#include "Krystal.Booey/Page/ResizeObserver.hpp"
#include "Krystal.Booey/Page/ResizeObserverEntry.hpp"
#include "Krystal.Booey/Page/ResizeObserverSize.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <cmath>
#include <format>

namespace krys::editor
{
  /// @brief The editor page's observers, made from C++: a resize observer that shows the size of
  /// `#resize-card` in `#resize-size`, a count of the lazily loaded images of `#lazy-list` that have loaded
  /// in
  /// `#lazy-count`, and an intersection observer that finds the chapter (`.chapter`) most in view, which the
  /// editor's header shows. A page without them is left alone.
  class PageObservers
  {
    KRYS_NON_COPY_MOVABLE(PageObservers);

    /// @brief A chapter and how much of it is in view, in CSS pixels.
    struct ChapterInView
    {
      WeakPtr<boo::dom::Element> Chapter;
      double VisibleHeight {0};
    };

  private:
    WeakPtr<boo::dom::Document> _document;
    RefPtr<boo::ResizeObserver> _resizeObserver;
    RefPtr<boo::IntersectionObserver> _chapterObserver;
    boo::dom::EventListenerHandle _lazyLoads;
    List<ChapterInView> _chapters;
    boo::dom::DOMString _currentChapter;
    size_t _imagesLoaded {0};

  public:
    explicit PageObservers(boo::dom::Document &document) noexcept
        : _document(WeakPtr<boo::dom::Document>::WithRef(document))
    {
      if (auto card = document.GetElementById(u8"resize-card"))
      {
        _resizeObserver =
          boo::ResizeObserver::Create(document, [this](const List<Ref<boo::ResizeObserverEntry>> &entries,
                                                       boo::ResizeObserver &) { ShowSize(*entries.back()); });
        _resizeObserver->Observe(*card);
      }

      if (auto list = document.GetElementById(u8"lazy-list"))
      {
        // `load` doesn't bubble, so this listens as it's captured.
        _lazyLoads = list->AddEventListener(
          u8"load",
          [this](boo::dom::Event &)
          {
            ++_imagesLoaded;
            ShowImagesLoaded();
          },
          true);
      }

      auto chapters = document.QuerySelectorAll(u8".chapter");
      if (chapters.HasException() || !chapters.Value()->Length())
      {
        return;
      }
      // Each tenth of a chapter coming into view or leaving it is reported, so the one most in view is kept
      // as the page scrolls.
      List<double> thresholds;
      for (int step = 0; step <= 10; ++step)
      {
        thresholds.push_back(step / 10.0);
      }
      auto observer = boo::IntersectionObserver::Create(
        document,
        [this](const List<Ref<boo::IntersectionObserverEntry>> &entries, boo::IntersectionObserver &)
        { UpdateChapters(entries); }, {.Threshold = krys::move(thresholds)});
      if (observer.HasException())
      {
        return;
      }
      _chapterObserver = observer.ReleaseValue();
      for (RefPtr<boo::dom::Node> node : *chapters.Value())
      {
        if (auto *chapter = DynamicDowncast<boo::dom::Element>(node.get()))
        {
          _chapters.push_back({CreateWeakPtr(chapter), 0});
          _chapterObserver->Observe(*chapter);
        }
      }
    }

    /// @brief The heading of the chapter most in view; empty if none is.
    KRYS_NODISCARD const boo::dom::DOMString &CurrentChapter() const noexcept
    {
      return _currentChapter;
    }

    /// @brief How many of the lazy list's images have loaded.
    KRYS_NODISCARD size_t ImagesLoaded() const noexcept
    {
      return _imagesLoaded;
    }

  private:
    void ShowSize(boo::ResizeObserverEntry &entry) noexcept
    {
      auto label = _document ? _document->GetElementById(u8"resize-size") : nullptr;
      if (!label)
      {
        return;
      }
      auto &size = *entry.ContentBoxSize().front();
      auto text =
        std::format("{} × {} pixels", std::lround(size.InlineSize()), std::lround(size.BlockSize()));
      (void)label->TextContent(
        boo::dom::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
    }

    void ShowImagesLoaded() noexcept
    {
      auto list = _document ? _document->GetElementById(u8"lazy-list") : nullptr;
      auto label = _document ? _document->GetElementById(u8"lazy-count") : nullptr;
      if (!list || !label)
      {
        return;
      }
      auto images = list->QuerySelectorAll(u8"img");
      auto total = images.HasException() ? 0uz : images.Value()->Length();
      auto text = std::format("{} of {} images loaded", _imagesLoaded, total);
      (void)label->TextContent(
        boo::dom::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
    }

    void UpdateChapters(const List<Ref<boo::IntersectionObserverEntry>> &entries) noexcept
    {
      for (auto &entry : entries)
      {
        for (auto &chapter : _chapters)
        {
          if (chapter.Chapter.get() == entry->Target())
          {
            chapter.VisibleHeight = entry->IsIntersecting() ? entry->IntersectionRect()->Height() : 0;
          }
        }
      }

      const ChapterInView *mostInView = nullptr;
      for (auto &chapter : _chapters)
      {
        if (chapter.Chapter && chapter.VisibleHeight > 0
            && (!mostInView || chapter.VisibleHeight > mostInView->VisibleHeight))
        {
          mostInView = &chapter;
        }
      }
      _currentChapter = {};
      if (mostInView)
      {
        if (auto heading = mostInView->Chapter->QuerySelector(u8"h2");
            !heading.HasException() && heading.Value())
        {
          _currentChapter = heading.Value()->TextContent().value_or(boo::dom::DOMString {});
        }
      }
    }
  };
}
