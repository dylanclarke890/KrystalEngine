#include "Krystal.Booey/SVG/SVGAElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/HyperlinkAlgorithms.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGTextContentElement.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::svg
{
  SVGAElement::SVGAElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::A),
        SVGURIReference(this), _target(SVGAnimatedString::Create(this))
  {
    SetEventTargetFlag(dom::EventTargetFlags::HasActivationBehavior);

    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGAElement::_target>(SVGNames::TargetAttr());
    }
  }

  SVGAElement::~SVGAElement() noexcept = default;

  void SVGAElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                     Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::TargetAttr())
    {
      _target->SetBaseValInternal(dom::DOMString {value.value_or(dom::DOMStringView {})});
      return;
    }
    else if (name == SVGNames::RelAttr())
    {
      // Update SVGAElement::relList() if more rel attributes values are supported.
      auto hasToken = [&](dom::DOMStringView relation)
      {
        dom::DOMStringView remaining = value.value_or(dom::DOMStringView {});
        while (!remaining.empty())
        {
          auto start =
            std::ranges::find_if_not(remaining, [](char8_t c) { return text::IsASCIIWhitespace(c); });
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
      };
      if (hasToken(u8"noreferrer"))
      {
        _linkRelations.Add(SVGALinkRelation::NoReferrer);
      }
      if (hasToken(u8"noopener"))
      {
        _linkRelations.Add(SVGALinkRelation::NoOpener);
      }
      if (hasToken(u8"opener"))
      {
        _linkRelations.Add(SVGALinkRelation::Opener);
      }
    }

    SVGURIReference::ParseAttribute(name, value);
    SVGGraphicsElement::AttributeChanged(name, oldValue, value);
  }

  void SVGAElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (SVGURIReference::IsKnownAttribute(attributeName))
    {
      // Whether the element is a link follows from the attribute (css::IsLink).
      InstanceInvalidationGuard guard(*this);
      return;
    }

    SVGGraphicsElement::SVGAttributeChanged(attributeName);
  }

  RenderPtr<RenderElement> SVGAElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    auto *svgParent = DynamicDowncast<SVGElement>(ParentNode());
    if (svgParent && Is<SVGTextContentElement>(*svgParent))
    {
      return CreateRenderer<RenderSVGInline>(RenderObject::Type::SVGInline, *this, krys::move(style));
    }

    return CreateRenderer<LegacyRenderSVGTransformableContainer>(*this, krys::move(style));
  }

  void SVGAElement::ActivationBehavior(dom::Event &event) noexcept
  {
    if (!css::IsLink(*this))
    {
      return;
    }
    FollowTheHyperlink(event);
  }

  void SVGAElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (css::IsLink(*this))
    {
      if (Focused() && html::HyperlinkAlgorithms::IsEnterKeyKeydownEvent(event))
      {
        event.SetDefaultHandled();
        (void)DispatchSimulatedClick(&event);
        return;
      }

      // A click is followed as the activation behaviour; other buttons' clicks (the middle button opening the
      // link elsewhere) are followed here, and the host decides from the button.
      if (event.Type() == EventNames::AuxClick && dom::MouseEvent::CanTriggerActivationBehavior(event))
      {
        FollowTheHyperlink(event);
        return;
      }
    }

    SVGGraphicsElement::DefaultEventHandler(event);
  }

  void SVGAElement::FollowTheHyperlink(dom::Event &event) noexcept
  {
    auto url = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(Href());

    if (!url.empty() && url.front() == u8'#')
    {
      // FIXME: An animation element the fragment names begins (SVGSMILElement::beginByLinkActivation) instead
      // of the link being followed, with SMIL (B11i).
    }

    Ref<dom::Document> document = ShareRef(NodeDocument());
    auto completedURL = document->CompleteURL(utf8_string(url));

    auto target = Target();
    if (target.View().empty()
        && GetAttributeNS(XLinkNames::ShowAttr().NamespaceURI(), XLinkNames::ShowAttr().LocalName())
             == u8"new")
    {
      target = dom::DOMStringAtom(u8"_blank");
    }
    event.SetDefaultHandled();

    auto referrerPolicy =
      HasRel(SVGALinkRelation::NoReferrer) ? ReferrerPolicy::NoReferrer : ReferrerPolicy::EmptyString;
    auto newFrameOpenerPolicy = NewFrameOpenerPolicy::Allow;
    if (HasRel(SVGALinkRelation::NoOpener) || HasRel(SVGALinkRelation::NoReferrer)
        || (!HasRel(SVGALinkRelation::Opener)
            && text::IsASCIICaselessEqual(target.View(), utf8_stringview(u8"_blank"))
            && !completedURL.ProtocolIsJavaScript()))
    {
      newFrameOpenerPolicy = NewFrameOpenerPolicy::Suppress;
    }

    if (RefPtr<LocalFrame> frame = ShareRefPtr(document->Frame()))
    {
      frame->Loader().ChangeLocation(completedURL, target, &event, referrerPolicy, newFrameOpenerPolicy,
                                     dom::DOMStringAtom::Null(), NavigationHistoryBehavior::Auto, this);
    }
  }

  bool SVGAElement::CanStartSelection() const noexcept
  {
    if (!css::IsLink(*this))
    {
      return SVGElement::CanStartSelection();
    }

    return HasEditableStyle();
  }

  bool SVGAElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    // http://www.w3.org/2003/01/REC-SVG11-20030114-errata#linking-text-environment
    // The 'a' element may contain any element that its parent may contain, except itself.
    if (auto *svgChild = DynamicDowncast<SVGElement>(child); svgChild && Is<SVGAElement>(*svgChild))
    {
      return false;
    }

    if (auto *parent = ParentElement())
    {
      if (auto *svgParent = DynamicDowncast<SVGElement>(*parent))
      {
        return svgParent->ChildShouldCreateRenderer(child);
      }
    }

    return SVGElement::ChildShouldCreateRenderer(child);
  }

  dom::DOMTokenList &SVGAElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = dom::ElementAlgorithms::CreateDOMTokenList(
        *this, SVGNames::RelAttr().LocalName(),
        [](dom::Document &, dom::DOMStringView token)
        {
          return text::IsASCIICaselessEqual(token, utf8_stringview(u8"noreferrer"))
                 || text::IsASCIICaselessEqual(token, utf8_stringview(u8"noopener"))
                 || text::IsASCIICaselessEqual(token, utf8_stringview(u8"opener"));
        });
    }
    return *_relList;
  }

  bool SVGAElement::HasRel(SVGALinkRelation relation) const noexcept
  {
    return _linkRelations.Contains(relation);
  }
}
