#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Web/Css/Geometry/DOMRectReadOnly.hpp"
#include "Krystal.Web/Css/ResizeObserver/ResizeObserver.hpp"
#include "Krystal.Web/Css/ResizeObserver/ResizeObserverEntry.hpp"
#include "Krystal.Web/Css/ResizeObserver/ResizeObserverSize.hpp"
#include "Krystal.Web/Dom/Collections/NodeList.hpp"
#include "Krystal.Web/Dom/Events/EventListenerHandle.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/IntersectionObserver/IntersectionObserver.hpp"
#include "Krystal.Web/IntersectionObserver/IntersectionObserverEntry.hpp"
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
      WeakPtr<web::Element> Chapter;
      double VisibleHeight {0};
    };

  private:
    WeakPtr<web::Document> _document;
    RefPtr<web::ResizeObserver> _resizeObserver;
    RefPtr<web::IntersectionObserver> _chapterObserver;
    web::EventListenerHandle _lazyLoads;
    List<ChapterInView> _chapters;
    web::DOMString _currentChapter;
    size_t _imagesLoaded {0};

  public:
    explicit PageObservers(web::Document &document) noexcept : _document(CreateWeakPtr(&document))
    {
      if (auto card = document.GetElementById(u8"resize-card"))
      {
        _resizeObserver =
          web::ResizeObserver::Create(document, [this](const List<Ref<web::ResizeObserverEntry>> &entries,
                                                       web::ResizeObserver &) { ShowSize(*entries.back()); });
        _resizeObserver->Observe(*card);
      }

      if (auto list = document.GetElementById(u8"lazy-list"))
      {
        // `load` doesn't bubble, so this listens as it's captured.
        _lazyLoads = list->AddEventListener(
          u8"load",
          [this](web::Event &)
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
      auto observer = web::IntersectionObserver::Create(
        document,
        [this](const List<Ref<web::IntersectionObserverEntry>> &entries, web::IntersectionObserver &)
        { UpdateChapters(entries); }, {.Threshold = krys::move(thresholds)});
      if (observer.HasException())
      {
        return;
      }
      _chapterObserver = observer.ReleaseValue();
      for (RefPtr<web::Node> node : *chapters.Value())
      {
        if (auto *chapter = DynamicDowncast<web::Element>(node.get()))
        {
          _chapters.push_back({CreateWeakPtr(chapter), 0});
          _chapterObserver->Observe(*chapter);
        }
      }
    }

    /// @brief The heading of the chapter most in view; empty if none is.
    KRYS_NODISCARD const web::DOMString &CurrentChapter() const noexcept
    {
      return _currentChapter;
    }

    /// @brief How many of the lazy list's images have loaded.
    KRYS_NODISCARD size_t ImagesLoaded() const noexcept
    {
      return _imagesLoaded;
    }

  private:
    void ShowSize(web::ResizeObserverEntry &entry) noexcept
    {
      auto label = _document ? _document->GetElementById(u8"resize-size") : nullptr;
      if (!label)
      {
        return;
      }
      auto &size = *entry.ContentBoxSize().front();
      auto text =
        std::format("{} × {} pixels", std::lround(size.InlineSize()), std::lround(size.BlockSize()));
      (void)label->TextContent(web::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
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
      (void)label->TextContent(web::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
    }

    void UpdateChapters(const List<Ref<web::IntersectionObserverEntry>> &entries) noexcept
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
          _currentChapter = heading.Value()->TextContent().value_or(web::DOMString {});
        }
      }
    }
  };
}
