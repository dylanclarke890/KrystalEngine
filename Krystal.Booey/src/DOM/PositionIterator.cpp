#include "Krystal.Booey/DOM/PositionIterator.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::dom
{
  PositionIterator::PositionIterator(const Position &position) noexcept
      : _anchorNode(ShareRefPtr(position.AnchorNode())),
        _nodeAfterPositionInAnchor(ShareRefPtr(
          _anchorNode->TraverseToChildAt(static_cast<uint32>(position.DeprecatedEditingOffset())))),
        _offsetInAnchor(_nodeAfterPositionInAnchor ? 0 : position.DeprecatedEditingOffset())
  {
  }

  PositionIterator::operator Position() const noexcept
  {
    auto *anchorNode = GetNode();
    if (_nodeAfterPositionInAnchor)
    {
      krys_debug_assert(_nodeAfterPositionInAnchor->ParentNode() == anchorNode);
      // FIXME: This check is inadaquete because any ancestor could be ignored by editing
      if (PositionBeforeOrAfterNodeIsCandidate(*anchorNode))
      {
        return PositionBeforeNode(anchorNode);
      }
      return PositionInParentBeforeNode(_nodeAfterPositionInAnchor.get());
    }
    if (PositionBeforeOrAfterNodeIsCandidate(*anchorNode))
    {
      return AtStartOfNode() ? PositionBeforeNode(anchorNode) : PositionAfterNode(anchorNode);
    }
    if (anchorNode->HasChildNodes())
    {
      return LastPositionInOrAfterNode(anchorNode);
    }
    return MakeDeprecatedLegacyPosition(ShareRefPtr(anchorNode), static_cast<uint32>(_offsetInAnchor));
  }

  void PositionIterator::Increment() noexcept
  {
    if (!_anchorNode)
    {
      return;
    }

    if (_nodeAfterPositionInAnchor)
    {
      _anchorNode = _nodeAfterPositionInAnchor;
      _nodeAfterPositionInAnchor = ShareRefPtr(_anchorNode->FirstChild());
      _offsetInAnchor = 0;
      return;
    }

    auto *anchorNode = GetNode();
    if (anchorNode->Renderer() && !anchorNode->HasChildNodes()
        && _offsetInAnchor < LastOffsetForEditing(*anchorNode))
    {
      _offsetInAnchor = Position::UncheckedNextOffset(anchorNode, static_cast<uint32>(_offsetInAnchor));
    }
    else
    {
      _nodeAfterPositionInAnchor = _anchorNode;
      _anchorNode = ShareRefPtr<Node>(_nodeAfterPositionInAnchor->ParentNode());
      _nodeAfterPositionInAnchor = ShareRefPtr(_nodeAfterPositionInAnchor->NextSibling());
      _offsetInAnchor = 0;
    }
  }

  void PositionIterator::Decrement() noexcept
  {
    if (!_anchorNode)
    {
      return;
    }

    if (_nodeAfterPositionInAnchor)
    {
      _anchorNode = ShareRefPtr(_nodeAfterPositionInAnchor->PreviousSibling());
      if (auto *anchorNode = GetNode())
      {
        _nodeAfterPositionInAnchor = nullptr;
        _offsetInAnchor = anchorNode->HasChildNodes() ? 0 : LastOffsetForEditing(*anchorNode);
      }
      else
      {
        _nodeAfterPositionInAnchor = ShareRefPtr<Node>(_nodeAfterPositionInAnchor->ParentNode());
        _anchorNode = ShareRefPtr<Node>(_nodeAfterPositionInAnchor->ParentNode());
        _offsetInAnchor = 0;
      }
      return;
    }

    if (_anchorNode->HasChildNodes())
    {
      _anchorNode = ShareRefPtr(_anchorNode->LastChild());
      _offsetInAnchor = _anchorNode->HasChildNodes() ? 0 : LastOffsetForEditing(*_anchorNode);
    }
    else
    {
      if (_offsetInAnchor && _anchorNode->Renderer())
      {
        _offsetInAnchor =
          Position::UncheckedPreviousOffset(_anchorNode.get(), static_cast<uint32>(_offsetInAnchor));
      }
      else
      {
        _nodeAfterPositionInAnchor = _anchorNode;
        _anchorNode = ShareRefPtr<Node>(_anchorNode->ParentNode());
      }
    }
  }

  bool PositionIterator::AtStart() const noexcept
  {
    if (!_anchorNode)
    {
      return true;
    }
    if (_anchorNode->ParentNode())
    {
      return false;
    }
    return (!_anchorNode->HasChildNodes() && !_offsetInAnchor)
           || (_nodeAfterPositionInAnchor && !_nodeAfterPositionInAnchor->PreviousSibling());
  }

  bool PositionIterator::AtEnd() const noexcept
  {
    auto *anchorNode = GetNode();
    if (!anchorNode)
    {
      return true;
    }
    if (_nodeAfterPositionInAnchor)
    {
      return false;
    }
    return !anchorNode->ParentNode()
           && (anchorNode->HasChildNodes() || _offsetInAnchor >= LastOffsetForEditing(*anchorNode));
  }

  bool PositionIterator::AtStartOfNode() const noexcept
  {
    if (!_anchorNode)
    {
      return true;
    }
    if (!_nodeAfterPositionInAnchor)
    {
      return !_anchorNode->HasChildNodes() && !_offsetInAnchor;
    }
    return !_nodeAfterPositionInAnchor->PreviousSibling();
  }

  bool PositionIterator::AtEndOfNode() const noexcept
  {
    auto *anchorNode = GetNode();
    if (!anchorNode)
    {
      return true;
    }
    if (_nodeAfterPositionInAnchor)
    {
      return false;
    }
    return anchorNode->HasChildNodes() || _offsetInAnchor >= LastOffsetForEditing(*anchorNode);
  }

  // This function should be kept in sync with Position::IsCandidate().
  bool PositionIterator::IsCandidate() const noexcept
  {
    auto *anchorNode = GetNode();
    if (!anchorNode)
    {
      return false;
    }

    auto *renderer = anchorNode->Renderer();
    if (!renderer)
    {
      return false;
    }

    if (renderer->Style().GetVisibility() != Visibility::Visible)
    {
      return false;
    }

    if (renderer->IsBR())
    {
      return Position(*this).IsCandidate();
    }

    if (auto *renderText = DynamicDowncast<RenderText>(*renderer))
    {
      return !Position::NodeIsUserSelectNone(anchorNode)
             && renderText->ContainsCaretOffset(static_cast<uint32>(_offsetInAnchor));
    }

    if (PositionBeforeOrAfterNodeIsCandidate(*anchorNode))
    {
      return (AtStartOfNode() || AtEndOfNode()) && !Position::NodeIsUserSelectNone(anchorNode->ParentNode());
    }

    if (Is<html::HTMLHtmlElement>(*anchorNode))
    {
      return false;
    }

    if (auto *block = DynamicDowncast<RenderBlock>(*renderer))
    {
      if (Is<RenderBlockFlow>(*block) || Is<RenderGrid>(*block) || Is<RenderFlexibleBox>(*block))
      {
        if (block->LogicalHeight() || Is<html::HTMLBodyElement>(*anchorNode)
            || anchorNode->IsRootEditableElement())
        {
          if (!Position::HasRenderedNonAnonymousDescendantsWithHeight(*block))
          {
            return AtStartOfNode() && !Position::NodeIsUserSelectNone(anchorNode);
          }
          return anchorNode->HasEditableStyle() && !Position::NodeIsUserSelectNone(anchorNode)
                 && Position(*this).AtEditingBoundary();
        }
        return false;
      }
    }

    return anchorNode->HasEditableStyle() && !Position::NodeIsUserSelectNone(anchorNode)
           && Position(*this).AtEditingBoundary();
  }
}
