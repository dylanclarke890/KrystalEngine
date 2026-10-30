#include "Krystal.Booey/HTML/Algorithms/HyperlinkAlgorithms.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <cmath>
#include <format>

namespace krys::boo::html
{
  namespace
  {
    /// @brief The `?x,y` a link around a server-side image map is followed with: where @p event clicked in
    /// the image.
    void AppendServerMapMousePosition(utf8_string &url, dom::Event &event) noexcept
    {
      auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event);
      if (!mouseEvent)
      {
        return;
      }

      auto *node = DynamicDowncast<dom::Node>(mouseEvent->Target().get());
      auto *imageElement = node ? DynamicDowncast<HTMLImageElement>(*node) : nullptr;
      if (!imageElement || !imageElement->IsServerMap())
      {
        return;
      }

      auto *renderer = DynamicDowncast<RenderImage>(imageElement->Renderer());
      if (!renderer)
      {
        return;
      }

      // FIXME: This should probably pass UseTransforms in the EnumSet<MapCoordinatesMode>.
      auto absolutePosition = renderer->AbsoluteToLocal(
        FloatPoint(static_cast<float>(mouseEvent->PageX()), static_cast<float>(mouseEvent->PageY())));
      auto position =
        std::format("?{},{}", std::lround(absolutePosition.X()), std::lround(absolutePosition.Y()));
      url.append(position.begin(), position.end());
    }

    /// @brief The target @p target, unless it has dangling markup (a newline and a `<`), which makes it
    /// `_blank`.
    /// @see https://html.spec.whatwg.org/#get-an-element's-target
    KRYS_NODISCARD dom::DOMStringAtom MakeTargetBlankIfHasDanglingMarkup(dom::DOMStringAtom target) noexcept
    {
      auto view = target.View();
      if ((view.find(u8'\n') != view.npos || view.find(u8'\r') != view.npos || view.find(u8'\t') != view.npos)
          && view.find(u8'<') != view.npos)
      {
        return dom::DOMStringAtom(u8"_blank");
      }
      return target;
    }
  }

  url::URL HyperlinkAlgorithms::Href(const HTMLElement &element) noexcept
  {
    auto href = element.GetAttribute(u8"href");
    if (!href)
    {
      return {};
    }
    return element.NodeDocument().CompleteURL(*href);
  }

  void HyperlinkAlgorithms::SetHref(HTMLElement &element, const url::URL &url) noexcept
  {
    (void)element.SetAttribute(u8"href", dom::DOMString(url.String()));
  }

  dom::DOMStringAtom HyperlinkAlgorithms::EffectiveTarget(const HTMLElement &element) noexcept
  {
    auto target = element.GetAttribute(u8"target");
    auto effectiveTarget = target && !target->empty()
                             ? dom::DOMStringAtom(*target)
                             : dom::DOMStringAtom(element.NodeDocument().BaseTarget());
    return MakeTargetBlankIfHasDanglingMarkup(effectiveTarget);
  }

  bool HyperlinkAlgorithms::IsEnterKeyKeydownEvent(const dom::Event &event) noexcept
  {
    if (event.Type() != EventNames::KeyDown)
    {
      return false;
    }
    auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event);
    return keyboardEvent && keyboardEvent->KeyIdentifier() == u8"Enter";
  }

  bool HyperlinkAlgorithms::HasRel(const HTMLElement &element, dom::DOMStringView relation) noexcept
  {
    auto rel = element.GetAttribute(u8"rel");
    if (!rel)
    {
      return false;
    }
    dom::DOMStringView remaining = *rel;
    while (!remaining.empty())
    {
      auto start = std::ranges::find_if_not(remaining, [](char8_t c) { return text::IsASCIIWhitespace(c); });
      remaining = remaining.substr(static_cast<size_t>(start - remaining.begin()));
      auto end = std::ranges::find_if(remaining, [](char8_t c) { return text::IsASCIIWhitespace(c); });
      auto token = remaining.substr(0, static_cast<size_t>(end - remaining.begin()));
      if (!token.empty() && text::IsASCIICaselessEqual(token, relation))
      {
        return true;
      }
      remaining = remaining.substr(token.size());
    }
    return false;
  }

  void HyperlinkAlgorithms::FollowTheHyperlink(HTMLElement &element, dom::Event &event) noexcept
  {
    event.SetDefaultHandled();

    Ref<dom::Document> document = ShareRef(element.NodeDocument());
    RefPtr<LocalFrame> frame = ShareRefPtr(document->Frame());
    if (!frame)
    {
      return;
    }

    if (!Is<HTMLAnchorElement>(element) && !element.IsConnected())
    {
      return;
    }

    auto href = element.GetAttribute(u8"href").value_or(dom::DOMString());
    auto trimmed = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(href);
    utf8_string url(trimmed);
    AppendServerMapMousePosition(url, event);
    auto completedURL = document->CompleteURL(url);

    // FIXME: The suggested filename is sanitized (ResourceResponse::sanitizeSuggestedFilename), and ignored
    // for a cross-origin URL, once there are downloads and origins.
    auto downloadAttribute = element.GetAttribute(u8"download");
    auto download = downloadAttribute ? dom::DOMStringAtom(*downloadAttribute) : dom::DOMStringAtom::Null();

    // FIXME: The referrerpolicy attribute, once requests have referrers.
    auto referrerPolicy =
      HasRel(element, u8"noreferrer") ? ReferrerPolicy::NoReferrer : ReferrerPolicy::EmptyString;

    auto effectiveTarget = EffectiveTarget(element);
    auto newFrameOpenerPolicy = NewFrameOpenerPolicy::Allow;
    if (HasRel(element, u8"noopener") || HasRel(element, u8"noreferrer")
        || (!HasRel(element, u8"opener")
            && text::IsASCIICaselessEqual(effectiveTarget.View(), utf8_stringview(u8"_blank"))
            && !completedURL.ProtocolIsJavaScript()))
    {
      newFrameOpenerPolicy = NewFrameOpenerPolicy::Suppress;
    }

    frame->Loader().ChangeLocation(completedURL, effectiveTarget, &event, referrerPolicy,
                                   newFrameOpenerPolicy, download, NavigationHistoryBehavior::Push, &element);

    // FIXME: Pings (the ping attribute), once there are network loads.
  }
}
