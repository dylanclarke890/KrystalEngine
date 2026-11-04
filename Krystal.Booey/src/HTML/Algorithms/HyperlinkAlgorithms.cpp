#include "Krystal.Booey/HTML/Algorithms/HyperlinkAlgorithms.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <cmath>
#include <format>

namespace krys::boo::html
{
  namespace
  {
    /// @brief Where @p event clicked in @p image, in CSS pixels from its top left corner.
    KRYS_NODISCARD Maybe<FloatPoint> ServerMapClickPosition(const HTMLImageElement &image,
                                                            const dom::Event &event) noexcept
    {
      auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event);
      auto *renderer = DynamicDowncast<RenderImage>(image.Renderer());
      if (!mouseEvent || !renderer)
      {
        return null;
      }

      // FIXME: This should probably pass UseTransforms in the EnumSet<MapCoordinatesMode>.
      return renderer->AbsoluteToLocal(
        FloatPoint(static_cast<float>(mouseEvent->PageX()), static_cast<float>(mouseEvent->PageY())));
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
    (void)element.SetAttribute(u8"href", url.String());
  }

  dom::DOMStringAtom HyperlinkAlgorithms::EffectiveTarget(const HTMLElement &element) noexcept
  {
    // 1. If target is null:
    // 1a. If element has a target attribute, then set target to that attribute's value.
    // 1b. Otherwise, if element's node document contains a base element with a target attribute, set target
    //     to the value of the target attribute of the first such base element.
    auto target = element.GetAttribute(u8"target");
    auto effectiveTarget =
      target ? dom::DOMStringAtom(*target) : dom::DOMStringAtom(element.NodeDocument().BaseTarget());

    // 2. If target is not null, and contains an ASCII tab or newline and a U+003C (<), then set target to
    //    "_blank".
    // 3. Return target.
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

  bool HyperlinkAlgorithms::GetNoopener(const HTMLElement &element, const url::URL &url,
                                        dom::DOMStringView target) noexcept
  {
    // 1. If element's link types include the noopener or noreferrer keyword, then return true.
    if (HasRel(element, u8"noopener") || HasRel(element, u8"noreferrer"))
    {
      return true;
    }

    // 2. If element's link types do not include the opener keyword and target is an ASCII case-insensitive
    //    match for "_blank", then return true.
    // (Not for a javascript: URL, which opens no document.)
    if (!HasRel(element, u8"opener") && text::IsASCIICaselessEqual(target, utf8_stringview(u8"_blank"))
        && !url.ProtocolIsJavaScript())
    {
      return true;
    }

    // 3. (Skipped: there's no blob URL store, so no URL has a blob URL entry.)
    // 4. Return false.
    return false;
  }

  void HyperlinkAlgorithms::ActivationBehavior(HTMLElement &element, dom::Event &event) noexcept
  {
    // 1. If element has no href attribute, then return.
    if (!element.HasAttribute(u8"href"))
    {
      return;
    }

    // 2. Let hyperlinkSuffix be null.
    Maybe<utf8_string> hyperlinkSuffix;

    // 3. If element is an a element, and event's target is an img with an ismap attribute specified:
    auto *targetNode = DynamicDowncast<dom::Node>(event.Target().get());
    auto *image = targetNode ? DynamicDowncast<HTMLImageElement>(*targetNode) : nullptr;
    if (Is<HTMLAnchorElement>(element) && image && image->HasAttribute(u8"ismap"))
    {
      // 3a. Let x and y be 0.
      long x = 0;
      long y = 0;

      // 3b. If event's isTrusted attribute is initialized to true, then set x to the distance in CSS pixels
      // from
      //     the left edge of the image to the location of the click, and set y to the distance in CSS pixels
      //     from the top edge of the image to the location of the click.
      if (event.IsTrusted())
      {
        if (auto position = ServerMapClickPosition(*image, event))
        {
          x = std::lround(position->X());
          y = std::lround(position->Y());
        }
      }

      // 3c. If x is negative, set x to 0.
      // 3d. If y is negative, set y to 0.
      x = std::max(x, 0L);
      y = std::max(y, 0L);

      // 3e. Set hyperlinkSuffix to the concatenation of U+003F (?), the value of x expressed as a base-ten
      //     integer using ASCII digits, U+002C (,), and the value of y expressed as a base-ten integer using
      //     ASCII digits.
      auto suffix = std::format("?{},{}", x, y);
      hyperlinkSuffix = utf8_string(suffix.begin(), suffix.end());
    }

    // 4. Let userInvolvement be event's user navigation involvement.
    // (The event goes to the loader, which tells the host about it.)
    // 5. If the user has expressed a preference to download the hyperlink, then set userInvolvement to
    //    "browser UI".
    // (There's no browser user interface to express one.)
    // 6. If element has a download attribute, or if the user has expressed a preference to download the
    //    hyperlink, then download the hyperlink created by element with hyperlinkSuffix set to
    //    hyperlinkSuffix and userInvolvement set to userInvolvement.
    // 7. Otherwise, follow the hyperlink created by element with hyperlinkSuffix set to hyperlinkSuffix and
    //    userInvolvement set to userInvolvement.
    // (Both follow it: the loader gives a download to the host.)
    // FIXME: Links in editable content follow the editable link behaviour setting (Settings::
    // editableLinkBehavior), once there is one; by default they're live, as here.
    FollowTheHyperlink(element, event, hyperlinkSuffix);
  }

  void HyperlinkAlgorithms::FollowTheHyperlink(HTMLElement &subject, dom::Event &event,
                                               const Maybe<utf8_string> &hyperlinkSuffix) noexcept
  {
    event.SetDefaultHandled();

    // 1. If subject cannot navigate, then return.
    Ref<dom::Document> document = ShareRef(subject.NodeDocument());
    RefPtr<LocalFrame> frame = ShareRefPtr(document->Frame());
    if (!frame || (!Is<HTMLAnchorElement>(subject) && !subject.IsConnected()))
    {
      return;
    }

    // 2. Let targetAttributeValue be the empty string.
    // 3. If subject is an a or area element, then set targetAttributeValue to the result of getting an
    //    element's target given subject.
    auto targetAttributeValue = EffectiveTarget(subject);

    // 4. Let urlRecord be the result of encoding-parsing a URL given subject's href attribute value, relative
    //    to subject's node document.
    auto href = subject.GetAttribute(u8"href").value_or(dom::DOMString());
    auto urlRecord = document->CompleteURL(infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(href));

    // 5. If urlRecord is failure, then return.
    if (!urlRecord.IsValid())
    {
      return;
    }

    // 6. Let noopener be the result of getting an element's noopener with subject, urlRecord, and
    //    targetAttributeValue.
    auto noopener = GetNoopener(subject, urlRecord, targetAttributeValue.View());

    // 7. Let targetNavigable be the first return value of applying the rules for choosing a navigable given
    //    targetAttributeValue, subject's node navigable, and noopener.
    // 8. If targetNavigable is null, then return.
    // (The loader chooses it, or has the host open a window.)

    // 9. Let urlString be the result of applying the URL serializer to urlRecord.
    // 10. If hyperlinkSuffix is non-null, then append it to urlString.
    if (hyperlinkSuffix)
    {
      utf8_string urlString(urlRecord.String());
      urlString += *hyperlinkSuffix;
      urlRecord = document->CompleteURL(urlString);
    }

    // 11. Navigate targetNavigable to urlString using subject's node document, with referrerPolicy set to
    //     subject's hyperlink referrer policy, userInvolvement set to userInvolvement, and sourceElement set
    //     to subject.
    // FIXME: The suggested filename is sanitized (ResourceResponse::sanitizeSuggestedFilename), and ignored
    // for a cross-origin URL, once there are downloads and origins.
    auto downloadAttribute = subject.GetAttribute(u8"download");
    auto download = downloadAttribute ? dom::DOMStringAtom(*downloadAttribute) : dom::DOMStringAtom::Null();
    frame->Loader().ChangeLocation(urlRecord, targetAttributeValue, &event, HyperlinkReferrerPolicy(subject),
                                   noopener ? NewFrameOpenerPolicy::Suppress : NewFrameOpenerPolicy::Allow,
                                   download, &subject);

    // FIXME: Pings (the ping attribute), once there are network loads.
  }

  ReferrerPolicy HyperlinkAlgorithms::HyperlinkReferrerPolicy(const HTMLElement &subject) noexcept
  {
    // 1. If subject's link types includes the noreferrer keyword, then return "no-referrer".
    if (HasRel(subject, u8"noreferrer"))
    {
      return ReferrerPolicy::NoReferrer;
    }

    // 2. Return the current state of subject's referrerpolicy content attribute.
    // FIXME: The referrerpolicy attribute, once requests have referrers.
    return ReferrerPolicy::EmptyString;
  }
}
