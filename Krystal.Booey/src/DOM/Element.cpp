#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Internals/RareData/ElementRareData.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.Booey/DOM/Mixins/ParentNode.hpp"
#include "Krystal.Booey/DOM/Mixins/Slottable.hpp"
#include "Krystal.Booey/DOM/NamedNodeMap.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/SelectorQuery.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/TreeResolver.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <ranges>

namespace krys::boo::dom
{
  Element::Element(Document &document, const QualifiedName &name, NodeFlags nodeFlags) noexcept
      : ContainerNode(document, NodeType::ELEMENT_NODE, nodeFlags), _qualifiedName(name)
  {
  }

  Element::~Element() noexcept = default;

  ElementRareData &Element::EnsureElementRareData() noexcept
  {
    if (!_elementRareData)
    {
      _elementRareData = CreateUnique<ElementRareData>();
    }
    return *_elementRareData;
  }

  double
    Element::LookupCSSRandomBaseValue(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                                      const css::calc::RandomCachingKey &key) const noexcept
  {
    return const_cast<Element *>(this)
      ->EnsureElementRareData()
      .EnsureRandomCachingKeyMap(pseudoElementIdentifier)
      ->LookupCSSRandomBaseValue(key);
  }

#pragma region Style

  const RenderStyle *Element::ExistingComputedStyle() const noexcept
  {
    if (_elementRareData)
    {
      if (auto *style = _elementRareData->ComputedStyle())
      {
        return style;
      }
    }

    if (HasDisplayNone())
    {
      return _elementRareData->DisplayContentsOrNoneStyle();
    }

    return RenderOrDisplayContentsStyle();
  }

  const RenderStyle *Element::RenderOrDisplayContentsStyle() const noexcept
  {
    return RenderOrDisplayContentsStyle(null);
  }

  const RenderStyle *Element::RenderOrDisplayContentsStyle(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    if (pseudoElementIdentifier)
    {
      if (auto *style = RenderOrDisplayContentsStyle())
      {
        if (auto *cachedPseudoStyle = style->GetCachedPseudoStyle(*pseudoElementIdentifier))
        {
          return cachedPseudoStyle;
        }
      }

      return nullptr;
    }

    if (HasDisplayContents())
    {
      return _elementRareData->DisplayContentsOrNoneStyle();
    }

    return GetRenderStyle();
  }

  const RenderStyle *Element::ResolveComputedStyle(ResolveComputedStyleMode mode) noexcept
  {
    krys_debug_assert(IsConnected());

    auto document = ShareRef(NodeDocument());
    document->StyleScope().FlushPendingUpdate();

    bool isInDisplayNoneTree = false;

    // Traverse the ancestor chain to find the rootmost element that has invalid computed style.
    auto rootmostInvalidElement = [&]() -> RefPtr<const Element>
    {
      // In ResolveComputedStyleMode::RenderedOnly case we check for display:none ancestors.
      if (mode != ResolveComputedStyleMode::RenderedOnly && !document->HasPendingStyleRecalc()
          && ExistingComputedStyle())
      {
        return nullptr;
      }

      if (document->HasPendingFullStyleRebuild())
      {
        return document->DocumentElement();
      }

      auto documentElement = document->DocumentElement();
      if (!documentElement || documentElement->HasNodeFlag(NodeFlags::IsComputedStyleInvalid))
      {
        return documentElement;
      }

      RefPtr<const Element> rootmost;

      for (auto element = ShareRefPtr<const Element>(this); element;
           element = ShareRefPtr<const Element>(element->ParentElementInComposedTree()))
      {
        if (element->HasNodeFlag(NodeFlags::IsComputedStyleInvalid))
        {
          rootmost = element;
          continue;
        }
        auto *existing = element->ExistingComputedStyle();
        if (!existing)
        {
          rootmost = element;
          continue;
        }
        if (mode == ResolveComputedStyleMode::RenderedOnly && existing->Display() == DisplayType::None)
        {
          isInDisplayNoneTree = true;
          // Invalid ancestor style may still affect this display:none style.
          rootmost = nullptr;
        }
      }
      return rootmost;
    }();

    if (!rootmostInvalidElement)
    {
      if (isInDisplayNoneTree)
      {
        return nullptr;
      }
      return ExistingComputedStyle();
    }

    auto ancestorWithValidStyle = ShareRefPtr(rootmostInvalidElement->ParentElementInComposedTree());

    List<RefPtr<Element>> elementsRequiringComputedStyle;
    for (auto toResolve = ShareRefPtr(this); toResolve != ancestorWithValidStyle;
         toResolve = ShareRefPtr(toResolve->ParentElementInComposedTree()))
    {
      elementsRequiringComputedStyle.push_back(toResolve);
    }

    auto *computedStyle = ancestorWithValidStyle ? ancestorWithValidStyle->ExistingComputedStyle() : nullptr;

    style::PostResolutionCallbackDisabler disabler(*document,
                                                   style::PostResolutionCallbackDisabler::DrainCallbacks::No);

    // Resolve and cache styles starting from the most distant ancestor.
    // FIXME: This is not as efficient as it could be. For example if an ancestor has a non-inherited style
    // change but the styles are otherwise clean we would not need to re-resolve descendants.
    for (auto &element : elementsRequiringComputedStyle | std::views::reverse)
    {
      if (computedStyle && computedStyle->GetContainerType() != ContainerType::Normal
          && mode != ResolveComputedStyleMode::Editability)
      {
        // If we find a query container we need to bail out and do full style update to resolve it.
        if (document->UpdateStyleIfNeeded())
        {
          return ComputedStyle();
        }
      }
      auto style = document->StyleForElementIgnoringPendingStylesheets(*element, computedStyle);
      computedStyle = style.get();
      auto &rareData = element->EnsureElementRareData();
      if (auto *existing = rareData.ComputedStyle())
      {
        auto changes = style::DetermineChanges(*existing, *style);
        changes.Remove(style::Change::NonInherited);
        if (!changes.IsEmpty())
        {
          for (auto &child : ComposedTreeChildren(*element))
          {
            if (auto *childElement = DynamicDowncast<Element>(child))
            {
              childElement->SetNodeFlag(NodeFlags::IsComputedStyleInvalid);
            }
          }
        }
      }
      rareData.SetComputedStyle(krys::move(style));
      element->ClearNodeFlag(NodeFlags::IsComputedStyleInvalid);

      if (mode == ResolveComputedStyleMode::RenderedOnly && computedStyle->Display() == DisplayType::None)
      {
        return nullptr;
      }
    }

    return computedStyle;
  }

  const RenderStyle &
    Element::ResolvePseudoElementStyle(const style::PseudoElementIdentifier &pseudoElementIdentifier) noexcept
  {
    auto *parentStyle = ExistingComputedStyle();
    krys_debug_assert(parentStyle);
    krys_debug_assert(!parentStyle->GetCachedPseudoStyle(pseudoElementIdentifier));

    auto document = ShareRef(NodeDocument());
    style::PostResolutionCallbackDisabler disabler(*document,
                                                   style::PostResolutionCallbackDisabler::DrainCallbacks::No);

    auto style =
      document->StyleForElementIgnoringPendingStylesheets(*this, parentStyle, pseudoElementIdentifier);
    if (!style)
    {
      style = RenderStyle::CreatePtr();
      style->InheritFrom(*parentStyle);
      style->SetPseudoElementIdentifier(pseudoElementIdentifier);
    }

    auto *computedStyle = style.get();
    const_cast<RenderStyle *>(parentStyle)->AddCachedPseudoStyle(krys::move(style));
    krys_debug_assert(parentStyle->GetCachedPseudoStyle(pseudoElementIdentifier));
    return *computedStyle;
  }

  const RenderStyle *
    Element::ComputedStyle(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    if (!IsConnected())
    {
      return nullptr;
    }

    // FIXME: Before and after pseudo-elements as nodes (PseudoElement), which have their own computed style.

    // FIXME: This should call resolveComputedStyle() unconditionally so we check if the style is valid.
    auto *style = ExistingComputedStyle();
    if (!style)
    {
      style = ResolveComputedStyle();
    }

    if (pseudoElementIdentifier)
    {
      if (auto *cachedPseudoStyle = style->GetCachedPseudoStyle(*pseudoElementIdentifier))
      {
        return cachedPseudoStyle;
      }
      return &ResolvePseudoElementStyle(*pseudoElementIdentifier);
    }

    return style;
  }

  void Element::ResetComputedStyle() noexcept
  {
    if (!_elementRareData || !_elementRareData->ComputedStyle())
    {
      return;
    }

    // FIXME: willResetComputedStyle for elements with custom style resolve callbacks.
    _elementRareData->SetComputedStyle(nullptr);
    for (auto *descendant = TreeQueries::Next(*this, this); descendant;
         descendant = TreeQueries::Next(*descendant, this))
    {
      auto *child = DynamicDowncast<Element>(*descendant);
      if (!child || !child->_elementRareData || !child->_elementRareData->ComputedStyle()
          || child->HasDisplayContents() || child->HasDisplayNone())
      {
        continue;
      }
      child->_elementRareData->SetComputedStyle(nullptr);
    }
  }

  void Element::ResetStyleRelations() noexcept
  {
    ClearStyleFlags(NodeStyleFlags::StyleAffectedByEmpty);
    ClearStyleFlags(NodeStyleFlags::AffectedByHasWithPositionalPseudoClass);
    if (!_elementRareData)
    {
      return;
    }
    _elementRareData->SetChildIndex(0);
  }

  void Element::ResetChildStyleRelations() noexcept
  {
    ClearStyleFlags(NodeStyleFlags::ChildrenAffectedByFirstChildRules
                    | NodeStyleFlags::ChildrenAffectedByLastChildRules
                    | NodeStyleFlags::ChildrenAffectedByForwardPositionalRules
                    | NodeStyleFlags::ChildrenAffectedByBackwardPositionalRules);
  }

  void Element::ResetAllDescendantStyleRelations() noexcept
  {
    ResetChildStyleRelations();

    ClearStyleFlags(NodeStyleFlags::DescendantsAffectedByForwardPositionalRules
                    | NodeStyleFlags::DescendantsAffectedByBackwardPositionalRules);
  }

  bool Element::HasDisplayContents() const noexcept
  {
    if (!_elementRareData)
    {
      return false;
    }
    auto *style = _elementRareData->DisplayContentsOrNoneStyle();
    return style && style->Display() == DisplayType::Contents;
  }

  bool Element::HasDisplayNone() const noexcept
  {
    if (!_elementRareData)
    {
      return false;
    }
    auto *style = _elementRareData->DisplayContentsOrNoneStyle();
    return style && style->Display() == DisplayType::None;
  }

  void Element::StoreDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept
  {
    // This is used by RenderTreeBuilder to store the style for Elements with display:{contents|none}.
    // Normally style is held in renderers but display:contents doesn't generate one.
    // This is kept distinct from ElementRareData::computedStyle() which can update outside style resolution.
    // This way renderOrDisplayContentsStyle() always returns consistent styles matching the rendering state.
    krys_debug_assert(
      style && (style->Display() == DisplayType::Contents || style->Display() == DisplayType::None));
    krys_debug_assert(!_rendererStyle);
    EnsureElementRareData().SetDisplayContentsOrNoneStyle(krys::move(style));
  }

  void Element::ClearDisplayContentsOrNoneStyle() noexcept
  {
    if (!_elementRareData)
    {
      return;
    }
    _elementRareData->SetDisplayContentsOrNoneStyle(nullptr);
  }

  bool Element::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    return style.Display() != DisplayType::None && style.Display() != DisplayType::Contents;
  }

  void Element::InvalidateStyle() noexcept
  {
    Node::InvalidateStyle(style::Validity::ElementInvalid);
  }

  void Element::InvalidateStyleAndLayerComposition() noexcept
  {
    Node::InvalidateStyle(style::Validity::ElementInvalid, style::InvalidationMode::RecompositeLayer);
  }

  void Element::InvalidateStyleForSubtree() noexcept
  {
    Node::InvalidateStyle(style::Validity::SubtreeInvalid);
  }

  void Element::InvalidateStyleAndRenderersForSubtree() noexcept
  {
    Node::InvalidateStyle(style::Validity::SubtreeInvalid, style::InvalidationMode::RebuildRenderer);
  }

  void Element::InvalidateRenderer() noexcept
  {
    Node::InvalidateStyle(style::Validity::Valid, style::InvalidationMode::RebuildRenderer);
  }

  void Element::InvalidateStyleInternal() noexcept
  {
    Node::InvalidateStyle(style::Validity::ElementInvalid);
  }

  void Element::InvalidateStyleForSubtreeInternal() noexcept
  {
    Node::InvalidateStyle(style::Validity::SubtreeInvalid);
  }

  void Element::InvalidateForQueryContainerSizeChange() noexcept
  {
    // FIXME: Ideally we would just recompute things that are actually affected by containers queries within
    // the subtree.
    Node::InvalidateStyle(style::Validity::SubtreeInvalid);
    SetNodeFlag(NodeFlags::NeedsUpdateQueryContainerDependentStyle);
  }

  void Element::InvalidateForResumingQueryContainerResolution() noexcept
  {
    SetChildNeedsStyleRecalc();
    MarkAncestorsForInvalidatedStyle();
  }

  bool Element::HasFlagsSetDuringStylingOfChildren() const noexcept
  {
    return ChildrenAffectedByFirstChildRules() || ChildrenAffectedByLastChildRules()
           || ChildrenAffectedByForwardPositionalRules() || DescendantsAffectedByForwardPositionalRules()
           || ChildrenAffectedByBackwardPositionalRules() || DescendantsAffectedByBackwardPositionalRules();
  }

  uint32 Element::ChildIndex() const noexcept
  {
    return _elementRareData ? _elementRareData->ChildIndex() : 0;
  }

  void Element::SetChildIndex(uint32 index) noexcept
  {
    EnsureElementRareData().SetChildIndex(index);
  }

#pragma endregion

  bool Element::HasRandomCachingKeyMap() const noexcept
  {
    if (!_elementRareData)
    {
      return false;
    }
    return _elementRareData->HasRandomCachingKeyMap();
  }

#pragma region Element

  DOMString Element::TagName() const noexcept
  {
    auto qualifiedName = _qualifiedName.Name();

    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIIUpper(qualifiedName);
    }

    return qualifiedName;
  }

  void Element::Id(DOMString &&id) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"id", krys::move(id));
  }

  DOMString Element::Id() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"id");
  }

  bool Element::HasClassName(DOMStringView className) const noexcept
  {
    auto classes = GetAttribute(u8"class");
    if (!classes || className.empty())
    {
      return false;
    }
    bool foldCase = NodeDocument().InQuirksMode();
    DOMStringView remaining = *classes;
    while (!remaining.empty())
    {
      auto start = std::ranges::find_if_not(remaining, [](char8 c) { return text::IsASCIIWhitespace(c); });
      auto end =
        std::ranges::find_if(start, remaining.end(), [](char8 c) { return text::IsASCIIWhitespace(c); });
      DOMStringView token(start, end);
      if (token.size() == className.size()
          && (foldCase
                ? std::ranges::equal(token, className, {}, [](char8 c) { return text::ToASCIILower(c); })
                : token == className))
      {
        return true;
      }
      remaining = DOMStringView(end, remaining.end());
    }
    return false;
  }

  List<DOMStringAtom> Element::ClassNames() const noexcept
  {
    List<DOMStringAtom> classNames;
    auto classes = GetAttribute(u8"class");
    if (!classes)
    {
      return classNames;
    }
    bool foldCase = NodeDocument().InQuirksMode();
    DOMStringView remaining = *classes;
    while (!remaining.empty())
    {
      auto start = std::ranges::find_if_not(remaining, [](char8 c) { return text::IsASCIIWhitespace(c); });
      auto end =
        std::ranges::find_if(start, remaining.end(), [](char8 c) { return text::IsASCIIWhitespace(c); });
      if (start != end)
      {
        DOMStringAtom className = foldCase ? DOMStringAtom(text::ToASCIILower(DOMStringView(start, end)))
                                           : DOMStringAtom(DOMStringView(start, end));
        if (std::ranges::find(classNames, className) == classNames.end())
        {
          classNames.push_back(className);
        }
      }
      remaining = DOMStringView(end, remaining.end());
    }
    return classNames;
  }

  DOMString Element::IdForStyleResolution() const noexcept
  {
    auto id = GetAttribute(u8"id").value_or(DOMString());
    if (NodeDocument().InQuirksMode())
    {
      return text::ToASCIILower(DOMStringView(id));
    }
    return id;
  }

  void Element::ClassName(DOMString &&className) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"class", krys::move(className));
  }

  DOMString Element::ClassName() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"class");
  }

  DOMTokenList &Element::ClassList() noexcept
  {
    if (_domTokenList == nullptr)
    {
      _domTokenList = ElementAlgorithms::CreateDOMTokenList(*this, u8"class");
    }

    return *_domTokenList;
  }

  void Element::Slot(DOMString &&slot) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"slot", krys::move(slot));
  }

  DOMString Element::Slot() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"slot");
  }

  bool Element::HasAttributes() const noexcept
  {
    SynchronizeAllAttributes();
    return !_attributes.empty();
  }

  NamedNodeMap &Element::Attributes() noexcept
  {
    if (_namedNodeMap == nullptr)
    {
      _namedNodeMap = UniquePtr<NamedNodeMap>(new NamedNodeMap(*this));
    }

    return *_namedNodeMap;
  }

  List<DOMString> Element::GetAttributeNames() const noexcept
  {
    SynchronizeAllAttributes();
    return std::ranges::views::transform(_attributes, [](const Ref<Attr> &attr) { return attr->Name(); })
           | std::ranges::to<List<DOMString>>();
  }

  Maybe<DOMString> Element::GetAttribute(DOMStringAtom qualifiedName) const noexcept
  {
    Attr *attr = ElementAlgorithms::GetAttributeByName(qualifiedName, *this);
    if (attr == nullptr)
    {
      return null;
    }

    return attr->Value();
  }

  Maybe<DOMString> Element::GetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    Attr *attr = ElementAlgorithms::GetAttributeByNamespace(namespaceURI, localName, *this);
    if (attr == nullptr)
    {
      return std::nullopt;
    }

    return attr->Value();
  }

  ExceptionOr<void> Element::SetAttribute(DOMStringAtom qualifiedName, DOMString &&value) noexcept
  {
    if (!NameValidation::IsValidAttributeLocalName(qualifiedName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
    // attribute value with qualifiedName, null, this, and value.

    SynchronizeAllAttributes();
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });
    if (it != _attributes.end())
    {
      ElementAlgorithms::ChangeAttribute(**it, krys::move(value));
    }
    else
    {
      auto attr = AdoptRef<Attr>(
        *new Attr(NodeDocument(), QualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), qualifiedName},
                  krys::move(value)));
      ElementAlgorithms::AppendAttribute(*attr, *this);
    }

    return {};
  }

  ExceptionOr<void> Element::SetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
                                            DOMString &&value) noexcept
  {
    auto validateAndExtractResult =
      NameValidation::ValidateAndExtract(namespaceURI, qualifiedName, ValidateAndExtractContext::Attribute);

    if (validateAndExtractResult.HasException())
    {
      return validateAndExtractResult.ReleaseException();
    }

    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
    // attribute value with localName, namespace, this, and value.

    const auto &qName = validateAndExtractResult.Value();
    ElementAlgorithms::SetAttributeValue(*this, qName.LocalName(), std::move(value), qName.NamespacePrefix(),
                                         qName.NamespaceURI());

    return {};
  }

  void Element::RemoveAttribute(DOMStringAtom qualifiedName) noexcept
  {
    ElementAlgorithms::RemoveAttributeByName(qualifiedName, *this);
  }

  void Element::RemoveAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) noexcept
  {
    ElementAlgorithms::RemoveAttributeByNamespace(namespaceURI, localName, *this);
  }

  ExceptionOr<bool> Element::ToggleAttribute(DOMStringAtom qualifiedName, const Maybe<bool> &force) noexcept
  {
    if (!NameValidation::IsValidAttributeLocalName(qualifiedName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    SynchronizeAllAttributes();
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });

    if (it == _attributes.end())
    {
      if (!force.has_value() || force.value())
      {
        auto attr = AdoptRef<Attr>(*new Attr(
          NodeDocument(), QualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), qualifiedName}, u8""));
        ElementAlgorithms::AppendAttribute(*attr, *this);
        return true;
      }

      return false;
    }

    if (!force.has_value() || !force.value())
    {
      ElementAlgorithms::RemoveAttribute(**it);
      return false;
    }

    return true;
  }

  bool Element::HasAttribute(DOMStringAtom qualifiedName) const noexcept
  {
    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    SynchronizeAllAttributes();
    return std::ranges::any_of(_attributes, [&](const Ref<Attr> &a) { return a->Name() == qualifiedName; });
  }

  bool Element::HasAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    SynchronizeAllAttributes();
    return std::ranges::any_of(_attributes, [&](const Ref<Attr> &a)
                               { return a->NamespaceURI() == namespaceURI && a->LocalName() == localName; });
  }

  Attr *Element::GetAttributeNode(DOMStringAtom qualifiedName) const noexcept
  {
    return ElementAlgorithms::GetAttributeByName(qualifiedName, *this);
  }

  Attr *Element::GetAttributeNodeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    return ElementAlgorithms::GetAttributeByNamespace(namespaceURI, localName, *this);
  }

  ExceptionOr<RefPtr<Attr>> Element::SetAttributeNode(Attr &attr) noexcept
  {
    return ElementAlgorithms::SetAttribute(attr, *this);
  }

  ExceptionOr<RefPtr<Attr>> Element::SetAttributeNodeNS(Attr &attr) noexcept
  {
    return ElementAlgorithms::SetAttribute(attr, *this);
  }

  ExceptionOr<Ref<Attr>> Element::RemoveAttributeNode(Attr &attr) noexcept
  {
    if (!std::ranges::any_of(_attributes, [&](const Ref<Attr> &a) { return a.get() == &attr; }))
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    ElementAlgorithms::RemoveAttribute(attr);

    return ShareRef(attr);
  }

  ExceptionOr<Ref<ShadowRoot>> Element::AttachShadow(const ShadowRootInit &init) noexcept
  {
    auto registry =
      init.CustomElementRegistry ? init.CustomElementRegistry : NodeDocument().CustomElementRegistry().get();

    if (registry != nullptr && !registry->IsScoped() && registry != NodeDocument().CustomElementRegistry())
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (auto attachResult =
          ElementAlgorithms::AttachShadowRoot(*this, init.Mode, init.Clonable, init.Serialisable,
                                              init.DelegatesFocus, init.SlotAssignment, registry);
        attachResult.HasException())
    {
      return attachResult.ReleaseException();
    }

    return ShareRef(*_shadowRoot);
  }

  RefPtr<ShadowRoot> Element::ShadowRoot() const noexcept
  {
    if (!_shadowRoot || _shadowRoot->Mode() == ShadowRootMode::Closed)
    {
      return nullptr;
    }

    return _shadowRoot;
  }

  ExceptionOr<RefPtr<Element>> Element::Closest(DOMStringView selectors) noexcept
  {
    auto query = NodeDocument().SelectorQueryForString(selectors);
    if (query.HasException())
    {
      return query.ReleaseException();
    }
    return ShareRefPtr(query.Value().Closest(*this));
  }

  ExceptionOr<bool> Element::Matches(DOMStringView selectors) const noexcept
  {
    auto query = NodeDocument().SelectorQueryForString(selectors);
    if (query.HasException())
    {
      return query.ReleaseException();
    }
    return query.Value().Matches(*this);
  }

  Ref<HTMLCollection> Element::GetElementsByTagName(DOMStringAtom qualifiedName) noexcept
  {
    return NodeAlgorithms::GetElementsByTagName(*this, qualifiedName);
  }

  Ref<HTMLCollection> Element::GetElementsByTagNameNS(DOMStringAtom namespaceURI,
                                                      DOMStringAtom localName) noexcept
  {
    return NodeAlgorithms::GetElementsByTagNameNS(*this, namespaceURI, localName);
  }

  Ref<HTMLCollection> Element::GetElementsByClassName(DOMStringAtom classNames) noexcept
  {
    return NodeAlgorithms::GetElementsByClassName(*this, classNames);
  }

  ExceptionOr<Element *> Element::InsertAdjacentElement(InsertAdjacentWhere where, Element &element) noexcept
  {
    if (auto result = ElementAlgorithms::InsertAdjacent(*this, where, element); result.HasException())
    {
      return result.ReleaseException();
    }

    return &element;
  }

  ExceptionOr<void> Element::InsertAdjacentText(InsertAdjacentWhere where, DOMString &&data) noexcept
  {
    auto textNode = CreateRef<Text>(NodeDocument(), krys::move(data));
    if (auto result = ElementAlgorithms::InsertAdjacent(*this, where, *textNode); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

#pragma endregion

#pragma region Node

  DOMString Element::NodeName() const noexcept
  {
    return TagName();
  }

  Maybe<DOMString> Element::TextContent() const noexcept
  {
    return TextAlgorithms::DescendantTextContent(*this);
  }

  ExceptionOr<void> Element::TextContent(DOMString &&value) noexcept
  {
    return NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

#pragma endregion

#pragma region ChildNode

  ExceptionOr<void> Element::Before(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ChildNode::Before(*this, nodes);
  }

  ExceptionOr<void> Element::After(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ChildNode::After(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ChildNode::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> Element::Remove() noexcept
  {
    return mixins::ChildNode::Remove(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RefPtr<const Element> Element::PreviousElementSibling() const noexcept
  {
    return mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<Element> Element::PreviousElementSibling() noexcept
  {
    return mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<const Element> Element::NextElementSibling() const noexcept
  {
    return mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

  RefPtr<Element> Element::NextElementSibling() noexcept
  {
    return mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Element::Children() noexcept
  {
    return EnsureElementRareData().Children(*this);
  }

  RefPtr<const Element> Element::FirstElementChild() const noexcept
  {
    return mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<Element> Element::FirstElementChild() noexcept
  {
    return mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<const Element> Element::LastElementChild() const noexcept
  {
    return mixins::ParentNode::LastElementChild(*this);
  }

  RefPtr<Element> Element::LastElementChild() noexcept
  {
    return mixins::ParentNode::LastElementChild(*this);
  }

  size_t Element::ChildElementCount() const noexcept
  {
    return mixins::ParentNode::ChildElementCount(*this);
  }

  ExceptionOr<void> Element::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> Element::Append(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> Element::MoveBefore(Node &node, Node *refChild) noexcept
  {
    return mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> Element::QuerySelector(DOMStringView selectors) noexcept
  {
    return mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<NodeList>> Element::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region Slottable

  RefPtr<html::HTMLSlotElement> Element::AssignedSlot() noexcept
  {
    return mixins::Slottable::AssignedSlot(*this);
  }

#pragma endregion

#pragma region Lazy Attributes

  void Element::SetSynchronizedLazyAttribute(DOMStringAtom localName, DOMString &&value) noexcept
  {
    auto it = std::ranges::find_if(
      _attributes, [&](const Ref<Attr> &attr)
      { return attr->LocalName() == localName && attr->NamespaceURI() == DOMStringAtom::Null(); });
    if (it != _attributes.end())
    {
      (*it)->_value = krys::move(value);
      return;
    }

    auto attr = AdoptRef<Attr>(
      *new Attr(NodeDocument(), QualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), localName},
                krys::move(value)));
    attr->_ownerElement = CreateWeakPtr(this);
    _attributes.push_back(krys::move(attr));
  }

  void Element::SynchronizeStyleAttribute() const noexcept
  {
    const_cast<StyledElement &>(Downcast<StyledElement>(*this)).SynchronizeStyleAttributeInternal();
  }

#pragma endregion

#pragma region Extension Hooks

  void Element::OnAttributeChanged(DOMStringAtom localName, Maybe<DOMStringView> oldValue,
                                   Maybe<DOMStringView> value, DOMStringAtom namespaceURI) noexcept
  {
    if (namespaceURI != DOMStringAtom::Null())
    {
      return;
    }

    // NOTE: we currently don't need to handle `id` as we just use the value stored in the attribute. Later,
    // when we optimise we can use this hook to update a member variable to skip needing to look up the
    // attribute. This will be particularly important for a faster implementation of GetElementById.

    if (localName == Attributes::Class)
    {
      auto &domTokenList = ClassList();
      domTokenList._tokens.clear();
      if (value.has_value())
      {
        auto tokens = OrderedSet::Parser(*value);
        domTokenList._tokens.append(tokens.begin(), tokens.end());
      }
    }
    else if (localName == Attributes::Slot)
    {
      if (value == oldValue)
      {
        return;
      }

      if (!value.has_value() && oldValue.has_value() && oldValue->empty())
      {
        return;
      }

      if (value.has_value() && value->empty() && !oldValue.has_value())
      {
        return;
      }

      if (!value.has_value() || value->empty())
      {
        _slottableName = u8"";
      }
      else
      {
        _slottableName = DOMString(*value);
      }

      if (auto *assignedSlot = SlotAlgorithms::GetAssignedSlot(*this))
      {
        SlotAlgorithms::AssignSlottables(*assignedSlot);
      }

      SlotAlgorithms::AssignSlot(*this);
    }
  }

#pragma endregion
}
