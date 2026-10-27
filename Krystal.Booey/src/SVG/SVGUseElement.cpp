#include "Krystal.Booey/SVG/SVGUseElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGSymbolElement.hpp"

namespace krys::boo::svg
{
  namespace
  {
    template <typename T>
    KRYS_NODISCARD T *DynamicDowncastSVG(dom::Node *node) noexcept
    {
      auto *svgElement = DynamicDowncast<SVGElement>(node);
      if constexpr (SameType<T, SVGElement>)
      {
        return svgElement;
      }
      else
      {
        return svgElement ? DynamicDowncast<T>(*svgElement) : nullptr;
      }
    }

    // Makes the SVG element @p localName, whose tag is @p tagName, in @p document.
    KRYS_NODISCARD Ref<SVGElement> CreateSVGElement(dom::Document &document, utf8_stringview localName,
                                                    html::TagName tagName) noexcept
    {
      auto element = dom::ElementFactory::Create(
        document, dom::QualifiedName(infra::Namespaces::SVG, dom::DOMStringAtom::Null(),
                                     dom::DOMStringAtom(localName), tagName, html::Namespace::SVG));
      return ShareRef(Downcast<SVGElement>(*element));
    }

    // Calls @p function with each descendant of @p root that is a @p T, in tree order; the function returns
    // the node to carry on after (the descendant itself, or the node that replaced it).
    template <typename T, typename TFunction>
    void ForEachDescendantOfType(dom::ContainerNode &root, TFunction &&function) noexcept
    {
      auto *node = root.FirstChild();
      while (node)
      {
        if (auto *element = DynamicDowncastSVG<T>(node))
        {
          node = &function(*element);
        }
        node = dom::TreeQueries::Next(*node, &root);
      }
    }
  }

  SVGUseElement::SVGUseElement(dom::Document &document) noexcept
      : SVGGraphicsElement(document, CreateUnique<PropertyRegistry>(*this), SVGElementInterface::Use),
        SVGURIReference(this), _x(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _y(SVGAnimatedLength::Create(this, SVGLengthMode::Height)),
        _width(SVGAnimatedLength::Create(this, SVGLengthMode::Width)),
        _height(SVGAnimatedLength::Create(this, SVGLengthMode::Height))
  {
    krys_debug_assert(HasCustomStyleResolveCallbacks());

    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGUseElement::_x>(SVGNames::XAttr());
      PropertyRegistry::RegisterProperty<&SVGUseElement::_y>(SVGNames::YAttr());
      PropertyRegistry::RegisterProperty<&SVGUseElement::_width>(SVGNames::WidthAttr());
      PropertyRegistry::RegisterProperty<&SVGUseElement::_height>(SVGNames::HeightAttr());
    }
  }

  void SVGUseElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                       Maybe<dom::DOMStringView> value) noexcept
  {
    auto parseError = SVGParsingError::None;
    auto string = value.value_or(dom::DOMStringView {});

    if (name == SVGNames::XAttr())
    {
      _x->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError));
    }
    else if (name == SVGNames::YAttr())
    {
      _y->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError));
    }
    else if (name == SVGNames::WidthAttr())
    {
      _width->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Width, string, parseError,
                                                           SVGLengthNegativeValuesMode::Forbid));
    }
    else if (name == SVGNames::HeightAttr())
    {
      _height->SetBaseValInternal(SVGLengthValue::Construct(SVGLengthMode::Height, string, parseError,
                                                            SVGLengthNegativeValuesMode::Forbid));
    }
    ReportAttributeParsingError(parseError, name, string);

    SVGURIReference::ParseAttribute(name, value);
    SVGGraphicsElement::AttributeChanged(name, oldValue, value);
  }

  void SVGUseElement::OnInsert() noexcept
  {
    SVGGraphicsElement::OnInsert();
    if (IsConnected())
    {
      if (_shadowTreeNeedsUpdate)
      {
        NodeDocument().AddElementWithPendingUserAgentShadowTreeUpdate(*this);
      }
      InvalidateShadowTree();
    }
  }

  void SVGUseElement::OnPostConnection() noexcept
  {
    SVGGraphicsElement::OnPostConnection();
    UpdateExternalDocument();
  }

  void SVGUseElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    // Check m_shadowTreeNeedsUpdate before calling SVGElement::removedFromAncestor which calls
    // SVGElement::invalidateInstances and SVGUseElement::updateExternalDocument which calls
    // invalidateShadowTree().
    bool disconnectedFromDocument = oldAncestor.IsConnected() && !IsConnected();
    if (disconnectedFromDocument && _shadowTreeNeedsUpdate)
    {
      NodeDocument().RemoveElementWithPendingUserAgentShadowTreeUpdate(*this);
    }
    SVGGraphicsElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (disconnectedFromDocument)
    {
      ClearShadowTree();
      UpdateExternalDocument();
    }
  }

  FloatRect SVGUseElement::GetBBox(StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    auto bbox = SVGGraphicsElement::GetBBox(styleUpdateStrategy);
    if (bbox.IsEmpty())
    {
      return {};
    }

    auto *transformableContainer = DynamicDowncast<LegacyRenderSVGTransformableContainer>(Renderer());
    krys_debug_assert(transformableContainer && transformableContainer->IsObjectBoundingBoxValid());
    bbox.Move(transformableContainer->AdditionalTranslation());
    return bbox;
  }

  void SVGUseElement::TransferSizeAttributesToTargetClone(SVGElement &shadowElement) const noexcept
  {
    // Use |shadowElement| for checking the element type, because we will
    // have replaced a <symbol> with an <svg> in the instance tree.
    if (!Is<SVGSymbolElement>(shadowElement) && !Is<SVGSvgElement>(shadowElement))
    {
      return;
    }

    // "The width and height properties on the 'use' element override the values
    // for the corresponding properties on a referenced 'svg' or 'symbol' element
    // when determining the used value for that property on the instance root
    // element. However, if the computed value for the property on the 'use'
    // element is auto, then the property is computed as normal for the element
    // instance. ... Because auto is the initial value, if dimensions are not
    // explicitly set on the 'use' element, the values set on the 'svg' or
    // 'symbol' will be used as defaults."
    // https://svgwg.org/svg2-draft/struct.html#UseElement

    RefPtr<SVGElement> correspondingElement = ShareRefPtr(shadowElement.CorrespondingElement());
    auto transfer = [&](const SVGLengthValue &length, const dom::QualifiedName &name)
    {
      Maybe<dom::DOMString> value;
      if (length.ValueInSpecifiedUnits())
      {
        value = length.ValueAsString();
      }
      else if (correspondingElement)
      {
        value = correspondingElement->GetAttribute(name.LocalName());
      }

      if (value)
      {
        (void)shadowElement.SetAttribute(name.LocalName(), krys::move(*value));
      }
      else
      {
        shadowElement.RemoveAttribute(name.LocalName());
      }
    };
    transfer(Width(), SVGNames::WidthAttr());
    transfer(Height(), SVGNames::HeightAttr());
  }

  void SVGUseElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    InstanceInvalidationGuard guard(*this);

    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      UpdateRelativeLengthsInformation();
      if (attributeName == SVGNames::WidthAttr() || attributeName == SVGNames::HeightAttr())
      {
        // FIXME: It's unnecessarily inefficient to update both width and height each time either is changed.
        if (auto targetClone = TargetClone())
        {
          TransferSizeAttributesToTargetClone(*targetClone);
        }
      }
      UpdateSVGRendererForElementChange();
      return;
    }

    if (SVGURIReference::IsKnownAttribute(attributeName))
    {
      UpdateExternalDocument();
      InvalidateShadowTree();
      return;
    }

    SVGGraphicsElement::SVGAttributeChanged(attributeName);
  }

  namespace
  {
    KRYS_NODISCARD bool IsDisallowedElement(const SVGElement &element) noexcept
    {
      using html::TagName;

      // Spec: "Any 'svg', 'symbol', 'g', graphics element or other 'use' is potentially a template object
      // that can be re-used (i.e., "instanced") in the SVG document via a 'use' element." "Graphics Element"
      // is defined as 'circle', 'ellipse', 'image', 'line', 'path', 'polygon', 'polyline', 'rect', 'text'
      // Excluded are anything that is used by reference or that only make sense to appear once in a document.
      switch (element.TagQualifiedName().TagName())
      {
        case TagName::a:
        case TagName::circle:
        case TagName::desc:
        case TagName::ellipse:
        case TagName::g:
        case TagName::image:
        case TagName::line:
        case TagName::metadata:
        case TagName::path:
        case TagName::polygon:
        case TagName::polyline:
        case TagName::rect:
        case TagName::svg:
        case TagName::switch_:
        case TagName::symbol:
        case TagName::text:
        case TagName::textPath:
        case TagName::title:
        case TagName::tspan:
        case TagName::use:
        {
          return false;
        }
        default:
        {
          break;
        }
      }
      return true;
    }

    KRYS_NODISCARD bool IsDisallowedElement(const dom::Element &element) noexcept
    {
      auto *svgElement = DynamicDowncast<SVGElement>(element);
      return !svgElement || IsDisallowedElement(*svgElement);
    }
  }

  void SVGUseElement::ClearShadowTree() noexcept
  {
    if (RefPtr<dom::ShadowRoot> root = ShareRefPtr(UserAgentShadowRoot()))
    {
      (void)dom::MutationAlgorithms::ReplaceAll(nullptr, *root);
    }
  }

  void SVGUseElement::BuildPendingResource() noexcept
  {
    InvalidateShadowTree();
  }

  void SVGUseElement::UpdateUserAgentShadowTree() noexcept
  {
    _shadowTreeNeedsUpdate = false;

    // FIXME: It's expensive to re-clone the entire tree every time. We should find a more efficient way to
    // handle this.
    ClearShadowTree();

    if (!IsConnected())
    {
      return;
    }
    NodeDocument().RemoveElementWithPendingUserAgentShadowTreeUpdate(*this);

    auto targetID = dom::DOMStringAtom::Null();
    RefPtr<SVGElement> target = FindTarget(&targetID);
    if (!target)
    {
      TreeScopeForSVGReferences().AddPendingSVGResource(targetID, *this);
      return;
    }

    krys_assert(!dom::TreeQueries::IsDescendant(*this, *target));
    {
      Ref<dom::ShadowRoot> shadowRoot = ShareRef(EnsureUserAgentShadowRoot());
      CloneTarget(*shadowRoot, *target);
      ExpandUseElementsInShadowTree();
      ExpandSymbolElementsInShadowTree();
      UpdateRelativeLengthsInformation();
    }

    TransferEventListenersToShadowTree();

    // When we invalidate the other shadow trees, it's important that we don't
    // follow any cycles and invalidate ourselves. To avoid that, we temporarily
    // set m_shadowTreeNeedsUpdate to true so invalidateShadowTree will
    // quickly return and do nothing.
    krys_debug_assert(!_shadowTreeNeedsUpdate);
    _shadowTreeNeedsUpdate = true;
    InvalidateDependentShadowTrees();
    _shadowTreeNeedsUpdate = false;
  }

  RefPtr<SVGElement> SVGUseElement::TargetClone() const noexcept
  {
    auto *root = UserAgentShadowRoot();
    return root ? ShareRefPtr(DynamicDowncast<SVGElement>(root->FirstChild())) : nullptr;
  }

  RenderPtr<RenderElement> SVGUseElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGTransformableContainer>(*this, krys::move(style));
  }

  namespace
  {
    KRYS_NODISCARD bool IsDirectReference(const SVGElement &element) noexcept
    {
      using html::TagName;
      switch (element.TagQualifiedName().TagName())
      {
        case TagName::circle:
        case TagName::ellipse:
        case TagName::path:
        case TagName::polygon:
        case TagName::polyline:
        case TagName::rect:
        case TagName::text:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }
  }

  RefPtr<SVGElement> SVGUseElement::ClipChild() const noexcept
  {
    auto targetClone = TargetClone();
    if (!targetClone || !IsDirectReference(*targetClone))
    {
      return nullptr;
    }
    return targetClone;
  }

  RenderElement *SVGUseElement::RendererClipChild() const noexcept
  {
    auto targetClone = TargetClone();
    if (!targetClone)
    {
      return nullptr;
    }
    if (!IsDirectReference(*targetClone))
    {
      return nullptr;
    }
    return targetClone->Renderer();
  }

  namespace
  {
    void DisassociateAndRemoveClones(const List<Ref<dom::Element>> &clones) noexcept
    {
      for (auto &clone : clones)
      {
        for (auto *node = clone->FirstChild(); node; node = dom::TreeQueries::Next(*node, clone.get()))
        {
          if (auto *descendant = DynamicDowncast<SVGElement>(node))
          {
            descendant->SetCorrespondingElement(nullptr);
          }
        }
        if (auto *svgClone = DynamicDowncast<SVGElement>(*clone))
        {
          svgClone->SetCorrespondingElement(nullptr);
        }
        (void)dom::MutationAlgorithms::Remove(*clone);
      }
    }

    // The elements of @p subtree that @p predicate matches, not looking inside them.
    template <typename TPredicate>
    KRYS_NODISCARD List<Ref<dom::Element>> MatchingElementsInSubtree(SVGElement &subtree,
                                                                     TPredicate &&predicate) noexcept
    {
      List<Ref<dom::Element>> elements;
      auto *node = subtree.FirstChild();
      while (node)
      {
        auto *element = DynamicDowncast<dom::Element>(node);
        if (element && predicate(*element))
        {
          elements.push_back(ShareRef(*element));
          node = dom::TreeQueries::NextSkippingChildren(*node, &subtree);
          continue;
        }
        node = dom::TreeQueries::Next(*node, &subtree);
      }
      return elements;
    }

    void RemoveDisallowedElementsFromSubtree(SVGElement &subtree) noexcept
    {
      // Remove disallowed elements after the fact rather than not cloning them in the first place.
      // This optimizes for the normal case where none of those elements are present.

      // This function is used only on elements in subtrees that are not yet in documents, so
      // mutation events are not a factor; there are no event listeners to handle those events.
      // Assert that it's not in a document to make sure callers are still using it this way.
      krys_debug_assert(!subtree.IsConnected());

      DisassociateAndRemoveClones(MatchingElementsInSubtree(subtree, [](const dom::Element &element)
                                                            { return IsDisallowedElement(element); }));
    }

    void RemoveSymbolElementsFromSubtree(SVGElement &subtree) noexcept
    {
      // Symbol elements inside the subtree should not be cloned for two reasons: 1) They are invisible and
      // don't need to be cloned to get correct rendering. 2) expandSymbolElementsInShadowTree will turn them
      // into <svg> elements, which is correct for symbol elements directly referenced by use elements,
      // but incorrect for ones that just happen to be in a subtree.
      DisassociateAndRemoveClones(MatchingElementsInSubtree(
        subtree, [](const dom::Element &element)
        { return DynamicDowncastSVG<SVGSymbolElement>(const_cast<dom::Element *>(&element)) != nullptr; }));
    }

    // Calls @p function with each SVG element of @p clone and the element of @p original in the same place,
    // which must have the same shape.
    template <typename TFunction>
    void ForEachDescendantPair(SVGElement &clone, SVGElement &original, TFunction &&function) noexcept
    {
      dom::Node *cloneNode = clone.FirstChild();
      dom::Node *originalNode = original.FirstChild();
      while (cloneNode && originalNode)
      {
        auto *cloneElement = DynamicDowncast<SVGElement>(cloneNode);
        auto *originalElement = DynamicDowncast<SVGElement>(originalNode);
        if (cloneElement && originalElement)
        {
          function(*cloneElement, *originalElement);
        }
        cloneNode = dom::TreeQueries::Next(*cloneNode, &clone);
        originalNode = dom::TreeQueries::Next(*originalNode, &original);
      }
    }

    void AssociateClonesWithOriginals(SVGElement &clone, SVGElement &original) noexcept
    {
      // This assertion checks that we don't call this with the arguments backwards.
      // The clone is new and so it's not installed in a parent yet.
      krys_debug_assert(!clone.ParentNode());

      // The loop below works because we are associating these clones immediately, before
      // doing transformations like removing disallowed elements or expanding elements.
      clone.SetCorrespondingElement(&original);
      ForEachDescendantPair(clone, original, [](SVGElement &cloneElement, SVGElement &originalElement)
                            { cloneElement.SetCorrespondingElement(&originalElement); });
    }

    void AssociateReplacementCloneWithOriginal(SVGElement &replacementClone,
                                               SVGElement &originalClone) noexcept
    {
      RefPtr<SVGElement> correspondingElement = ShareRefPtr(originalClone.CorrespondingElement());
      krys_debug_assert(correspondingElement);
      originalClone.SetCorrespondingElement(nullptr);
      replacementClone.SetCorrespondingElement(correspondingElement.get());
    }

    void AssociateReplacementClonesWithOriginals(SVGElement &replacementClone,
                                                 SVGElement &originalClone) noexcept
    {
      // This assertion checks that we don't call this with the arguments backwards.
      // The replacement clone is new and so it's not installed in a parent yet.
      krys_debug_assert(!replacementClone.ParentNode());

      // The loop below works because we are associating these clones immediately, before
      // doing transformations like removing disallowed elements or expanding elements.
      AssociateReplacementCloneWithOriginal(replacementClone, originalClone);
      ForEachDescendantPair(replacementClone, originalClone, &AssociateReplacementCloneWithOriginal);
    }
  }

  RefPtr<SVGElement> SVGUseElement::FindTarget(dom::DOMStringAtom *targetID) const noexcept
  {
    auto *correspondingElement = CorrespondingElement();
    const SVGUseElement &original =
      correspondingElement ? Downcast<SVGUseElement>(*correspondingElement) : *this;

    // FIXME: The element in the external document of a reference to another file, with XML and SVG
    // documents (B11d).
    auto targetResult = TargetElementFromIRIString(original.Href(), original.TreeScope());
    if (targetID)
    {
      *targetID = krys::move(targetResult.Identifier);
      // If the reference is external, don't return the target ID to the caller.
      // The caller would use the target ID to wait for a pending resource on the wrong document.
      // If we ever want the change that and let the caller to wait on the external document,
      // we should change this function so it returns the appropriate document to go with the ID.
      if (*targetID != dom::DOMStringAtom::Null()
          && IsExternalURIReference(original.Href(), original.NodeDocument()))
      {
        *targetID = dom::DOMStringAtom::Null();
      }
    }
    RefPtr<SVGElement> target = ShareRefPtr(DynamicDowncast<SVGElement>(targetResult.Element.get()));
    if (!target)
    {
      return nullptr;
    }

    if (!target->IsConnected() || IsDisallowedElement(*target))
    {
      return nullptr;
    }

    if (correspondingElement)
    {
      for (const SVGElement *ancestor = this; ancestor;
           ancestor = DynamicDowncast<SVGElement>(ancestor->ParentNode()))
      {
        if (ancestor->CorrespondingElement() == target.get())
        {
          return nullptr;
        }
      }
    }
    else
    {
      if (dom::TreeQueries::IsInclusiveAncestor(*target, *this))
      {
        return nullptr;
      }
      // Target should only refer to a node in the same tree or a node in another document.
      krys_debug_assert(!dom::TreeQueries::IsShadowIncludingDescendant(*this, *target));
    }

    return target;
  }

  void SVGUseElement::CloneTarget(dom::ContainerNode &container, SVGElement &target) const noexcept
  {
    auto clone = dom::NodeAlgorithms::CloneNode(target, &NodeDocument(), true);
    krys_assert(!clone.HasException());
    Ref<SVGElement> targetClone = ShareRef(Downcast<SVGElement>(*clone.ReleaseValue()));
    AssociateClonesWithOriginals(*targetClone, target);
    RemoveDisallowedElementsFromSubtree(*targetClone);
    RemoveSymbolElementsFromSubtree(*targetClone);
    TransferSizeAttributesToTargetClone(*targetClone);
    (void)container.AppendChild(*targetClone);
  }

  namespace
  {
    void CloneDataAndChildren(SVGElement &replacementClone, SVGElement &originalClone) noexcept
    {
      // This assertion checks that we don't call this with the arguments backwards.
      // The replacement clone is new and so it's not installed in a parent yet.
      krys_debug_assert(!replacementClone.ParentNode());

      originalClone.SynchronizeAllAttributes();
      for (const auto &attribute : originalClone.AttributeList())
      {
        (void)replacementClone.SetAttributeNS(attribute->NamespaceURI(),
                                              dom::DOMStringAtom(attribute->Name()),
                                              dom::DOMString(attribute->Value()));
      }
      for (auto *child = originalClone.FirstChild(); child; child = child->NextSibling())
      {
        krys_assert(
          !dom::NodeAlgorithms::CloneNode(*child, &replacementClone.NodeDocument(), true, &replacementClone)
             .HasException());
      }
      AssociateReplacementClonesWithOriginals(replacementClone, originalClone);
      RemoveDisallowedElementsFromSubtree(replacementClone);
    }
  }

  void SVGUseElement::ExpandUseElementsInShadowTree() const noexcept
  {
    ForEachDescendantOfType<SVGUseElement>(
      *UserAgentShadowRoot(),
      [&](SVGUseElement &element) -> dom::Node &
      {
        Ref<SVGUseElement> originalClone = ShareRef(element);

        RefPtr<SVGElement> target = originalClone->FindTarget();

        // Spec: In the generated content, the 'use' will be replaced by 'g', where all attributes from the
        // 'use' element except for x, y, width, height and xlink:href are transferred to the generated 'g'
        // element.

        Ref<SVGElement> replacementClone = CreateSVGElement(NodeDocument(), u8"g", html::TagName::g);

        CloneDataAndChildren(*replacementClone, *originalClone);

        replacementClone->RemoveAttribute(SVGNames::XAttr().LocalName());
        replacementClone->RemoveAttribute(SVGNames::YAttr().LocalName());
        replacementClone->RemoveAttribute(SVGNames::WidthAttr().LocalName());
        replacementClone->RemoveAttribute(SVGNames::HeightAttr().LocalName());
        replacementClone->RemoveAttribute(SVGNames::HrefAttr().LocalName());
        replacementClone->RemoveAttributeNS(XLinkNames::HrefAttr().NamespaceURI(),
                                            XLinkNames::HrefAttr().LocalName());

        if (target)
        {
          originalClone->CloneTarget(*replacementClone, *target);
        }

        (void)originalClone->ParentNode()->ReplaceChild(*replacementClone, *originalClone);

        // Resume iterating, starting just inside the replacement clone.
        return *replacementClone;
      });
  }

  void SVGUseElement::ExpandSymbolElementsInShadowTree() const noexcept
  {
    ForEachDescendantOfType<SVGSymbolElement>(
      *UserAgentShadowRoot(),
      [&](SVGSymbolElement &element) -> dom::Node &
      {
        Ref<SVGSymbolElement> originalClone = ShareRef(element);

        // Spec: The referenced 'symbol' and its contents are deep-cloned into the generated tree,
        // with the exception that the 'symbol' is replaced by an 'svg'. This generated 'svg' will
        // always have explicit values for attributes width and height. If attributes width and/or
        // height are provided on the 'use' element, then these attributes will be transferred to
        // the generated 'svg'. If attributes width and/or height are not specified, the generated
        // 'svg' element will use values of 100% for these attributes.

        Ref<SVGElement> replacementClone = CreateSVGElement(NodeDocument(), u8"svg", html::TagName::svg);

        CloneDataAndChildren(*replacementClone, *originalClone);

        (void)originalClone->ParentNode()->ReplaceChild(*replacementClone, *originalClone);

        // Resume iterating, starting just inside the replacement clone.
        return *replacementClone;
      });
  }

  void SVGUseElement::TransferEventListenersToShadowTree() const noexcept
  {
    // FIXME: Don't directly add event listeners on each descendant. Copy event listeners on the use element
    // instead.
    ForEachDescendantOfType<SVGElement>(
      *UserAgentShadowRoot(),
      [](SVGElement &descendant) -> dom::Node &
      {
        for (auto &listener : descendant.CorrespondingElement()->EventListeners())
        {
          descendant.AddEventListener(
            listener->Type(), ShareRef(*listener->Callback()),
            dom::AddEventListenerOptions {
              {listener->Capture()}, listener->Once(), listener->Passive(), ShareRefPtr(listener->Signal())});
        }
        return descendant;
      });
  }

  void SVGUseElement::InvalidateShadowTree() noexcept
  {
    if (_shadowTreeNeedsUpdate)
    {
      return;
    }
    _shadowTreeNeedsUpdate = true;
    InvalidateStyleAndRenderersForSubtree();
    InvalidateDependentShadowTrees();
    if (IsConnected())
    {
      NodeDocument().AddElementWithPendingUserAgentShadowTreeUpdate(*this);
    }
  }

  void SVGUseElement::InvalidateDependentShadowTrees() noexcept
  {
    for (auto &instance : Instances())
    {
      if (RefPtr<SVGUseElement> element = ShareRefPtr(instance->CorrespondingUseElement()))
      {
        element->InvalidateShadowTree();
      }
    }
  }

  bool SVGUseElement::SelfHasRelativeLengths() const noexcept
  {
    if (X().IsRelative() || Y().IsRelative() || Width().IsRelative() || Height().IsRelative())
    {
      return true;
    }

    auto targetClone = TargetClone();
    return targetClone && targetClone->HasRelativeLengths();
  }

  void SVGUseElement::UpdateExternalDocument() noexcept
  {
    // FIXME: Load the document of a reference to another file (requestSVGDocument), dispatching load and
    // error events when it finishes (notifyFinished), with XML and SVG documents (B11d). Until then such a
    // reference finds no target.
  }
}
