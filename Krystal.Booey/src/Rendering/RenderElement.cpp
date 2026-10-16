#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLMarqueeElement.hpp"

namespace krys::boo
{
  namespace
  {
    // FIXME: Stands in for the renderer classes (`RenderBlockFlow`, `RenderInline`, `RenderFlexibleBox`, ...)
    // until they're ported.
    class GenericRenderElement final : public RenderElement
    {
      KRYS_NON_COPY_MOVABLE(GenericRenderElement);
      KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(GenericRenderElement);

    public:
      GenericRenderElement(Type type, dom::Element &element, RenderStyle &&style) noexcept
          : RenderElement(type, element, krys::move(style), TypeFlagsFor(type), {})
      {
      }

      KRYS_NODISCARD bool CanHaveChildren() const noexcept override
      {
        return true;
      }

      KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
      {
        switch (GetType())
        {
          case Type::BlockFlow:             return u8"RenderBlockFlow";
          case Type::Inline:                return u8"RenderInline";
          case Type::ListItem:              return u8"RenderListItem";
          case Type::FlexibleBox:           return u8"RenderFlexibleBox";
          case Type::Grid:                  return u8"RenderGrid";
          case Type::DeprecatedFlexibleBox: return u8"RenderDeprecatedFlexibleBox";
          case Type::Table:                 return u8"RenderTable";
          case Type::TableCell:             return u8"RenderTableCell";
          case Type::TableCaption:          return u8"RenderTableCaption";
          case Type::TableSection:          return u8"RenderTableSection";
          case Type::TableRow:              return u8"RenderTableRow";
          case Type::TableCol:              return u8"RenderTableCol";
          default:                          return u8"RenderElement";
        }
      }

    private:
      // The flags the constructors of the renderer classes between `RenderElement` and this type's class add.
      KRYS_NODISCARD static EnumSet<TypeFlag> TypeFlagsFor(Type type) noexcept
      {
        const EnumSet<TypeFlag> box {TypeFlag::IsLayerModelObject, TypeFlag::IsBoxModelObject,
                                     TypeFlag::IsBox};
        switch (type)
        {
          case Type::Inline:
            return {TypeFlag::IsLayerModelObject, TypeFlag::IsBoxModelObject, TypeFlag::IsRenderInline};
          case Type::FlexibleBox:  return box | TypeFlag::IsRenderBlock | TypeFlag::IsFlexibleBox;
          case Type::TableSection:
          case Type::TableRow:
          case Type::TableCol:     return box;
          default:                 return box | TypeFlag::IsRenderBlock;
        }
      }
    };

    KRYS_NODISCARD RenderPtr<RenderElement>
      CreateGenericRenderer(RenderObject::Type type, dom::Element &element, RenderStyle &&style) noexcept
    {
      return CreateRenderer<GenericRenderElement>(type, element, krys::move(style));
    }
  }

  RenderElement::RenderElement(Type type, dom::ContainerNode &elementOrDocument, RenderStyle &&style,
                               EnumSet<TypeFlag> flags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderObject(type, elementOrDocument, flags, typeSpecificFlags), _style(krys::move(style))
  {
  }

  RenderElement::RenderElement(Type type, dom::Element &element, RenderStyle &&style,
                               EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderElement(type, static_cast<dom::ContainerNode &>(element), krys::move(style), baseTypeFlags,
                      typeSpecificFlags)
  {
  }

  RenderElement::RenderElement(Type type, dom::Document &document, RenderStyle &&style,
                               EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderElement(type, static_cast<dom::ContainerNode &>(document), krys::move(style), baseTypeFlags,
                      typeSpecificFlags)
  {
  }

  RenderElement::~RenderElement() noexcept
  {
    // Do not add any code here. Add it to willBeDestroyed() instead.
    krys_debug_assert(!_firstChild);
  }

  RenderPtr<RenderElement>
    RenderElement::CreateFor(dom::Element &element, RenderStyle &&style,
                             EnumSet<ConstructBlockLevelRendererFor> rendererTypeOverride) noexcept
  {
    // FIXME: Generated content images (`content: url(...)`) when there's no override, once there are images.
    switch (style.Display())
    {
      case DisplayType::None:
      case DisplayType::Contents: return nullptr;
      case DisplayType::Inline:
        if (rendererTypeOverride.Contains(ConstructBlockLevelRendererFor::Inline))
        {
          return CreateGenericRenderer(Type::BlockFlow, element, krys::move(style));
        }
        return CreateGenericRenderer(Type::Inline, element, krys::move(style));
      case DisplayType::Block:
      case DisplayType::FlowRoot:
      case DisplayType::InlineBlock:
        return CreateGenericRenderer(Type::BlockFlow, element, krys::move(style));
      case DisplayType::ListItem:
        if (rendererTypeOverride.Contains(ConstructBlockLevelRendererFor::ListItem))
        {
          return CreateGenericRenderer(Type::BlockFlow, element, krys::move(style));
        }
        return CreateGenericRenderer(Type::ListItem, element, krys::move(style));
      case DisplayType::Flex:
      case DisplayType::InlineFlex:
        return CreateGenericRenderer(Type::FlexibleBox, element, krys::move(style));
      case DisplayType::Grid:
      case DisplayType::InlineGrid:
      case DisplayType::GridLanes:
      case DisplayType::InlineGridLanes: return CreateGenericRenderer(Type::Grid, element, krys::move(style));
      case DisplayType::Box:
      case DisplayType::InlineBox:
        return CreateGenericRenderer(Type::DeprecatedFlexibleBox, element, krys::move(style));
      case DisplayType::RubyBase: return CreateGenericRenderer(Type::Inline, element, krys::move(style));
      case DisplayType::RubyAnnotation:
        return CreateGenericRenderer(Type::BlockFlow, element, krys::move(style));
      case DisplayType::Ruby:      return CreateGenericRenderer(Type::Inline, element, krys::move(style));
      case DisplayType::RubyBlock: return CreateGenericRenderer(Type::BlockFlow, element, krys::move(style));
      default:                     break;
    }

    if (style.IsDisplayTableOrTablePart()
        && rendererTypeOverride.Contains(ConstructBlockLevelRendererFor::TableOrTablePart))
    {
      return CreateGenericRenderer(Type::BlockFlow, element, krys::move(style));
    }

    switch (style.Display())
    {
      case DisplayType::Table:
      case DisplayType::InlineTable: return CreateGenericRenderer(Type::Table, element, krys::move(style));
      case DisplayType::TableCell:   return CreateGenericRenderer(Type::TableCell, element, krys::move(style));
      case DisplayType::TableCaption:
        return CreateGenericRenderer(Type::TableCaption, element, krys::move(style));
      case DisplayType::TableRowGroup:
      case DisplayType::TableHeaderGroup:
      case DisplayType::TableFooterGroup:
        return CreateGenericRenderer(Type::TableSection, element, krys::move(style));
      case DisplayType::TableRow:         return CreateGenericRenderer(Type::TableRow, element, krys::move(style));
      case DisplayType::TableColumnGroup:
      case DisplayType::TableColumn:      return CreateGenericRenderer(Type::TableCol, element, krys::move(style));
      default:                            break;
    }
    krys_debug_assert(false);
    return nullptr;
  }

  style::Difference RenderElement::AdjustStyleDifference(style::Difference diff) const noexcept
  {
    // FIXME: Renderers have no layers yet (hasLayer() is false), so the composited cases and the layer status
    // changes of plug-ins, iframes and canvas don't apply.

    // If transform changed, and we are not composited, need to do a layout.
    if (diff.ContextSensitiveProperties.Contains(style::DifferenceContextSensitiveProperty::Transform))
    {
      // FIXME: when transforms are taken into account for overflow, we will need to do a layout.
      diff.Result = std::max(diff.Result, style::DifferenceResult::Layout);
    }

    if (diff.ContextSensitiveProperties.Contains(style::DifferenceContextSensitiveProperty::Opacity))
    {
      diff.Result = std::max(diff.Result, style::DifferenceResult::RepaintLayer);
    }

    if (diff.ContextSensitiveProperties.Contains(style::DifferenceContextSensitiveProperty::ClipPath))
    {
      diff.Result = std::max(diff.Result, style::DifferenceResult::Repaint);
    }

    if (diff.ContextSensitiveProperties.Contains(style::DifferenceContextSensitiveProperty::WillChange))
    {
      if (Style().GetWillChange().CanTriggerCompositing())
      {
        diff.Result = std::max(diff.Result, style::DifferenceResult::RecompositeLayer);
      }
    }

    if (IsHTMLMarquee())
    {
      diff.Result = std::max(diff.Result, style::DifferenceResult::Layout);
    }

    // If we have no layer(), just treat a RepaintLayer hint as a normal Repaint.
    if (diff == style::DifferenceResult::RepaintLayer)
    {
      diff.Result = style::DifferenceResult::Repaint;
    }

    return diff;
  }

  bool RenderElement::IsHTMLMarquee() const noexcept
  {
    auto *element = GetElement();
    if (!element || element->Renderer() != this)
    {
      return false;
    }
    auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element);
    return htmlElement && Is<html::HTMLMarqueeElement>(*htmlElement);
  }

  void RenderElement::InitializeStyle() noexcept
  {
    // FIXME: Pending resources (Style::loadPendingResources), styleWillChange and styleDidChange with
    // DifferenceResult::NewStyle, and view transitions, once there are any.
    _hasInitializedStyle = true;
  }

  void RenderElement::SetStyle(RenderStyle &&style, style::DifferenceResult minimalStyleDifference) noexcept
  {
    // FIXME: Should change RenderView so it can use initializeStyle too.
    // If we do that, we can assert m_hasInitializedStyle unconditionally,
    // and remove the check of m_hasInitializedStyle below too.
    krys_debug_assert(_hasInitializedStyle || IsRenderView());

    auto diff = style::Difference {};
    if (_hasInitializedStyle)
    {
      diff = style::GetDifference(_style, style);
    }

    diff.Result = std::max(diff.Result, minimalStyleDifference);
    diff = AdjustStyleDifference(diff);

    // FIXME: Pending resources, the repaint before the change, styleWillChange, styleDidChange (also of the
    // text children), and the layout and repaint the difference asks for, once there's layout and painting.
    (void)diff;
    _style = krys::move(style);
  }

  dom::Element *RenderElement::GetElement() const noexcept
  {
    return static_cast<dom::Element *>(GetNode());
  }

  void RenderElement::DidAttachChild(RenderObject &, RenderObject *) noexcept
  {
    // FIXME: Tell text children their style changed, and remove the layers created in hidden SVG subtrees,
    // once there are text renderers and layers.
  }

  RenderObject *RenderElement::AttachRendererInternal(RenderPtr<RenderObject> child,
                                                      RenderObject *beforeChild) noexcept
  {
    child->SetParent(this);

    if (_firstChild == beforeChild)
    {
      _firstChild = child.get();
    }

    if (beforeChild)
    {
      auto *previousSibling = beforeChild->PreviousSibling();
      if (previousSibling)
      {
        previousSibling->SetNextSibling(child.get());
      }
      child->SetPreviousSibling(previousSibling);
      child->SetNextSibling(beforeChild);
      beforeChild->SetPreviousSibling(child.get());
      return child.release();
    }
    {
      auto *lastChild = _lastChild;
      if (lastChild)
      {
        lastChild->SetNextSibling(child.get());
      }
      child->SetPreviousSibling(lastChild);
    }
    _lastChild = child.get();
    return child.release();
  }

  RenderPtr<RenderObject> RenderElement::DetachRendererInternal(RenderObject &renderer) noexcept
  {
    auto *parent = renderer.Parent();
    krys_debug_assert(parent);
    auto *nextSibling = renderer.NextSibling();

    if (auto *previousSibling = renderer.PreviousSibling())
    {
      previousSibling->SetNextSibling(nextSibling);
    }
    if (nextSibling)
    {
      nextSibling->SetPreviousSibling(renderer.PreviousSibling());
    }

    if (parent->FirstChild() == &renderer)
    {
      parent->_firstChild = nextSibling;
    }
    if (parent->LastChild() == &renderer)
    {
      parent->_lastChild = renderer.PreviousSibling();
    }

    renderer.SetPreviousSibling(nullptr);
    renderer.SetNextSibling(nullptr);
    renderer.SetParent(nullptr);
    return RenderPtr<RenderObject>(&renderer);
  }

  void RenderElement::InsertedIntoTree() noexcept
  {
    // FIXME: Keep the layer hierarchy updated and the parent layer's visible content status, once there are
    // layers.
    RenderObject::InsertedIntoTree();
  }

  void RenderElement::WillBeRemovedFromTree() noexcept
  {
    // FIXME: The enclosing layer's visible content status and the layer hierarchy, once there are layers.
    RenderObject::WillBeRemovedFromTree();
  }

  void RenderElement::WillBeDestroyed() noexcept
  {
    // FIXME: Slow repaint objects, visibility callbacks, counter nodes, the subtree layout root, image
    // clients, outlines, paused image animations and the other upstream clean-up, once there are any.
    RenderObject::WillBeDestroyed();
  }
}
