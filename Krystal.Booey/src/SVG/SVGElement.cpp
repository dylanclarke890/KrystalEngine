#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/RenderAncestorIterator.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGProperty.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyAnimatorFactory.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGDocumentExtensions.hpp"
#include "Krystal.Booey/SVG/SVGElementRareData.hpp"
#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGSymbolElement.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <algorithm>

namespace krys::boo::svg
{
  namespace
  {
    KRYS_NODISCARD List<Ref<SVGElement>> CopyToRefs(const List<WeakPtr<SVGElement>> &elements) noexcept
    {
      List<Ref<SVGElement>> result;
      result.reserve(elements.size());
      for (const auto &element : elements)
      {
        if (element)
        {
          result.push_back(ShareRef(*element));
        }
      }
      return result;
    }

    template <typename T>
    KRYS_NODISCARD T *DynamicDowncastSVG(dom::Node *node) noexcept
    {
      auto *svgElement = DynamicDowncast<SVGElement>(node);
      return svgElement ? DynamicDowncast<T>(*svgElement) : nullptr;
    }
  }

  SVGElement::SVGElement(dom::Document &document, UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                         SVGElementInterface interface, SVGElementFlags flags,
                         dom::NodeFlags nodeFlags) noexcept
      : StyledElement(document, dom::NullQualifiedName(),
                      dom::NodeFlags::IsSVGElement | dom::NodeFlags::HasCustomStyleResolveCallbacks
                        | nodeFlags),
        _flags(flags), _interface(interface),
        _propertyAnimatorFactory(CreateUnique<SVGPropertyAnimatorFactory>()),
        _propertyRegistry(krys::move(propertyRegistry)), _className(SVGAnimatedString::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGElement::_className>(SVGNames::ClassAttr());
    }
  }

  SVGElement::SVGElement(dom::Document &document, SVGElementInterface interface, SVGElementFlags flags,
                         dom::NodeFlags nodeFlags) noexcept
      : SVGElement(document, CreateUnique<PropertyRegistry>(*this), interface, flags, nodeFlags)
  {
  }

  SVGElement::SVGElement(dom::Document &document) noexcept : SVGElement(document, SVGElementInterface::None)
  {
  }

  SVGElement::~SVGElement() noexcept
  {
    if (_svgRareData)
    {
      // NOTE: a document's nodes are destroyed with it without being removed, and an element before its
      // children, so an element can be destroyed while its descendants still reference it (an animation
      // element's target is its parent by default); their references are dropped.
      auto referencingElements = CopyToRefs(_svgRareData->TakeReferencingElements());
      krys_assert(std::ranges::all_of(referencingElements,
                                      [&](const auto &element) { return element->IsDescendantOf(*this); }));
      for (auto &instance : Instances())
      {
        instance->_svgRareData->SetCorrespondingElement(nullptr);
      }
      // NOTE: a document's nodes are destroyed with it without being removed, so an instance can be destroyed
      // while it's still in its <use> element's shadow tree; it leaves its original's instances.
      if (auto *correspondingElement = _svgRareData->CorrespondingElement())
      {
        correspondingElement->EnsureSVGRareData().RemoveInstance(*this);
      }
      _svgRareData = nullptr;
    }

    if (auto *extensions = NodeDocument().SVGExtensionsIfExists())
    {
      extensions->RemoveElementToRebuild(*this);
    }

    if (HasPendingResources())
    {
      TreeScopeForSVGReferences().RemoveElementFromPendingSVGResources(*this);
      krys_debug_assert(!HasPendingResources());
    }
  }

  void SVGElement::WillRecalcStyle(EnumSet<style::Change> change) noexcept
  {
    if (!_svgRareData || StyleResolutionShouldRecompositeLayer())
    {
      return;
    }
    // If the style changes because of a regular property change (not induced by SMIL animations themselves)
    // reset the "computed style without SMIL style properties", so the base value change gets reflected.
    if (!change.IsEmpty() || NeedsStyleRecalc())
    {
      _svgRareData->SetNeedsOverrideComputedStyleUpdate();
    }
  }

  SVGElementRareData &SVGElement::EnsureSVGRareData() noexcept
  {
    if (!_svgRareData)
    {
      _svgRareData = CreateUnique<SVGElementRareData>();
    }
    return *_svgRareData;
  }

  bool SVGElement::IsInnerSVGSVGElement() const noexcept
  {
    if (!IsSVGSvgElement())
    {
      return false;
    }

    // Element may not be in the document, pretend we're outermost for viewport(), getCTM(), etc.
    if (!ParentNode())
    {
      return false;
    }

    return Is<SVGElement>(*ParentNode());
  }

  bool SVGElement::IsOutermostSVGSVGElement() const noexcept
  {
    if (!IsSVGSvgElement())
    {
      return false;
    }

    // Element may not be in the document, pretend we're outermost for viewport(), getCTM(), etc.
    auto *parent = ParentNode();
    if (!parent)
    {
      return true;
    }

    // We act like an outermost SVG element, if we're a direct child of a <foreignObject> element.
    if (DynamicDowncastSVG<SVGForeignObjectElement>(parent))
    {
      return true;
    }

    // If we're inside the shadow tree of a <use> element, we're always an inner <svg> element.
    if (IsInShadowTree() && DynamicDowncastSVG<SVGUseElement>(dom::TreeQueries::ShadowHost(*this)))
    {
      return false;
    }

    // This is true whenever this is the outermost SVG, even if there are HTML elements outside it
    return !Is<SVGElement>(*parent);
  }

  boo::ColorInterpolation SVGElement::GetColorInterpolation() const noexcept
  {
    if (auto *renderer = Renderer())
    {
      return renderer->Style().ColorInterpolationFilters();
    }

    // Try to determine the property value from the computed style.
    if (const auto *style = const_cast<SVGElement *>(this)->ComputedStyle(null))
    {
      return style->ColorInterpolationFilters();
    }

    return boo::ColorInterpolation::Auto;
  }

  void SVGElement::ReportAttributeParsingError(SVGParsingError error, const dom::QualifiedName &name,
                                               dom::DOMStringView value) noexcept
  {
    if (error == SVGParsingError::None)
    {
      return;
    }

    // FIXME: Report "Invalid value for <tag> attribute name="value"" (or "Invalid negative value for ...") to
    // the console, once there's one.
    (void)name;
    (void)value;
  }

  void SVGElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    StyledElement::OnRemove(isSubtreeRoot, oldAncestor);

    if (isSubtreeRoot)
    {
      _hasRegisteredWithParentForRelativeLengths = false;
      if (auto *oldParent = DynamicDowncast<SVGElement>(oldAncestor))
      {
        oldParent->UpdateRelativeLengthsInformationForChild(false, *this);
      }
    }

    if (HasPendingResources())
    {
      TreeScopeForSVGReferences().RemoveElementFromPendingSVGResources(*this);
    }

    if (oldAncestor.IsConnected())
    {
      auto &extensions = NodeDocument().SVGExtensions();
      if (_svgRareData)
      {
        for (auto &element : CopyToRefs(_svgRareData->TakeReferencingElements()))
        {
          extensions.AddElementToRebuild(*element);
          element->ClearTarget();
        }
        krys_assert(_svgRareData->ReferencingElements().empty());
      }
      extensions.RemoveElementToRebuild(*this);
    }
    InvalidateInstances();

    // NOTE: every node removed with the subtree is told its subtree root's old parent.
    auto *oldShadowRoot = DynamicDowncast<dom::ShadowRoot>(oldAncestor);
    if (oldShadowRoot && oldShadowRoot->Mode() == dom::ShadowRootMode::UserAgent)
    {
      SetCorrespondingElement(nullptr);
    }
  }

  SVGSvgElement *SVGElement::OwnerSVGElement() const noexcept
  {
    if (IsOutermostSVGSVGElement())
    {
      return nullptr;
    }

    auto *node = ParentNode();
    while (node)
    {
      if (auto *svg = DynamicDowncastSVG<SVGSvgElement>(node))
      {
        return svg;
      }

      node = node->ParentOrShadowHostNode();
    }

    return nullptr;
  }

  SVGElement *SVGElement::ViewportElement(ViewportElementType type) const noexcept
  {
    // This function needs shadow tree support - as RenderSVGContainer uses this function
    // to determine the "overflow" property. <use> on <symbol> wouldn't work otherwise.
    auto *node = ParentNode();
    while (node)
    {
      if (DynamicDowncastSVG<SVGSvgElement>(node) || DynamicDowncastSVG<SVGImageElement>(node))
      {
        return DynamicDowncast<SVGElement>(node);
      }

      if (type == ViewportElementType::Any && DynamicDowncastSVG<SVGSymbolElement>(node))
      {
        return DynamicDowncast<SVGElement>(node);
      }

      node = node->ParentOrShadowHostNode();
    }

    return nullptr;
  }

  AffineTransform SVGElement::LocalCoordinateSpaceTransform(CTMScope) const noexcept
  {
    // To be overridden by SVGGraphicsElement (or as special case SVGTextElement and SVGPatternElement)
    return AffineTransform();
  }

  List<Ref<SVGElement>> SVGElement::Instances() const noexcept
  {
    if (!_svgRareData)
    {
      return {};
    }
    return CopyToRefs(_svgRareData->Instances());
  }

  List<Ref<SVGElement>> SVGElement::ReferencingElements() const noexcept
  {
    if (!_svgRareData)
    {
      return {};
    }
    return CopyToRefs(_svgRareData->ReferencingElements());
  }

  List<WeakPtr<SVGResourceElementClient>> SVGElement::ReferencingCSSClients() const noexcept
  {
    if (!_svgRareData)
    {
      return {};
    }
    return _svgRareData->ReferencingCSSClients();
  }

  void SVGElement::AddReferencingCSSClient(SVGResourceElementClient &client) noexcept
  {
    EnsureSVGRareData().AddReferencingCSSClient(client);
  }

  void SVGElement::RemoveReferencingCSSClient(SVGResourceElementClient &client) noexcept
  {
    if (!_svgRareData)
    {
      return;
    }
    EnsureSVGRareData().RemoveReferencingCSSClient(client);
  }

  void SVGElement::AddReferencingElement(SVGElement &element) noexcept
  {
    EnsureSVGRareData().AddReferencingElement(element);
    auto &rareDataOfReferencingElement = element.EnsureSVGRareData();
    krys_assert(!rareDataOfReferencingElement.ReferenceTarget());
    rareDataOfReferencingElement.SetReferenceTarget(CreateWeakPtr(this));
  }

  void SVGElement::RemoveReferencingElement(SVGElement &element) noexcept
  {
    EnsureSVGRareData().RemoveReferencingElement(element);
    element.EnsureSVGRareData().SetReferenceTarget(nullptr);
  }

  void SVGElement::RemoveElementReference() noexcept
  {
    if (!_svgRareData)
    {
      return;
    }
    if (auto *destination = _svgRareData->ReferenceTarget())
    {
      destination->RemoveReferencingElement(*this);
    }
  }

  SVGElement *SVGElement::CorrespondingElement() const noexcept
  {
    return _svgRareData ? _svgRareData->CorrespondingElement() : nullptr;
  }

  SVGUseElement *SVGElement::CorrespondingUseElement() const noexcept
  {
    auto *root = dom::TreeQueries::ContainingShadowRoot(*this);
    if (!root)
    {
      return nullptr;
    }
    if (root->Mode() != dom::ShadowRootMode::UserAgent)
    {
      return nullptr;
    }
    return DynamicDowncastSVG<SVGUseElement>(root->Host());
  }

  void SVGElement::SetCorrespondingElement(SVGElement *correspondingElement) noexcept
  {
    if (_svgRareData)
    {
      if (auto *oldCorrespondingElement = _svgRareData->CorrespondingElement())
      {
        oldCorrespondingElement->_svgRareData->RemoveInstance(*this);
      }
    }
    if (_svgRareData || correspondingElement)
    {
      EnsureSVGRareData().SetCorrespondingElement(correspondingElement);
    }
    if (correspondingElement)
    {
      correspondingElement->EnsureSVGRareData().AddInstance(*this);
    }
  }

  bool SVGElement::HaveLoadedRequiredResources() noexcept
  {
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      if (auto *svgChild = DynamicDowncast<SVGElement>(child);
          svgChild && !svgChild->HaveLoadedRequiredResources())
      {
        return false;
      }
    }
    return true;
  }

  void SVGElement::SendLoadEventIfPossible() noexcept
  {
    if (!IsConnected() || !NodeDocument().Frame())
    {
      return;
    }

    // NOTE: the event is dispatched whether or not anything listens for it.
    if (!HaveLoadedRequiredResources())
    {
      return;
    }

    auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Load), false, false);
    (void)DispatchEvent(*event);
  }

  void SVGElement::FinishParsingChildren() noexcept
  {
    StyledElement::FinishParsingChildren();

    if (IsOutermostSVGSVGElement())
    {
      return;
    }

    // Notify all the elements which have references to this element to rebuild their shadow and render
    // trees, e.g. a <use> element references a target element before this target element is defined.
    InvalidateInstances();
  }

  void SVGElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                      Maybe<dom::DOMStringView> value,
                                      dom::DOMStringAtom namespaceURI) noexcept
  {
    AttributeChanged(dom::QualifiedName {namespaceURI, dom::DOMStringAtom::Null(), localName}, oldValue,
                     value);
  }

  void SVGElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                    Maybe<dom::DOMStringView> value) noexcept
  {
    StyledElement::OnAttributeChanged(name.LocalName(), oldValue, value, name.NamespaceURI());

    if (name == SVGNames::IdAttr())
    {
      NodeDocument().SVGExtensions().RebuildAllElementReferencesForTarget(*this);
    }
    else if (name == SVGNames::ClassAttr())
    {
      _className->SetBaseValInternal(dom::DOMString {value.value_or(dom::DOMStringView {})});
    }

    // Changes to the style attribute are processed lazily (see Element::getAttribute() and related methods),
    // so we don't want changes to the style attribute to result in extra work here except
    // invalidateInstances().
    if (name == SVGNames::StyleAttr())
    {
      InvalidateInstances();
    }
    else
    {
      SVGAttributeChanged(name);
    }
  }

  bool SVGElement::IsAnimatedAttribute(const dom::QualifiedName &attributeName) const noexcept
  {
    return SVGPropertyAnimatorFactory::IsKnownAttribute(attributeName)
           || IsAnimatedPropertyAttribute(attributeName);
  }

  bool SVGElement::IsAnimatedStyleAttribute(const dom::QualifiedName &attributeName) const noexcept
  {
    return SVGPropertyAnimatorFactory::IsKnownAttribute(attributeName)
           || _propertyRegistry->IsAnimatedStylePropertyAttribute(attributeName);
  }

  RefPtr<SVGAttributeAnimator> SVGElement::CreateAnimator(const dom::QualifiedName &attributeName,
                                                          AnimationMode animationMode, CalcMode calcMode,
                                                          bool isAccumulated, bool isAdditive) noexcept
  {
    // Property animator, e.g. "fill" or "fill-opacity".
    if (auto animator = PropertyAnimatorFactory().CreateAnimator(attributeName, animationMode, calcMode,
                                                                 isAccumulated, isAdditive))
    {
      return animator;
    }

    // Animated property animator.
    RefPtr animator =
      _propertyRegistry->CreateAnimator(attributeName, animationMode, calcMode, isAccumulated, isAdditive);
    if (!animator)
    {
      return animator;
    }
    for (auto &instance : Instances())
    {
      instance->_propertyRegistry->AppendAnimatedInstance(attributeName, *animator);
    }
    return animator;
  }

  void SVGElement::AnimatorWillBeDeleted(const dom::QualifiedName &attributeName) noexcept
  {
    PropertyAnimatorFactory().AnimatorWillBeDeleted(attributeName);
  }

  css::MutableStyleProperties *SVGElement::AnimatedSMILStyleProperties() const noexcept
  {
    if (_svgRareData)
    {
      return _svgRareData->AnimatedSMILStyleProperties();
    }
    return nullptr;
  }

  css::MutableStyleProperties &SVGElement::EnsureAnimatedSMILStyleProperties() noexcept
  {
    return EnsureSVGRareData().EnsureAnimatedSMILStyleProperties();
  }

  void SVGElement::SetUseOverrideComputedStyle(bool value) noexcept
  {
    if (_svgRareData)
    {
      _svgRareData->SetUseOverrideComputedStyle(value);
    }
  }

  const RenderStyle *SVGElementRareData::OverrideComputedStyle(dom::Element &element,
                                                               const RenderStyle *parentStyle) noexcept
  {
    if (!_useOverrideComputedStyle)
    {
      return nullptr;
    }
    if (!_overrideComputedStyle || _needsOverrideComputedStyleUpdate)
    {
      // The style computed here contains no CSS Animations/Transitions or SMIL induced rules - this is needed
      // to compute the "base value" for the SMIL animation sandwhich model.
      _overrideComputedStyle =
        element.StyleResolver()
          .StyleForElement(element, {parentStyle}, RuleMatchingBehavior::MatchAllRulesExcludingSMIL)
          .Style;
      _needsOverrideComputedStyleUpdate = false;
    }
    krys_debug_assert(_overrideComputedStyle);
    return _overrideComputedStyle.get();
  }

  const RenderStyle *
    SVGElement::ComputedStyle(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    if (!_svgRareData || !_svgRareData->UseOverrideComputedStyle())
    {
      return Element::ComputedStyle(pseudoElementIdentifier);
    }

    const RenderStyle *parentStyle = nullptr;
    if (RefPtr parent = ShareRefPtr(dom::TreeQueries::ParentOrShadowHostElement(*this)))
    {
      if (auto *renderer = parent->Renderer())
      {
        parentStyle = &renderer->Style();
      }
    }

    return _svgRareData->OverrideComputedStyle(*this, parentStyle);
  }

  dom::QualifiedName SVGElement::AnimatableAttributeForName(dom::DOMStringView localName) noexcept
  {
    static const NeverDestroyed<HashMap<dom::DOMStringView, dom::QualifiedName>> animatableAttributes = []
    {
      const Array names {
        &SVGNames::ClassAttr(),
        &SVGNames::AmplitudeAttr(),
        &SVGNames::AzimuthAttr(),
        &SVGNames::BaseFrequencyAttr(),
        &SVGNames::BiasAttr(),
        &SVGNames::ClipPathUnitsAttr(),
        &SVGNames::CxAttr(),
        &SVGNames::CyAttr(),
        &SVGNames::DiffuseConstantAttr(),
        &SVGNames::DivisorAttr(),
        &SVGNames::DxAttr(),
        &SVGNames::DyAttr(),
        &SVGNames::EdgeModeAttr(),
        &SVGNames::ElevationAttr(),
        &SVGNames::ExponentAttr(),
        &SVGNames::FilterUnitsAttr(),
        &SVGNames::FxAttr(),
        &SVGNames::FyAttr(),
        &SVGNames::GradientTransformAttr(),
        &SVGNames::GradientUnitsAttr(),
        &SVGNames::HeightAttr(),
        &SVGNames::In2Attr(),
        &SVGNames::InAttr(),
        &SVGNames::InterceptAttr(),
        &SVGNames::K1Attr(),
        &SVGNames::K2Attr(),
        &SVGNames::K3Attr(),
        &SVGNames::K4Attr(),
        &SVGNames::KernelMatrixAttr(),
        &SVGNames::KernelUnitLengthAttr(),
        &SVGNames::LengthAdjustAttr(),
        &SVGNames::LimitingConeAngleAttr(),
        &SVGNames::MarkerHeightAttr(),
        &SVGNames::MarkerUnitsAttr(),
        &SVGNames::MarkerWidthAttr(),
        &SVGNames::MaskContentUnitsAttr(),
        &SVGNames::MaskUnitsAttr(),
        &SVGNames::MethodAttr(),
        &SVGNames::ModeAttr(),
        &SVGNames::NumOctavesAttr(),
        &SVGNames::OffsetAttr(),
        &SVGNames::OperatorAttr(),
        &SVGNames::OrderAttr(),
        &SVGNames::OrientAttr(),
        &SVGNames::PathLengthAttr(),
        &SVGNames::PatternContentUnitsAttr(),
        &SVGNames::PatternTransformAttr(),
        &SVGNames::PatternUnitsAttr(),
        &SVGNames::PointsAtXAttr(),
        &SVGNames::PointsAtYAttr(),
        &SVGNames::PointsAtZAttr(),
        &SVGNames::PreserveAlphaAttr(),
        &SVGNames::PreserveAspectRatioAttr(),
        &SVGNames::PrimitiveUnitsAttr(),
        &SVGNames::RadiusAttr(),
        &SVGNames::RAttr(),
        &SVGNames::RefXAttr(),
        &SVGNames::RefYAttr(),
        &SVGNames::ResultAttr(),
        &SVGNames::RotateAttr(),
        &SVGNames::RxAttr(),
        &SVGNames::RyAttr(),
        &SVGNames::ScaleAttr(),
        &SVGNames::SeedAttr(),
        &SVGNames::SlopeAttr(),
        &SVGNames::SpacingAttr(),
        &SVGNames::SpecularConstantAttr(),
        &SVGNames::SpecularExponentAttr(),
        &SVGNames::SpreadMethodAttr(),
        &SVGNames::StartOffsetAttr(),
        &SVGNames::StdDeviationAttr(),
        &SVGNames::StitchTilesAttr(),
        &SVGNames::SurfaceScaleAttr(),
        &SVGNames::TableValuesAttr(),
        &SVGNames::TargetAttr(),
        &SVGNames::TargetXAttr(),
        &SVGNames::TargetYAttr(),
        &SVGNames::TransformAttr(),
        &SVGNames::TypeAttr(),
        &SVGNames::ValuesAttr(),
        &SVGNames::ViewBoxAttr(),
        &SVGNames::WidthAttr(),
        &SVGNames::X1Attr(),
        &SVGNames::X2Attr(),
        &SVGNames::XAttr(),
        &SVGNames::XChannelSelectorAttr(),
        &SVGNames::Y1Attr(),
        &SVGNames::Y2Attr(),
        &SVGNames::YAttr(),
        &SVGNames::YChannelSelectorAttr(),
        &SVGNames::ZAttr(),
        &SVGNames::HrefAttr(),
      };
      HashMap<dom::DOMStringView, dom::QualifiedName> map;
      for (const auto *name : names)
      {
        KRYS_MAYBE_UNUSED auto [iterator, isNewEntry] = map.try_emplace(name->LocalName().View(), *name);
        krys_debug_assert(isNewEntry);
      }
      return map;
    }();
    auto iterator = animatableAttributes->find(localName);
    return iterator != animatableAttributes->end() ? iterator->second : dom::NullQualifiedName();
  }

#if KRYS_DEBUG
  bool SVGElement::IsAnimatableAttribute(const dom::QualifiedName &name) const noexcept
  {
    if (AnimatableAttributeForName(name.LocalName().View()) == name)
    {
      return !FilterOutAnimatableAttribute(name);
    }
    return false;
  }

  bool SVGElement::FilterOutAnimatableAttribute(const dom::QualifiedName &) const noexcept
  {
    return false;
  }
#endif

  void SVGElement::SynchronizeSVGAttribute(const dom::QualifiedName &name) noexcept
  {
    // If the value of the property has changed, serialize the new value to the attribute.
    if (auto value = _propertyRegistry->Synchronize(name))
    {
      SetSynchronizedLazyAttribute(name, krys::move(*value));
    }
  }

  void SVGElement::SynchronizeAllSVGAttributes() noexcept
  {
    // SVGPropertyRegistry::synchronizeAllAttributes() returns the new values of
    // the properties which have changed but not committed yet.
    for (auto &[name, value] : _propertyRegistry->SynchronizeAllAttributes())
    {
      SetSynchronizedLazyAttribute(name, krys::move(value));
    }
  }

  void SVGElement::CommitPropertyChange(SVGProperty *property) noexcept
  {
    // We want to dirty the top-level property when a descendant changes. For example
    // a change in an SVGLength item in SVGLengthList should set the dirty flag on
    // SVGLengthList and not the SVGLength.
    property->SetDirty();

    SetAnimatedSVGAttributesAreDirty();
    SVGAttributeChanged(_propertyRegistry->PropertyAttributeName(*property));
  }

  void SVGElement::CommitPropertyChange(SVGAnimatedProperty &animatedProperty) noexcept
  {
    auto attributeName = _propertyRegistry->AnimatedPropertyAttributeName(animatedProperty);
    krys_assert(attributeName != dom::NullQualifiedName());

    // A change in a style property, e.g SVGRectElement::x should be serialized to
    // the attribute immediately. Otherwise it is okay to be lazy in this regard.
    if (!_propertyRegistry->IsAnimatedStylePropertyAttribute(attributeName))
    {
      _propertyRegistry->SetAnimatedPropertyDirty(attributeName, animatedProperty);
    }
    else
    {
      SetSynchronizedLazyAttribute(attributeName, animatedProperty.BaseValAsString());
    }

    SetAnimatedSVGAttributesAreDirty();
    SVGAttributeChanged(attributeName);
  }

  bool SVGElement::IsAnimatedPropertyAttribute(const dom::QualifiedName &attributeName) const noexcept
  {
    return _propertyRegistry->IsAnimatedPropertyAttribute(attributeName);
  }

  void SVGElement::DetachAllProperties() noexcept
  {
    _propertyRegistry->DetachAllProperties();
  }

  void SVGElement::SetAnimatedSVGAttributesAreDirty() noexcept
  {
    SetNodeFlag(dom::NodeFlags::AnimatedSVGAttributesAreDirty);
  }

  void SVGElement::SetPresentationalHintStyleIsDirty() noexcept
  {
    InvalidatePresentationalHintStyle();
    InvalidateStyle();
  }

  dom::DOMStringAtom SVGElement::ClassName() const noexcept
  {
    return dom::DOMStringAtom {_className->CurrentValue()};
  }

  bool SVGElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    auto *svgChild = DynamicDowncast<SVGElement>(child);
    if (!svgChild)
    {
      return false;
    }

    switch (svgChild->TagQualifiedName().TagName())
    {
      case html::TagName::textPath:
      case html::TagName::tspan:
      {
        return false;
      }
      default:
      {
        break;
      }
    }
    return svgChild->IsValid();
  }

  bool SVGElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    // http://www.w3.org/TR/SVG/extend.html#PrivateData
    // Prevent anything other than SVG renderers from appearing in our render tree
    // Spec: SVG allows inclusion of elements from foreign namespaces anywhere
    // with the SVG content. In general, the SVG user agent will include the unknown
    // elements in the DOM but will otherwise ignore unknown elements.
    auto *parent = dom::TreeQueries::ParentOrShadowHostElement(*this);
    if (!parent || Is<SVGElement>(*parent))
    {
      return dom::StyledElement::RendererIsNeeded(style);
    }

    return false;
  }

  void SVGElement::UpdateSVGRendererForElementChange() noexcept
  {
    NodeDocument().UpdateSVGRenderer(*this);
  }

  Maybe<FloatRect> SVGElement::GetBoundingBox() const noexcept
  {
    if (Is<SVGGraphicsElement>(*this))
    {
      if (auto *renderer = Renderer())
      {
        return renderer->ObjectBoundingBox();
      }
    }
    return null;
  }

  bool SVGElement::HasAssociatedSVGLayoutBox() const noexcept
  {
    if (!Renderer())
    {
      return false;
    }

    // Legacy SVG engine specific condition.
    if (Renderer()->IsLegacyRenderSVGRoot())
    {
      return false;
    }

    return true;
  }

  void SVGElement::SVGAttributeChanged(const dom::QualifiedName &attrName) noexcept
  {
    auto propId = CssPropertyIdForSVGAttributeName(attrName.LocalName(), attrName.NamespaceURI());
    if (propId != css::PropertyId::Invalid)
    {
      InvalidateInstances();
      return;
    }

    if (attrName == SVGNames::ClassAttr())
    {
      // NOTE: the element's classes are read from its class attribute, so a change to className through the
      // DOM is written to the attribute now, whose change steps invalidate the style.
      if (auto value = _propertyRegistry->Synchronize(attrName))
      {
        (void)SetAttribute(attrName.LocalName(), krys::move(*value));
      }
      InvalidateInstances();
      return;
    }

    if (attrName == SVGNames::IdAttr())
    {
      // Notify resources about id changes, this is important as we cache resources by id in
      // SVGDocumentExtensions
      if (auto *container = DynamicDowncast<LegacyRenderSVGResourceContainer>(Renderer()))
      {
        container->IdChanged();
      }
      if (IsConnected())
      {
        BuildPendingResourcesIfNeeded();
      }
      InvalidateInstances();
      return;
    }
  }

  void SVGElement::OnInsert() noexcept
  {
    StyledElement::OnInsert();

    // NOTE: a node's insertion steps don't say whether it's the root of the inserted tree; telling the parent
    // again about a child that was inserted with it changes nothing.
    if (!_hasInitializedRelativeLengthsState)
    {
      UpdateRelativeLengthsInformation();
    }
    else if (auto *parentElement = DynamicDowncast<SVGElement>(ParentNode()))
    {
      parentElement->UpdateRelativeLengthsInformationForChild(HasRelativeLengths(), *this);
    }

    // NOTE: the pending resources that wait for the element are built in its post-connection steps.
  }

  void SVGElement::OnPostConnection() noexcept
  {
    StyledElement::OnPostConnection();
    BuildPendingResourcesIfNeeded();
  }

  void SVGElement::BuildPendingResourcesIfNeeded() noexcept
  {
    if (!NeedsPendingResourceHandling() || !IsConnected() || IsInShadowTree())
    {
      return;
    }

    auto &treeScope = TreeScopeForSVGReferences();
    auto resourceId = dom::DOMStringAtom(Id());
    if (!treeScope.IsIdOfPendingSVGResource(resourceId))
    {
      return;
    }

    treeScope.MarkPendingSVGResourcesForRemoval(resourceId);

    // Rebuild pending resources for each client of a pending resource that is being removed.
    while (RefPtr<SVGElement> clientElement =
             treeScope.TakeElementFromPendingSVGResourcesForRemovalMap(resourceId))
    {
      krys_debug_assert(clientElement->HasPendingResources());
      if (clientElement->HasPendingResources())
      {
        clientElement->BuildPendingResource();
        if (auto *renderer = clientElement->Renderer())
        {
          for (auto &ancestor : AncestorsOfType<LegacyRenderSVGResourceContainer>(*renderer))
          {
            ancestor.MarkAllClientsForRepaint();
          }
        }
        treeScope.ClearHasPendingSVGResourcesIfPossible(*clientElement);
      }
    }
  }

  Maybe<style::UnadjustedStyle>
    SVGElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                   const RenderStyle *) noexcept
  {
    // If the element is in a <use> tree we get the style from the definition tree.
    if (RefPtr<SVGElement> styleElement = ShareRefPtr(CorrespondingElement()))
    {
      auto styleElementResolutionContext = resolutionContext;
      // Can't use the state since we are going to another part of the tree.
      styleElementResolutionContext.SelectorMatchingState = nullptr;
      styleElementResolutionContext.IsSVGUseTreeRoot = true;
      return styleElement->ResolveStyle(styleElementResolutionContext);
    }

    return ResolveStyle(resolutionContext);
  }

  void SVGElement::DidAddEventListener(const dom::EventListener &listener) noexcept
  {
    StyledElement::DidAddEventListener(listener);

    if (dom::TreeQueries::ContainingShadowRoot(*this))
    {
      return;
    }

    // Add event listener to all shadow tree DOM element instances
    krys_debug_assert(!InstanceUpdatesBlocked());
    for (auto &instance : Instances())
    {
      krys_debug_assert(instance->CorrespondingElement() == this);
      krys_debug_assert(instance->IsInUserAgentShadowTree());
      instance->AddEventListener(
        listener.Type(), ShareRef(*listener.Callback()),
        dom::AddEventListenerOptions {
          {listener.Capture()}, listener.Once(), listener.Passive(), ShareRefPtr(listener.Signal())});
    }
  }

  void SVGElement::DidRemoveEventListener(const dom::EventListener &listener) noexcept
  {
    StyledElement::DidRemoveEventListener(listener);

    if (dom::TreeQueries::ContainingShadowRoot(*this))
    {
      return;
    }

    // Remove event listener from all shadow tree DOM element instances
    krys_debug_assert(!InstanceUpdatesBlocked());
    for (auto &instance : Instances())
    {
      krys_debug_assert(instance->CorrespondingElement() == this);
      krys_debug_assert(instance->IsInUserAgentShadowTree());
      instance->RemoveEventListener(listener.Type(), ShareRef(*listener.Callback()), listener.Capture());
    }
  }

  void SVGElement::OnChildrenChanged() noexcept
  {
    StyledElement::OnChildrenChanged();

    if (HasNodeFlag(dom::NodeFlags::IsParsingChildren))
    {
      return;
    }
    InvalidateInstances();
  }

  bool SVGElement::InstanceUpdatesBlocked() const noexcept
  {
    return _svgRareData && _svgRareData->InstanceUpdatesBlocked();
  }

  void SVGElement::SetInstanceUpdatesBlocked(bool value) noexcept
  {
    // Catch any callers that calls setInstanceUpdatesBlocked(true) twice in a row.
    // That probably indicates nested use of InstanceUpdateBlocker and a bug.
    krys_assert(!value || !InstanceUpdatesBlocked());

    if (_svgRareData)
    {
      _svgRareData->SetInstanceUpdatesBlocked(value);
    }
  }

  bool SVGElement::HasRelativeLengths() const noexcept
  {
    return _selfHasRelativeLengths
           || std::ranges::any_of(_childElementsWithRelativeLengths,
                                  [](const WeakPtr<SVGElement> &element) { return !!element; });
  }

  void SVGElement::UpdateRelativeLengthsInformation() noexcept
  {
    _hasInitializedRelativeLengthsState = true;
    _selfHasRelativeLengths = SelfHasRelativeLengths();
    if (auto *parent = DynamicDowncast<SVGElement>(ParentNode()))
    {
      parent->UpdateRelativeLengthsInformationForChild(HasRelativeLengths(), *this);
    }
  }

  void SVGElement::UpdateRelativeLengthsInformationForChild(bool hasRelativeLengths,
                                                            SVGElement &element) noexcept
  {
    krys_assert(element.ParentNode() == this || (!hasRelativeLengths && !element.ParentNode()));
    bool notifyParent = false;
    std::erase_if(_childElementsWithRelativeLengths,
                  [&](const WeakPtr<SVGElement> &child) { return !child || child.get() == &element; });
    if (hasRelativeLengths)
    {
      _childElementsWithRelativeLengths.push_back(CreateWeakPtr(&element));
      notifyParent = !_hasRegisteredWithParentForRelativeLengths;
    }
    else
    {
      notifyParent = _childElementsWithRelativeLengths.empty() && _hasRegisteredWithParentForRelativeLengths;
    }

    if (!notifyParent)
    {
      return;
    }

    if (auto *parent = DynamicDowncast<SVGElement>(ParentNode()))
    {
      _hasRegisteredWithParentForRelativeLengths = hasRelativeLengths;
      parent->UpdateRelativeLengthsInformationForChild(hasRelativeLengths, *this);
    }
  }

  void SVGElement::InvalidateInstances() noexcept
  {
    if (InstanceUpdatesBlocked())
    {
      return;
    }

    for (auto &instance : Instances())
    {
      if (RefPtr<SVGUseElement> useElement = ShareRefPtr(instance->CorrespondingUseElement()))
      {
        useElement->InvalidateShadowTree();
      }
      instance->SetCorrespondingElement(nullptr);
    }
  }

  SVGConditionalProcessingAttributes &SVGElement::ConditionalProcessingAttributes() noexcept
  {
    return EnsureSVGRareData().ConditionalProcessingAttributes(*this);
  }

  SVGConditionalProcessingAttributes *SVGElement::ConditionalProcessingAttributesIfExists() const noexcept
  {
    if (!_svgRareData)
    {
      return nullptr;
    }
    return _svgRareData->ConditionalProcessingAttributesIfExists();
  }

#pragma region Presentational hints

  css::PropertyId SVGElement::CssPropertyIdForSVGAttributeName(dom::DOMStringAtom localName,
                                                               dom::DOMStringAtom namespaceURI) noexcept
  {
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return css::PropertyId::Invalid;
    }

    // FIXME: `d` maps to the d property (as a `path()` value), once there's SVGPathElement.
    static const HashMap<dom::DOMStringView, css::PropertyId> map {
      {u8"alignment-baseline", css::PropertyId::AlignmentBaseline},
      {u8"baseline-shift", css::PropertyId::BaselineShift},
      {u8"buffered-rendering", css::PropertyId::BufferedRendering},
      {u8"clip", css::PropertyId::Clip},
      {u8"clip-path", css::PropertyId::ClipPath},
      {u8"clip-rule", css::PropertyId::ClipRule},
      {u8"color", css::PropertyId::Color},
      {u8"color-interpolation", css::PropertyId::ColorInterpolation},
      {u8"color-interpolation-filters", css::PropertyId::ColorInterpolationFilters},
      {u8"cursor", css::PropertyId::Cursor},
      {u8"cx", css::PropertyId::Cx},
      {u8"cy", css::PropertyId::Cy},
      {u8"d", css::PropertyId::D},
      {u8"direction", css::PropertyId::Direction},
      {u8"display", css::PropertyId::Display},
      {u8"dominant-baseline", css::PropertyId::DominantBaseline},
      {u8"fill", css::PropertyId::Fill},
      {u8"fill-opacity", css::PropertyId::FillOpacity},
      {u8"fill-rule", css::PropertyId::FillRule},
      {u8"filter", css::PropertyId::Filter},
      {u8"flood-color", css::PropertyId::FloodColor},
      {u8"flood-opacity", css::PropertyId::FloodOpacity},
      {u8"font-family", css::PropertyId::FontFamily},
      {u8"font-size", css::PropertyId::FontSize},
      {u8"font-size-adjust", css::PropertyId::FontSizeAdjust},
      {u8"font-stretch", css::PropertyId::FontWidth},
      {u8"font-width", css::PropertyId::FontWidth},
      {u8"font-style", css::PropertyId::FontStyle},
      {u8"font-variant", css::PropertyId::FontVariant},
      {u8"font-weight", css::PropertyId::FontWeight},
      {u8"glyph-orientation-horizontal", css::PropertyId::GlyphOrientationHorizontal},
      {u8"glyph-orientation-vertical", css::PropertyId::GlyphOrientationVertical},
      {u8"image-rendering", css::PropertyId::ImageRendering},
      {u8"height", css::PropertyId::Height},
      {u8"letter-spacing", css::PropertyId::LetterSpacing},
      {u8"lighting-color", css::PropertyId::LightingColor},
      {u8"marker-end", css::PropertyId::MarkerEnd},
      {u8"marker-mid", css::PropertyId::MarkerMid},
      {u8"marker-start", css::PropertyId::MarkerStart},
      {u8"mask", css::PropertyId::Mask},
      {u8"mask-type", css::PropertyId::MaskType},
      {u8"opacity", css::PropertyId::Opacity},
      {u8"overflow", css::PropertyId::Overflow},
      {u8"paint-order", css::PropertyId::PaintOrder},
      {u8"pointer-events", css::PropertyId::PointerEvents},
      {u8"r", css::PropertyId::R},
      {u8"rx", css::PropertyId::Rx},
      {u8"ry", css::PropertyId::Ry},
      {u8"shape-rendering", css::PropertyId::ShapeRendering},
      {u8"stop-color", css::PropertyId::StopColor},
      {u8"stop-opacity", css::PropertyId::StopOpacity},
      {u8"stroke", css::PropertyId::Stroke},
      {u8"stroke-dasharray", css::PropertyId::StrokeDasharray},
      {u8"stroke-dashoffset", css::PropertyId::StrokeDashoffset},
      {u8"stroke-linecap", css::PropertyId::StrokeLinecap},
      {u8"stroke-linejoin", css::PropertyId::StrokeLinejoin},
      {u8"stroke-miterlimit", css::PropertyId::StrokeMiterlimit},
      {u8"stroke-opacity", css::PropertyId::StrokeOpacity},
      {u8"stroke-width", css::PropertyId::StrokeWidth},
      {u8"text-anchor", css::PropertyId::TextAnchor},
      {u8"text-decoration", css::PropertyId::TextDecoration},
      {u8"text-rendering", css::PropertyId::TextRendering},
      {u8"unicode-bidi", css::PropertyId::UnicodeBidi},
      {u8"vector-effect", css::PropertyId::VectorEffect},
      {u8"visibility", css::PropertyId::Visibility},
      {u8"width", css::PropertyId::Width},
      {u8"word-spacing", css::PropertyId::WordSpacing},
      {u8"writing-mode", css::PropertyId::WritingMode},
      {u8"x", css::PropertyId::X},
      {u8"y", css::PropertyId::Y},
      {u8"transform-origin", css::PropertyId::TransformOrigin},
    };

    auto it = map.find(localName.View());
    return it == map.end() ? css::PropertyId::Invalid : it->second;
  }

  bool SVGElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                      dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (CssPropertyIdForSVGAttributeName(localName, namespaceURI) != css::PropertyId::Invalid)
    {
      return true;
    }
    return StyledElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void SVGElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                          dom::DOMStringAtom namespaceURI,
                                                          dom::DOMStringView value,
                                                          css::MutableStyleProperties &style) noexcept
  {
    auto property = CssPropertyIdForSVGAttributeName(localName, namespaceURI);
    if (property != css::PropertyId::Invalid)
    {
      AddPropertyToPresentationalHintStyle(style, property, css::CSSOMString(value));
    }
  }

#pragma endregion
}
