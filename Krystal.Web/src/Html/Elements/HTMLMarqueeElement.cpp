#include "Krystal.Web/Html/Elements/HTMLMarqueeElement.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Web/Animation/KeyframeAnimationOptions.hpp"
#include "Krystal.Web/Animation/Keyframes/KeyframeInput.hpp"
#include "Krystal.Web/Animation/WebAnimation.hpp"
#include "Krystal.Web/Css/UserAgentStyleSheets.hpp"
#include "Krystal.Web/Dom/Internals/ElementFactory.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/Elements/HTMLDivElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"
#include "Krystal.Web/Html/Internals/HTMLElementFactory.hpp"
#include "Krystal.Web/Html/Internals/PresentationalHints.hpp"
#include "Krystal.Web/Html/MicroParsers/Numbers.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include <algorithm>
#include <cmath>
#include <format>

namespace krys::web
{
  namespace
  {
    constexpr uint32 DefaultMarqueeScrollAmount = 6;
    constexpr uint32 DefaultMarqueeScrollDelay = 85;
    constexpr uint32 MinimumMarqueeScrollDelay = 60;

    enum class MarqueeBehavior : uint8
    {
      Scroll,
      Slide,
      Alternate,
    };

    enum class MarqueeDirection : uint8
    {
      Left,
      Right,
      Up,
      Down,
    };

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#attr-marquee-behavior
    KRYS_NODISCARD MarqueeBehavior BehaviorOf(const HTMLMarqueeElement &marquee) noexcept
    {
      auto value = marquee.GetAttribute(u8"behavior").value_or(u8"");
      if (text::IsASCIICaselessEqual(value, u8"alternate"))
      {
        return MarqueeBehavior::Alternate;
      }
      if (text::IsASCIICaselessEqual(value, u8"slide"))
      {
        return MarqueeBehavior::Slide;
      }
      return MarqueeBehavior::Scroll;
    }

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#attr-marquee-direction
    KRYS_NODISCARD MarqueeDirection DirectionOf(const HTMLMarqueeElement &marquee) noexcept
    {
      auto value = marquee.GetAttribute(u8"direction").value_or(u8"");
      if (text::IsASCIICaselessEqual(value, u8"right"))
      {
        return MarqueeDirection::Right;
      }
      if (text::IsASCIICaselessEqual(value, u8"up"))
      {
        return MarqueeDirection::Up;
      }
      if (text::IsASCIICaselessEqual(value, u8"down"))
      {
        return MarqueeDirection::Down;
      }
      return MarqueeDirection::Left;
    }

    /// @brief Where a loop moves the mover from and to, and how far that is.
    struct MarqueeAnimationParameters
    {
      float Begin {0};
      float End {0};
      float Distance {0};
    };

    KRYS_NODISCARD DOMString MarqueeTransform(bool horizontal, float offset) noexcept
    {
      auto text = std::format("translate{}({}px)", horizontal ? 'X' : 'Y', offset);
      return DOMString(text.begin(), text.end());
    }
  }

  HTMLMarqueeElement::HTMLMarqueeElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Marquee)
  {
  }

  HTMLMarqueeElement::~HTMLMarqueeElement() noexcept = default;

  Ref<HTMLMarqueeElement> HTMLMarqueeElement::Create(Document &document) noexcept
  {
    auto marquee = CreateRef<HTMLMarqueeElement>(document);
    marquee->EnsureUserAgentShadowRoot();
    return marquee;
  }

  void HTMLMarqueeElement::DidAddUserAgentShadowRoot(web::ShadowRoot &root) noexcept
  {
    auto &document = NodeDocument();
    auto style = document.CreateElementNS(infra::namespaces::HTML, u8"style").ReleaseValue();
    (void)style->TextContent(DOMString(css::MarqueeElementShadowUserAgentStyleSheet));
    (void)root.AppendChild(*style);

    auto mover = dom::ElementFactory::CreateHTMLElement<HTMLDivElement>(document, u8"div");
    (void)mover->AppendChild(*dom::ElementFactory::CreateHTMLElement<HTMLSlotElement>(document, u8"slot"));
    (void)root.AppendChild(*mover);
    _mover = CreateWeakPtr(mover.get());
  }

  HTMLDivElement *HTMLMarqueeElement::Mover() const noexcept
  {
    return _mover.get();
  }

  void HTMLMarqueeElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    // A marquee is turned on as it's made; it starts moving once it's in a document.
    if (IsConnected())
    {
      Start();
    }
  }

  void HTMLMarqueeElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (oldAncestor.IsConnected())
    {
      Stop();
    }
  }

  uint32 HTMLMarqueeElement::HSpace() const noexcept
  {
    return html::Reflect<uint32>(*this, u8"hspace");
  }

  void HTMLMarqueeElement::HSpace(uint32 value) noexcept
  {
    html::Reflect<uint32>(*this, u8"hspace", krys::move(value));
  }

  uint32 HTMLMarqueeElement::VSpace() const noexcept
  {
    return html::Reflect<uint32>(*this, u8"vspace");
  }

  void HTMLMarqueeElement::VSpace(uint32 value) noexcept
  {
    html::Reflect<uint32>(*this, u8"vspace", krys::move(value));
  }

  // https://html.spec.whatwg.org/multipage/obsolete.html#marquee-loop-count
  int32 HTMLMarqueeElement::Loop() const noexcept
  {
    // A marquee element has a marquee loop count, which, if the element has a loop attribute, and parsing its
    // value using the rules for parsing integers does not return an error or a number less than 1, is the
    // parsed value, and otherwise is -1.
    if (auto value = GetAttribute(u8"loop"))
    {
      if (auto parsed = html::Numbers::ParseInteger(*value); parsed.Success() && parsed.Value >= 1)
      {
        return static_cast<int32>(std::min<int64>(parsed.Value, std::numeric_limits<int32>::max()));
      }
    }
    return -1;
  }

  // https://html.spec.whatwg.org/multipage/obsolete.html#dom-marquee-loop
  void HTMLMarqueeElement::Loop(int32 value) noexcept
  {
    // On setting, if the new value is different than the element's marquee loop count and either greater than
    // zero or equal to -1, must set the element's loop content attribute (adding it if necessary) to the
    // valid integer that represents the new value. (Other values are ignored.)
    if (value != Loop() && (value > 0 || value == -1))
    {
      auto text = std::to_string(value);
      (void)SetAttribute(u8"loop", DOMString(text.begin(), text.end()));
    }
  }

  // https://html.spec.whatwg.org/multipage/obsolete.html#marquee-scroll-distance
  uint32 HTMLMarqueeElement::ScrollAmount() const noexcept
  {
    return html::Reflect<uint32, html::ReflectDefault<uint32> {DefaultMarqueeScrollAmount}>(*this,
                                                                                            u8"scrollamount");
  }

  void HTMLMarqueeElement::ScrollAmount(uint32 value) noexcept
  {
    html::Reflect<uint32, html::ReflectDefault<uint32> {DefaultMarqueeScrollAmount}>(*this, u8"scrollamount",
                                                                                     krys::move(value));
  }

  uint32 HTMLMarqueeElement::ScrollDelay() const noexcept
  {
    return html::Reflect<uint32, html::ReflectDefault<uint32> {DefaultMarqueeScrollDelay}>(*this,
                                                                                           u8"scrolldelay");
  }

  void HTMLMarqueeElement::ScrollDelay(uint32 value) noexcept
  {
    html::Reflect<uint32, html::ReflectDefault<uint32> {DefaultMarqueeScrollDelay}>(*this, u8"scrolldelay",
                                                                                    krys::move(value));
  }

  bool HTMLMarqueeElement::TrueSpeed() const noexcept
  {
    return html::Reflect<bool>(*this, u8"truespeed");
  }

  void HTMLMarqueeElement::TrueSpeed(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"truespeed", krys::move(value));
  }

  // https://html.spec.whatwg.org/multipage/obsolete.html#marquee-scroll-interval
  uint32 HTMLMarqueeElement::ScrollInterval() const noexcept
  {
    // 1. If the element has a scrolldelay attribute, and parsing its value using the rules for parsing
    //    non-negative integers does not return an error, then let delay be the parsed value. Otherwise, let
    //    delay be 85.
    // 2. If the element does not have a truespeed attribute, and the delay value is less than 60, then let
    //    delay be 60 instead.
    // 3. The marquee scroll interval is delay, interpreted in milliseconds.
    auto delay = ScrollDelay();
    if (!HasAttribute(u8"truespeed") && delay < MinimumMarqueeScrollDelay)
    {
      delay = MinimumMarqueeScrollDelay;
    }
    return delay;
  }

  // https://html.spec.whatwg.org/multipage/obsolete.html#dom-marquee-start
  void HTMLMarqueeElement::Start() noexcept
  {
    // The marquee element must be turned on: it moves on from the next animation frame.
    if (_continueCallbackRequestId)
    {
      return;
    }
    _continueCallbackRequestId = NodeDocument().RequestAnimationFrame(
      [weakThis = CreateWeakPtr(this)](dom::DOMHighResTimeStamp)
      {
        if (RefPtr<HTMLMarqueeElement> marquee = ShareRefPtr(weakThis.get()))
        {
          marquee->_continueCallbackRequestId = 0;
          marquee->ContinueAnimation();
        }
      });
  }

  // https://html.spec.whatwg.org/multipage/obsolete.html#dom-marquee-stop
  void HTMLMarqueeElement::Stop() noexcept
  {
    // The marquee element must be turned off: it holds where it is.
    if (_continueCallbackRequestId)
    {
      NodeDocument().CancelAnimationFrame(_continueCallbackRequestId);
      _continueCallbackRequestId = 0;
      return;
    }
    if (_player)
    {
      (void)_player->Pause();
    }
  }

  bool HTMLMarqueeElement::ShouldContinue() const noexcept
  {
    // https://html.spec.whatwg.org/multipage/obsolete.html#increment-the-marquee-current-loop-index
    // 1. If the marquee loop count is -1, then return. (A slide loops once, as Chrome slides.)
    auto loopCount = Loop();
    if (loopCount <= 0 && BehaviorOf(*this) == MarqueeBehavior::Slide)
    {
      loopCount = 1;
    }
    if (loopCount <= 0)
    {
      return true;
    }
    // 3. If the marquee current loop index is now greater than or equal to the element's marquee loop count,
    //    turn off the marquee element.
    return _currentLoopIndex < loopCount;
  }

  void HTMLMarqueeElement::ContinueAnimation() noexcept
  {
    RefPtr<HTMLDivElement> mover = ShareRefPtr(_mover.get());
    if (!mover || !IsConnected() || !ShouldContinue())
    {
      return;
    }

    // A marquee turned off and on again plays on from where it stopped.
    if (_player && _player->GetPlayState() == WebAnimation::PlayState::Paused)
    {
      (void)_player->Play();
      return;
    }

    // The marquee's content box, and the size of its contents: the mover's max-content width, or its height.
    NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    auto *marqueeBox = DynamicDowncast<RenderBox>(Renderer());
    auto *moverBox = DynamicDowncast<RenderBox>(mover->Renderer());
    if (!marqueeBox || !moverBox)
    {
      return;
    }
    auto direction = DirectionOf(*this);
    bool horizontal = direction == MarqueeDirection::Left || direction == MarqueeDirection::Right;
    float marqueeSize =
      horizontal ? marqueeBox->ContentBoxWidth().ToFloat() : marqueeBox->ContentBoxHeight().ToFloat();
    float contentSize =
      horizontal ? moverBox->MaxPreferredLogicalWidth().ToFloat() : moverBox->ContentBoxHeight().ToFloat();
    float innerSize = marqueeSize - contentSize;

    // Each loop moves the contents from one place to another: in a scroll, in from one edge and out the
    // other; in a slide, in from one edge to the other; in an alternation, between the edges, back on odd
    // loops.
    bool forwards = direction == MarqueeDirection::Left || direction == MarqueeDirection::Up;
    MarqueeAnimationParameters parameters;
    switch (BehaviorOf(*this))
    {
      case MarqueeBehavior::Alternate:
        parameters = forwards
                       ? MarqueeAnimationParameters {std::max(innerSize, 0.f), std::min(innerSize, 0.f), 0}
                       : MarqueeAnimationParameters {std::min(innerSize, 0.f), std::max(innerSize, 0.f), 0};
        parameters.Distance = std::abs(innerSize);
        if (_currentLoopIndex % 2)
        {
          std::swap(parameters.Begin, parameters.End);
        }
        break;
      case MarqueeBehavior::Slide:
        parameters = forwards ? MarqueeAnimationParameters {marqueeSize, 0, marqueeSize}
                              : MarqueeAnimationParameters {-contentSize, innerSize, marqueeSize};
        break;
      case MarqueeBehavior::Scroll:
        parameters = forwards
                       ? MarqueeAnimationParameters {marqueeSize, -contentSize, marqueeSize + contentSize}
                       : MarqueeAnimationParameters {-contentSize, marqueeSize, marqueeSize + contentSize};
        break;
    }

    // It moves the marquee scroll distance each marquee scroll interval.
    auto scrollAmount = ScrollAmount();
    double duration = scrollAmount ? parameters.Distance * ScrollInterval() / scrollAmount : 0;
    if (duration <= 0)
    {
      return;
    }

    List<InputKeyframe> keyframes(2);
    keyframes[0].Properties.emplace_back(u8"transform", MarqueeTransform(horizontal, parameters.Begin));
    keyframes[1].Properties.emplace_back(u8"transform", MarqueeTransform(horizontal, parameters.End));
    KeyframeAnimationOptions options;
    options.Duration = duration;
    options.Fill = FillMode::Forwards;
    auto animation = mover->Animate(KeyframesInput(krys::move(keyframes)),
                                    Variant<double, KeyframeAnimationOptions>(krys::move(options)));
    if (animation.HasException())
    {
      return;
    }
    _player = animation.ReleaseValue();

    // https://html.spec.whatwg.org/multipage/obsolete.html#increment-the-marquee-current-loop-index
    // The loop's end increments the marquee current loop index, and the next loop starts.
    _player->WhenFinished(
      [weakThis = CreateWeakPtr(this), player = _player.get()](WebAnimation &)
      {
        RefPtr<HTMLMarqueeElement> marquee = ShareRefPtr(weakThis.get());
        if (!marquee || marquee->_player.get() != player)
        {
          return;
        }
        // 2. Increment the marquee current loop index by one.
        ++marquee->_currentLoopIndex;
        marquee->Start();
      });
  }

  DOMString HTMLMarqueeElement::Behavior() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"behavior").value_or(u8"");
  }

  void HTMLMarqueeElement::Behavior(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"behavior", krys::move(value));
  }

  DOMString HTMLMarqueeElement::BgColor() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLMarqueeElement::BgColor(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"bgcolor", krys::move(value));
  }

  DOMString HTMLMarqueeElement::Direction() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"direction").value_or(u8"");
  }

  void HTMLMarqueeElement::Direction(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"direction", krys::move(value));
  }

  DOMString HTMLMarqueeElement::Height() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLMarqueeElement::Height(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"height", krys::move(value));
  }

  DOMString HTMLMarqueeElement::Width() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLMarqueeElement::Width(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"width", krys::move(value));
  }

  bool
    HTMLMarqueeElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                           const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null
        && (localName == u8"width" || localName == u8"height" || localName == u8"bgcolor"
            || localName == u8"vspace" || localName == u8"hspace"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLMarqueeElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                                  const DOMStringAtom &namespaceURI,
                                                                  DOMStringView value,
                                                                  css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"width")
    {
      if (!value.empty())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::Width, value);
      }
      return;
    }
    if (localName == u8"height")
    {
      if (!value.empty())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::Height, value);
      }
      return;
    }
    if (localName == u8"bgcolor")
    {
      if (!value.empty())
      {
        AddHTMLColorToStyle(style, css::PropertyId::BackgroundColor, value);
      }
      return;
    }
    if (localName == u8"vspace")
    {
      if (!value.empty())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::MarginTop, value);
        AddHTMLLengthToStyle(style, css::PropertyId::MarginBottom, value);
      }
      return;
    }
    if (localName == u8"hspace")
    {
      if (!value.empty())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::MarginLeft, value);
        AddHTMLLengthToStyle(style, css::PropertyId::MarginRight, value);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }
}
