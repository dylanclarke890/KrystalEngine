#pragma once

#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/Mixins/HTMLOrSVGOrMathMLElement.hpp"
#include "Krystal.Booey/SVG/Enums/SVGElementFlags.hpp"
#include "Krystal.Booey/SVG/Enums/SVGElementInterface.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwner.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyRegistry.hpp"
#include "Krystal.Booey/SVG/SVGParsingError.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class AffineTransform;
}

namespace krys::boo::svg
{
  class SVGAnimatedProperty;
  enum class CTMScope : bool;
  class SVGConditionalProcessingAttributes;
  class SVGElementRareData;
  class SVGSvgElement;
  class SVGUseElement;

  template <typename TOwner, typename... TBaseTypes>
  class SVGPropertyOwnerRegistry;

  /// @brief An element in the SVG namespace.
  /// @note Subclasses react to their attributes in `AttributeChanged` (parsing them into their properties,
  /// then calling their base's), and to an attribute or a property having changed in `SVGAttributeChanged`.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGElement
  class SVGElement : public dom::StyledElement, public SVGPropertyOwner
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGElement);

  protected:
    SVGElementFlags _flags : BitCount<SVGElementFlags>() {SVGElementFlags::None};
    SVGElementInterface _interface : BitCount<SVGElementInterface>() {SVGElementInterface::None};

  private:
    UniquePtr<SVGElementRareData> _svgRareData;

    List<WeakPtr<SVGElement>> _childElementsWithRelativeLengths;
    bool _hasRegisteredWithParentForRelativeLengths : 1 {false};
    bool _selfHasRelativeLengths : 1 {false};
    bool _hasInitializedRelativeLengthsState : 1 {false};

    // FIXME: The animator factory of the element's presentation attributes (SVGPropertyAnimatorFactory), with
    // SMIL (B11i).

    UniquePtr<SVGPropertyRegistry> _propertyRegistry;
    Ref<SVGAnimatedString> _className;

  protected:
    SVGElement(dom::Document &document, UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
               SVGElementInterface interface, SVGElementFlags flags = SVGElementFlags::None,
               dom::NodeFlags nodeFlags = dom::NodeFlags::None) noexcept;

    // FIXME: Only for the elements that aren't ported yet, whose only properties are SVGElement's.
    SVGElement(dom::Document &document, SVGElementInterface interface,
               SVGElementFlags flags = SVGElementFlags::None,
               dom::NodeFlags nodeFlags = dom::NodeFlags::None) noexcept;

  public:
    explicit SVGElement(dom::Document &document) noexcept;
    ~SVGElement() noexcept override;

#pragma region HTMLOrSVGOrMathMLElement Mixin - https://html.spec.whatwg.org/#htmlorsvgormathmlelement

    /// @see https://html.spec.whatwg.org/#dom-tabindex
    KRYS_NODISCARD int32 TabIndex() const noexcept
    {
      return html::mixins::HTMLOrSVGOrMathMLElement::TabIndex(*this);
    }

    /// @see https://html.spec.whatwg.org/#dom-tabindex
    void TabIndex(int32 value) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::TabIndex(*this, value);
    }

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    KRYS_NODISCARD bool Autofocus() const noexcept
    {
      return html::mixins::HTMLOrSVGOrMathMLElement::Autofocus(*this);
    }

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    void Autofocus(bool value) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Autofocus(*this, value);
    }

    /// @see https://html.spec.whatwg.org/#dom-focus
    void Focus(const dom::FocusOptions &options = {}) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Focus(*this, options);
    }

    /// @see https://html.spec.whatwg.org/#dom-blur
    void Blur() noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Blur(*this);
    }

#pragma endregion

    /// @brief Whether this is an `<svg>` element inside another SVG element.
    KRYS_NODISCARD bool IsInnerSVGSVGElement() const noexcept;

    /// @brief Whether this is an `<svg>` element that establishes an SVG document fragment: it has no parent
    /// node, its parent isn't an SVG element, or its parent is a `<foreignObject>`.
    KRYS_NODISCARD bool IsOutermostSVGSVGElement() const noexcept;

    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGElement__ownerSVGElement
    KRYS_NODISCARD SVGSvgElement *OwnerSVGElement() const noexcept;

    enum class ViewportElementType : uint8
    {
      Any,        // Returns first SVGSVGElement, SVGImageElement, or symbol
      SVGSVGOnly, // Returns only SVGSVGElement
    };

    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGElement__viewportElement
    KRYS_NODISCARD SVGElement *
      ViewportElement(ViewportElementType type = ViewportElementType::Any) const noexcept;

    KRYS_NODISCARD virtual bool SupportsMarkers() const noexcept
    {
      return false;
    }

    /// @brief Whether the element or one of its descendants has lengths relative to the viewport.
    KRYS_NODISCARD bool HasRelativeLengths() const noexcept;

    KRYS_NODISCARD bool InstanceUpdatesBlocked() const noexcept;
    void SetInstanceUpdatesBlocked(bool value) noexcept;
    KRYS_NODISCARD virtual AffineTransform LocalCoordinateSpaceTransform(CTMScope) const noexcept;

    // For SVGTests
    KRYS_NODISCARD virtual bool IsValid() const noexcept
    {
      return true;
    }

    /// @brief Reacts to the attribute @p attributeName, or the property that reflects it, having changed.
    virtual void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept;

    void SendLoadEventIfPossible() noexcept;

    KRYS_NODISCARD virtual AffineTransform *EnsureSupplementalTransform() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual AffineTransform *SupplementalTransform() const noexcept
    {
      return nullptr;
    }

    void SetAnimatedSVGAttributesAreDirty() noexcept;
    void SetPresentationalHintStyleIsDirty() noexcept;

    /// @brief The element's instances: the clones made of it in the shadow trees of `<use>` elements.
    KRYS_NODISCARD List<Ref<SVGElement>> Instances() const noexcept;

    KRYS_NODISCARD List<Ref<SVGElement>> ReferencingElements() const noexcept;
    void AddReferencingElement(SVGElement &element) noexcept;
    void RemoveReferencingElement(SVGElement &element) noexcept;
    void RemoveElementReference() noexcept;

    /// @brief The element this is an instance of, if it's in a `<use>` element's shadow tree.
    KRYS_NODISCARD SVGElement *CorrespondingElement() const noexcept;
    KRYS_NODISCARD SVGUseElement *CorrespondingUseElement() const noexcept;
    void SetCorrespondingElement(SVGElement *correspondingElement) noexcept;

    KRYS_NODISCARD virtual bool HaveLoadedRequiredResources() noexcept;

    void CallClearTarget() noexcept
    {
      ClearTarget();
    }

    class InstanceUpdateBlocker;
    class InstanceInvalidationGuard;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGElement>;

    KRYS_NODISCARD const SVGPropertyRegistry &GetPropertyRegistry() const noexcept
    {
      return *_propertyRegistry;
    }

    void DetachAllProperties() noexcept;

    KRYS_NODISCARD bool IsAnimatedPropertyAttribute(const dom::QualifiedName &attributeName) const noexcept;

    /// @brief Writes the property that reflects @p name back to the attribute, if it has changed.
    void SynchronizeSVGAttribute(const dom::QualifiedName &name) noexcept;

    /// @brief Writes the properties that have changed back to the attributes they reflect.
    void SynchronizeAllSVGAttributes() noexcept;

    void CommitPropertyChange(SVGProperty *property) noexcept override;
    void CommitPropertyChange(SVGAnimatedProperty &animatedProperty) noexcept;

    KRYS_NODISCARD const SVGElement *AttributeContextElement() const noexcept override
    {
      return this;
    }

    // These are needed for the RenderTree, animation and DOM.
    KRYS_NODISCARD dom::DOMStringAtom ClassName() const noexcept;

    KRYS_NODISCARD SVGAnimatedString &ClassNameAnimated() noexcept
    {
      return *_className;
    }

    KRYS_NODISCARD SVGConditionalProcessingAttributes &ConditionalProcessingAttributes() noexcept;
    KRYS_NODISCARD SVGConditionalProcessingAttributes *
      ConditionalProcessingAttributesIfExists() const noexcept;

    void InvalidateInstances() noexcept;

    /// @brief The property a presentation attribute maps to, or `PropertyId::Invalid` if the attribute isn't
    /// one.
    /// @see https://svgwg.org/svg2-draft/styling.html#PresentationAttributes
    KRYS_NODISCARD static css::PropertyId
      CssPropertyIdForSVGAttributeName(dom::DOMStringAtom localName,
                                       dom::DOMStringAtom namespaceURI) noexcept;

  protected:
    void FinishParsingChildren() noexcept override;

    /// @brief The attribute change steps of SVG elements, run for every change to one of the element's
    /// attributes. Overrides parse the attributes they reflect, then call their base's.
    /// @param oldValue The previous value, or null if the attribute was added.
    /// @param value The new value, or null if the attribute was removed.
    virtual void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                  Maybe<dom::DOMStringView> value) noexcept;

    KRYS_NODISCARD SVGElementRareData &EnsureSVGRareData() noexcept;

    void ReportAttributeParsingError(SVGParsingError error, const dom::QualifiedName &name,
                                     dom::DOMStringView value) noexcept;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnChildrenChanged() noexcept override;

    KRYS_NODISCARD virtual bool SelfHasRelativeLengths() const noexcept
    {
      return false;
    }

    void UpdateRelativeLengthsInformation() noexcept;
    void UpdateRelativeLengthsInformationForChild(bool hasRelativeLengths, SVGElement &element) noexcept;

#pragma region GlobalEventHandlers, DocumentAndElementEventHandlers - https://html.spec.whatwg.org/#event-handler-attributes

    KRYS_GLOBAL_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)
    KRYS_DOCUMENT_AND_ELEMENT_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

#pragma region Type Checks

    KRYS_NODISCARD bool IsSVGCircleElement() const noexcept
    {
      return _interface == SVGElementInterface::Circle;
    }

    KRYS_NODISCARD bool IsSVGDefsElement() const noexcept
    {
      return _interface == SVGElementInterface::Defs;
    }

    KRYS_NODISCARD bool IsSVGDescElement() const noexcept
    {
      return _interface == SVGElementInterface::Desc;
    }

    KRYS_NODISCARD bool IsSVGEllipseElement() const noexcept
    {
      return _interface == SVGElementInterface::Ellipse;
    }

    KRYS_NODISCARD bool IsSVGForeignObjectElement() const noexcept
    {
      return _interface == SVGElementInterface::ForeignObject;
    }

    KRYS_NODISCARD bool IsSVGGElement() const noexcept
    {
      return _interface == SVGElementInterface::G;
    }

    KRYS_NODISCARD bool IsSVGGeometryElement() const noexcept
    {
      return HasFlag(_flags, SVGElementFlags::IsSvgGeometryElement);
    }

    KRYS_NODISCARD bool IsSVGGraphicsElement() const noexcept
    {
      return HasFlag(_flags, SVGElementFlags::IsSvgGraphicsElement);
    }

    KRYS_NODISCARD bool IsSVGImageElement() const noexcept
    {
      return _interface == SVGElementInterface::Image;
    }

    KRYS_NODISCARD bool IsSVGLineElement() const noexcept
    {
      return _interface == SVGElementInterface::Line;
    }

    KRYS_NODISCARD bool IsSVGMetadataElement() const noexcept
    {
      return _interface == SVGElementInterface::Metadata;
    }

    KRYS_NODISCARD bool IsSVGPolygonElement() const noexcept
    {
      return _interface == SVGElementInterface::Polygon;
    }

    KRYS_NODISCARD bool IsSVGPolylineElement() const noexcept
    {
      return _interface == SVGElementInterface::Polyline;
    }

    KRYS_NODISCARD bool IsSVGRectElement() const noexcept
    {
      return _interface == SVGElementInterface::Rect;
    }

    KRYS_NODISCARD bool IsSVGScriptElement() const noexcept
    {
      return _interface == SVGElementInterface::Script;
    }

    KRYS_NODISCARD bool IsSVGStyleElement() const noexcept
    {
      return _interface == SVGElementInterface::Style;
    }

    KRYS_NODISCARD bool IsSVGSvgElement() const noexcept
    {
      return _interface == SVGElementInterface::Svg;
    }

    KRYS_NODISCARD bool IsSVGSwitchElement() const noexcept
    {
      return _interface == SVGElementInterface::Switch;
    }

    KRYS_NODISCARD bool IsSVGSymbolElement() const noexcept
    {
      return _interface == SVGElementInterface::Symbol;
    }

    KRYS_NODISCARD bool IsSVGTitleElement() const noexcept
    {
      return _interface == SVGElementInterface::Title;
    }

    // NOTE: SVGUnknownElement type check is not needed here as Node already has it.

    KRYS_NODISCARD bool IsSVGUseElement() const noexcept
    {
      return _interface == SVGElementInterface::Use;
    }

#pragma endregion

  private:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value, dom::DOMStringAtom namespaceURI) noexcept final;

    virtual void ClearTarget() noexcept
    {
    }
  };

  class SVGElement::InstanceInvalidationGuard
  {
    KRYS_NON_COPY_MOVABLE(InstanceInvalidationGuard);

  private:
    Ref<SVGElement> _element;

  public:
    explicit InstanceInvalidationGuard(SVGElement &element) noexcept : _element(ShareRef(element))
    {
    }

    ~InstanceInvalidationGuard() noexcept
    {
      _element->InvalidateInstances();
    }
  };

  class SVGElement::InstanceUpdateBlocker
  {
    KRYS_NON_COPY_MOVABLE(InstanceUpdateBlocker);

  private:
    Ref<SVGElement> _element;

  public:
    explicit InstanceUpdateBlocker(SVGElement &element) noexcept : _element(ShareRef(element))
    {
      _element->SetInstanceUpdatesBlocked(true);
    }

    ~InstanceUpdateBlocker() noexcept
    {
      _element->SetInstanceUpdatesBlocked(false);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsSVGElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
