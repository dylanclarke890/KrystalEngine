#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class Document;
  class Node;
}

namespace krys::boo
{
  enum class RenderObjectTypeFlag : uint8
  {
    IsAnonymous,
    IsText,
    IsBox,
    IsBoxModelObject,
    IsLayerModelObject,
    IsRenderInline,
    IsRenderBlock,
    IsFlexibleBox
  };

  enum class RenderObjectBlockFlowFlag : uint8
  {
    IsFragmentContainer,
    IsFragmentedFlow,
    IsTextControl,
    IsSVGBlock,
    IsViewTransitionContainingBlock
  };

  enum class RenderObjectLineBreakFlag : uint8
  {
    IsWBR
  };

  enum class RenderObjectReplacedFlag : uint8
  {
    IsImage,
    IsMedia,
    IsWidget,
    IsViewTransitionCapture,
    UsesBoundaryCaching
  };

  enum class RenderObjectSVGModelObjectFlag : uint8
  {
    IsLegacy,
    IsContainer,
    IsHiddenContainer,
    IsResourceContainer,
    IsShape,
    UsesBoundaryCaching
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectTypeFlag, 8uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectBlockFlowFlag, 5uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectLineBreakFlag, 1uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectReplacedFlag, 5uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderObjectSVGModelObjectFlag, 6uz);

namespace krys::boo
{
  class RenderElement;
  class RenderStyle;

  /// @brief A node of the render tree: the box, text or other object a node (or, for anonymous objects, the
  /// document) is rendered as.
  /// @note FIXME: There's no render tree yet: renderers have no parent, children, layout or painting.
  class RenderObject : public CanMakeCheckedPtr<RenderObject>
  {
    KRYS_NON_COPY_MOVABLE(RenderObject);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderObject);

  public:
    enum class Type : uint8
    {
      BlockFlow,
      Button,
      CombineText,
      Counter,
      DeprecatedFlexibleBox,
      DetailsMarker,
      EmbeddedObject,
      FileUploadControl,
      FlexibleBox,
      Frame,
      FrameSet,
      Grid,
      HTMLCanvas,
      IFrame,
      Image,
      Inline,
      LineBreak,
      ListBox,
      ListItem,
      ListMarker,
      Media,
      MenuList,
      Meter,
      MultiColumnFlow,
      MultiColumnSet,
      MultiColumnSpannerPlaceholder,
      Progress,
      Quote,
      Replica,
      ScrollbarPart,
      SearchField,
      Slider,
      SliderContainer,
      Table,
      TableCaption,
      TableCell,
      TableCol,
      TableRow,
      TableSection,
      Text,
      TextControlInnerBlock,
      TextControlInnerContainer,
      TextControlMultiLine,
      TextControlSingleLine,
      TextFragment,
      VTTCue,
      Video,
      View,
      ViewTransitionCapture,
      MathMLBlock,
      MathMLFenced,
      MathMLFencedOperator,
      MathMLFraction,
      MathMLMath,
      MathMLMenclose,
      MathMLOperator,
      MathMLPadded,
      MathMLRoot,
      MathMLRow,
      MathMLScripts,
      MathMLSpace,
      MathMLTable,
      MathMLToken,
      MathMLUnderOver,
      SVGEllipse,
      SVGForeignObject,
      SVGGradientStop,
      SVGHiddenContainer,
      SVGImage,
      SVGInline,
      SVGInlineText,
      SVGPath,
      SVGRect,
      SVGResourceClipper,
      SVGResourceFilter,
      SVGResourceFilterPrimitive,
      SVGResourceLinearGradient,
      SVGResourceMarker,
      SVGResourceMasker,
      SVGResourcePattern,
      SVGResourceRadialGradient,
      SVGRoot,
      SVGTSpan,
      SVGText,
      SVGTextPath,
      SVGTransformableContainer,
      SVGViewportContainer,
      LegacySVGEllipse,
      LegacySVGForeignObject,
      LegacySVGHiddenContainer,
      LegacySVGImage,
      LegacySVGPath,
      LegacySVGRect,
      LegacySVGResourceClipper,
      LegacySVGResourceFilter,
      LegacySVGResourceFilterPrimitive,
      LegacySVGResourceLinearGradient,
      LegacySVGResourceMarker,
      LegacySVGResourceMasker,
      LegacySVGResourcePattern,
      LegacySVGResourceRadialGradient,
      LegacySVGRoot,
      LegacySVGTransformableContainer,
      LegacySVGViewportContainer
    };

    using TypeFlag = RenderObjectTypeFlag;
    using BlockFlowFlag = RenderObjectBlockFlowFlag;
    using LineBreakFlag = RenderObjectLineBreakFlag;
    using ReplacedFlag = RenderObjectReplacedFlag;
    using SVGModelObjectFlag = RenderObjectSVGModelObjectFlag;

    /// @brief The flags of one kind of renderer.
    class TypeSpecificFlags
    {
    public:
      enum class Kind : uint8
      {
        Invalid = 0,
        BlockFlow,
        LineBreak,
        Replaced,
        SVGModelObject
      };

    private:
      Kind _kind {Kind::Invalid};
      uint8 _flags {0};

    public:
      constexpr TypeSpecificFlags() noexcept = default;

      constexpr TypeSpecificFlags(EnumSet<BlockFlowFlag> flags) noexcept
          : _kind(Kind::BlockFlow), _flags(static_cast<uint8>(flags.ToRaw()))
      {
      }

      constexpr TypeSpecificFlags(EnumSet<LineBreakFlag> flags) noexcept
          : _kind(Kind::LineBreak), _flags(static_cast<uint8>(flags.ToRaw()))
      {
      }

      constexpr TypeSpecificFlags(EnumSet<ReplacedFlag> flags) noexcept
          : _kind(Kind::Replaced), _flags(static_cast<uint8>(flags.ToRaw()))
      {
      }

      constexpr TypeSpecificFlags(EnumSet<SVGModelObjectFlag> flags) noexcept
          : _kind(Kind::SVGModelObject), _flags(static_cast<uint8>(flags.ToRaw()))
      {
      }

      KRYS_NODISCARD constexpr Kind GetKind() const noexcept
      {
        return _kind;
      }

      KRYS_NODISCARD constexpr EnumSet<BlockFlowFlag> BlockFlowFlags() const noexcept
      {
        return EnumSet<BlockFlowFlag>::FromRaw(ValueForKind(Kind::BlockFlow));
      }
      KRYS_NODISCARD constexpr EnumSet<LineBreakFlag> LineBreakFlags() const noexcept
      {
        return EnumSet<LineBreakFlag>::FromRaw(ValueForKind(Kind::LineBreak));
      }
      KRYS_NODISCARD constexpr EnumSet<ReplacedFlag> ReplacedFlags() const noexcept
      {
        return EnumSet<ReplacedFlag>::FromRaw(ValueForKind(Kind::Replaced));
      }
      KRYS_NODISCARD constexpr EnumSet<SVGModelObjectFlag> SVGFlags() const noexcept
      {
        return EnumSet<SVGModelObjectFlag>::FromRaw(ValueForKind(Kind::SVGModelObject));
      }

    private:
      KRYS_NODISCARD constexpr uint8 ValueForKind(Kind kind) const noexcept
      {
        return _kind == kind ? _flags : 0;
      }
    };

  private:
    dom::Node &_node;
    // FIXME: The parent, siblings and children come with the render tree.
    RenderElement *_parent {nullptr};
    EnumSet<TypeFlag> _typeFlags;
    Type _type;
    TypeSpecificFlags _typeSpecificFlags;

  public:
    /// @brief Anonymous objects should pass the document as their node, and they will then automatically be
    /// marked as anonymous in the constructor.
    RenderObject(Type type, dom::Node &node, EnumSet<TypeFlag> typeFlags,
                 TypeSpecificFlags typeSpecificFlags) noexcept;
    virtual ~RenderObject() noexcept;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD virtual utf8_stringview RenderName() const noexcept = 0;

    KRYS_NODISCARD RenderElement *Parent() const noexcept
    {
      return _parent;
    }

    KRYS_NODISCARD bool IsRenderElement() const noexcept
    {
      return !IsRenderText();
    }
    KRYS_NODISCARD bool IsRenderText() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsText);
    }
    KRYS_NODISCARD bool IsRenderBoxModelObject() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsBoxModelObject);
    }
    KRYS_NODISCARD bool IsRenderBlock() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsRenderBlock);
    }
    KRYS_NODISCARD bool IsRenderInline() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsRenderInline);
    }
    KRYS_NODISCARD bool IsRenderBox() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsBox);
    }
    KRYS_NODISCARD bool IsRenderGrid() const noexcept
    {
      return _type == Type::Grid;
    }
    KRYS_NODISCARD bool IsRenderView() const noexcept
    {
      return _type == Type::View;
    }
    KRYS_NODISCARD bool IsLegacyRenderSVGModelObject() const noexcept
    {
      return _typeSpecificFlags.GetKind() == TypeSpecificFlags::Kind::SVGModelObject
             && _typeSpecificFlags.SVGFlags().Contains(SVGModelObjectFlag::IsLegacy);
    }
    KRYS_NODISCARD bool IsRenderSVGModelObject() const noexcept
    {
      return _typeSpecificFlags.GetKind() == TypeSpecificFlags::Kind::SVGModelObject
             && !_typeSpecificFlags.SVGFlags().Contains(SVGModelObjectFlag::IsLegacy);
    }
    KRYS_NODISCARD bool IsRenderOrLegacyRenderSVGModelObject() const noexcept
    {
      return IsRenderSVGModelObject() || IsLegacyRenderSVGModelObject();
    }

    KRYS_NODISCARD bool IsAnonymous() const noexcept
    {
      return _typeFlags.Contains(TypeFlag::IsAnonymous);
    }

    /// @brief The node rendered, or null for an anonymous object.
    KRYS_NODISCARD dom::Node *GetNode() const noexcept
    {
      if (IsAnonymous())
      {
        return nullptr;
      }
      return &_node;
    }

    KRYS_NODISCARD dom::Node &NodeForNonAnonymous() const noexcept
    {
      krys_debug_assert(!IsAnonymous());
      return _node;
    }

    KRYS_NODISCARD dom::Document &GetDocument() const noexcept;

    /// @brief The style the object is rendered with: an element's own, or a text's parent's.
    KRYS_NODISCARD const RenderStyle &Style() const noexcept;

  protected:
    KRYS_NODISCARD EnumSet<TypeFlag> TypeFlags() const noexcept
    {
      return _typeFlags;
    }
    KRYS_NODISCARD TypeSpecificFlags GetTypeSpecificFlags() const noexcept
    {
      return _typeSpecificFlags;
    }
  };
}
