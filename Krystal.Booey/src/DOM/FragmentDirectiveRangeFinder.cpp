#include "Krystal.Booey/DOM/FragmentDirectiveRangeFinder.hpp"
#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/HTML/HTMLAudioElement.hpp"
#include "Krystal.Booey/HTML/HTMLIFrameElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLMeterElement.hpp"
#include "Krystal.Booey/HTML/HTMLObjectElement.hpp"
#include "Krystal.Booey/HTML/HTMLProgressElement.hpp"
#include "Krystal.Booey/HTML/HTMLScriptElement.hpp"
#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/HTML/HTMLVideoElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <compare>
#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/utf8.h>

namespace krys::boo::dom
{
  namespace
  {
    enum class BoundaryPointIsAtEnd : bool
    {
      No,
      Yes
    };

    enum class WordBounded : bool
    {
      No,
      Yes
    };

    // https://wicg.github.io/scroll-to-text-fragment/#search-invisible
    KRYS_NODISCARD bool IsSearchInvisible(const Node &node) noexcept
    {
      auto *renderStyle = node.GetRenderStyle();
      if (!renderStyle || renderStyle->EffectiveDisplay() == DisplayType::None)
      {
        return true;
      }

      // FIXME: If the node serializes as void.

      if (auto *element = DynamicDowncast<html::HTMLElement>(node))
      {
        if (Is<html::HTMLIFrameElement>(*element) || Is<html::HTMLImageElement>(*element)
            || Is<html::HTMLMeterElement>(*element) || Is<html::HTMLObjectElement>(*element)
            || Is<html::HTMLProgressElement>(*element) || Is<html::HTMLStyleElement>(*element)
            || Is<html::HTMLScriptElement>(*element) || Is<html::HTMLVideoElement>(*element)
            || Is<html::HTMLAudioElement>(*element))
        {
          return true;
        }
      }

      // FIXME: Is a select element whose multiple content attribute is absent.

      return false;
    }

    // https://wicg.github.io/scroll-to-text-fragment/#non-searchable-subtree
    KRYS_NODISCARD bool IsNonSearchableSubtree(const Node &node) noexcept
    {
      for (const Node *traversingNode = &node; traversingNode;
           traversingNode = traversingNode->ParentOrShadowHostNode())
      {
        if (IsSearchInvisible(*traversingNode))
        {
          return true;
        }
      }
      return false;
    }

    // https://wicg.github.io/scroll-to-text-fragment/#get-boundary-point-at-index
    KRYS_NODISCARD Maybe<BoundaryPoint> BoundaryPointAtIndexInNodes(uint32 index,
                                                                    const List<Ref<Text>> &textNodeList,
                                                                    BoundaryPointIsAtEnd isEnd) noexcept
    {
      uint32 currentLength = 0;

      for (auto &currentNode : textNodeList)
      {
        auto nodeEnd = currentLength + static_cast<uint32>(currentNode->Length());
        if (isEnd == BoundaryPointIsAtEnd::Yes)
        {
          nodeEnd++;
        }
        if (nodeEnd > index)
        {
          return BoundaryPoint {ShareRef<Node>(*currentNode), index - currentLength};
        }
        currentLength += static_cast<uint32>(currentNode->Length());
      }
      return std::nullopt;
    }

    // https://wicg.github.io/scroll-to-text-fragment/#is-at-a-word-boundary
    KRYS_NODISCARD bool IndexIsAtWordBoundary(utf8_stringview string, uint32 index) noexcept
    {
      auto *iterator = WordBreakIterator(string);
      return iterator && ubrk_isBoundary(iterator, static_cast<int32_t>(index));
    }

    // https://wicg.github.io/scroll-to-text-fragment/#visible-text-node
    KRYS_NODISCARD bool IsVisibleTextNode(const Node &node) noexcept
    {
      if (auto *renderText = DynamicDowncast<RenderText>(node.Renderer()))
      {
        return renderText->Style().UsedVisibility() == Visibility::Visible;
      }
      return false;
    }

    /// @brief The first offset at or after @p start where @p query occurs in @p buffer, ignoring ASCII case.
    KRYS_NODISCARD size_t FindIgnoringASCIICase(utf8_stringview buffer, utf8_stringview query,
                                                size_t start) noexcept
    {
      if (query.size() > buffer.size())
      {
        return utf8_stringview::npos;
      }
      for (auto index = start; index + query.size() <= buffer.size(); ++index)
      {
        if (text::IsASCIICaselessEqual(buffer.substr(index, query.size()), query))
        {
          return index;
        }
      }
      return utf8_stringview::npos;
    }

    // https://wicg.github.io/scroll-to-text-fragment/#find-a-range-from-a-node-list
    KRYS_NODISCARD Maybe<SimpleRange>
      FindRangeFromNodeList(utf8_stringview query, const SimpleRange &searchRange, List<Ref<Text>> &nodes,
                            WordBounded wordStartBounded, WordBounded wordEndBounded) noexcept
    {
      if (nodes.empty())
      {
        return std::nullopt;
      }

      utf8_string searchBufferBuilder;
      for (auto &node : nodes)
      {
        searchBufferBuilder += node->Data();
      }

      // The search runs over the buffer with its quotation marks folded; offsets map back to the buffer.
      List<uint32> sourceOffsets;
      auto searchBuffer = FoldQuoteMarks(searchBufferBuilder, &sourceOffsets);
      auto foldedQuery = FoldQuoteMarks(query);

      // FIXME: The plain text search over the whole range first (findPlainText, with its collation), once
      // there's find in page.

      uint32 searchStart = 0;

      if (nodes[0].get() == &searchRange.StartContainer())
      {
        searchStart = static_cast<uint32>(searchRange.StartOffset());
      }

      Maybe<BoundaryPoint> start;
      Maybe<BoundaryPoint> end;

      Maybe<uint32> matchIndex;
      Maybe<uint32> matchEnd;

      // The folded offset of the original offset searchStart.
      size_t foldedSearchStart = 0;
      while (foldedSearchStart < sourceOffsets.size() - 1 && sourceOffsets[foldedSearchStart] < searchStart)
      {
        ++foldedSearchStart;
      }

      while (!matchIndex)
      {
        // FIXME: find only using the base characters i.e. also fold accents and others.
        auto potentialIndex = FindIgnoringASCIICase(searchBuffer, foldedQuery, foldedSearchStart);

        if (potentialIndex == utf8_stringview::npos)
        {
          return std::nullopt;
        }
        matchIndex = sourceOffsets[potentialIndex];
        matchEnd = sourceOffsets[potentialIndex + foldedQuery.size()];

        start = BoundaryPointAtIndexInNodes(*matchIndex, nodes, BoundaryPointIsAtEnd::No);
        end = BoundaryPointAtIndexInNodes(*matchEnd, nodes, BoundaryPointIsAtEnd::Yes);

        if ((wordStartBounded == WordBounded::Yes && !IndexIsAtWordBoundary(searchBufferBuilder, *matchIndex))
            || (wordEndBounded == WordBounded::Yes && !IndexIsAtWordBoundary(searchBufferBuilder, *matchEnd)))
        {
          foldedSearchStart = potentialIndex + 1;
          matchIndex = std::nullopt;
        }
      }

      uint32 endInset = 0;
      if (nodes.back().get() == &searchRange.EndContainer())
      {
        endInset = static_cast<uint32>(searchRange.EndContainer().Length() - searchRange.EndOffset());
      }

      if (*matchEnd > searchBufferBuilder.size() - endInset)
      {
        return std::nullopt;
      }

      krys_debug_assert_msg(start, "Scroll To Text Fragment: Start cannot be null");
      krys_debug_assert_msg(end, "Scroll To Text Fragment: End cannot be null");

      if (!start || !end || !Contains(searchRange, *start) || !Contains(searchRange, *end))
      {
        return std::nullopt;
      }

      return SimpleRange {*start, *end};
    }

    // https://wicg.github.io/scroll-to-text-fragment/#find-a-string-in-range
    KRYS_NODISCARD Maybe<SimpleRange> RangeOfStringInRange(utf8_stringview query, SimpleRange &searchRange,
                                                           WordBounded wordStartBounded,
                                                           WordBounded wordEndBounded) noexcept
    {
      while (!searchRange.Collapsed())
      {
        RefPtr<Node> currentNode = ShareRefPtr(&searchRange.StartContainer());

        if (IsNonSearchableSubtree(*currentNode))
        {
          if (auto *newStart = TreeQueries::NextSkippingChildren(*currentNode))
          {
            searchRange.Start = {ShareRef(*newStart), 0};
          }
          else
          {
            return std::nullopt;
          }
          continue;
        }

        if (!IsVisibleTextNode(*currentNode))
        {
          do
          {
            if (auto *newStart = TreeQueries::Next(*currentNode))
            {
              searchRange.Start = {ShareRef(*newStart), 0};
              currentNode = ShareRefPtr(newStart);
            }
            else
            {
              return std::nullopt;
            }
          } while (Is<DocumentType>(*currentNode));
          continue;
        }

        Ref<ContainerNode> blockAncestor =
          ShareRef(FragmentDirectiveUtilities::NearestBlockAncestor(*currentNode));
        List<Ref<Text>> textNodeList;
        // FIXME: this is O^2 since treeOrder will also do traversal, optimize.
        while (currentNode && currentNode->IsDescendantOf(*blockAncestor)
               && std::is_lteq(TreeOrder(BoundaryPoint {ShareRef(*currentNode), 0}, searchRange.End)))
        {
          if (auto *renderElement = DynamicDowncast<RenderElement>(currentNode->Renderer());
              renderElement && renderElement->Style().IsDisplayBlockLevel())
          {
            break;
          }

          if (IsSearchInvisible(*currentNode))
          {
            currentNode = ShareRefPtr(TreeQueries::NextSkippingChildren(*currentNode));
            continue;
          }
          if (auto *textNode = DynamicDowncast<Text>(*currentNode); textNode && IsVisibleTextNode(*textNode))
          {
            textNodeList.push_back(ShareRef(*textNode));
          }
          currentNode = ShareRefPtr(TreeQueries::Next(*currentNode));
        }

        if (auto range =
              FindRangeFromNodeList(query, searchRange, textNodeList, wordStartBounded, wordEndBounded))
        {
          return range;
        }

        if (!currentNode)
        {
          break;
        }

        if (!std::is_gt(TreeOrder(*currentNode, *searchRange.Start.Container)))
        {
          return std::nullopt;
        }

        searchRange.Start = {ShareRef(*currentNode), 0};
      }
      return std::nullopt;
    }

    // https://wicg.github.io/scroll-to-text-fragment/#next-non-whitespace-position
    KRYS_NODISCARD Maybe<SimpleRange> AdvanceRangeStartToNextNonWhitespace(SimpleRange &range) noexcept
    {
      auto newRange = range;
      while (!newRange.Collapsed())
      {
        Ref<Node> node = ShareRef(newRange.StartContainer());
        auto offset = static_cast<size_t>(newRange.StartOffset());

        // This check is not in the spec.
        // I believe there is an error in the spec which I have filed an issue for
        // https://github.com/WICG/scroll-to-text-fragment/issues/189
        if (offset == node->Length())
        {
          if (auto *newStart = TreeQueries::Next(*node))
          {
            newRange.Start = {ShareRef(*newStart), 0};
            continue;
          }
          return newRange;
        }

        if (IsNonSearchableSubtree(*node))
        {
          if (auto *newStart = TreeQueries::Next(*node))
          {
            newRange.Start = {ShareRef(*newStart), 0};
          }
          else
          {
            return newRange;
          }
          continue;
        }

        if (!IsVisibleTextNode(*node))
        {
          if (auto *newStart = TreeQueries::Next(*node))
          {
            newRange.Start = {ShareRef(*newStart), 0};
          }
          else
          {
            return newRange;
          }
          continue;
        }

        auto string = node->TextContent().value_or(DOMString());

        if (utf8_stringview(string).substr(offset, 6) == u8"&nbsp;")
        {
          offset += 6;
        }

        if (utf8_stringview(string).substr(offset, 5) == u8"&nbsp")
        {
          offset += 5;
        }

        if (offset >= string.size())
        {
          return newRange;
        }

        auto index = static_cast<int32_t>(offset);
        UChar32 character = 0;
        U8_NEXT(reinterpret_cast<const uint8_t *>(string.data()), index, static_cast<int32_t>(string.size()),
                character);
        if (character < 0 || !u_isUWhiteSpace(character))
        {
          return newRange;
        }
        offset = static_cast<size_t>(index);

        if (offset >= node->Length())
        {
          if (auto *newStart = TreeQueries::Next(*node))
          {
            newRange.Start = {ShareRef(*newStart), 0};
          }
          else
          {
            return newRange;
          }
        }
        else
        {
          newRange.Start = {node, offset};
        }
      }
      return newRange;
    }
  }

  // https://wicg.github.io/scroll-to-text-fragment/#find-a-range-from-a-text-directive
  Maybe<SimpleRange>
    FragmentDirectiveRangeFinder::FindRangeFromTextDirective(const ParsedTextDirective &parsedTextDirective,
                                                             Document &document) noexcept
  {
    auto searchRange = MakeRangeSelectingNodeContents(document);
    Maybe<SimpleRange> matchRange;
    auto mustEndAtWordBoundary = (!parsedTextDirective.EndText.empty() || parsedTextDirective.Suffix.empty())
                                   ? WordBounded::Yes
                                   : WordBounded::No;
    Maybe<SimpleRange> potentialMatch;

    while (!searchRange.Collapsed())
    {
      if (!parsedTextDirective.Prefix.empty())
      {
        auto prefixMatch =
          RangeOfStringInRange(parsedTextDirective.Prefix, searchRange, WordBounded::Yes, WordBounded::No);
        if (!prefixMatch)
        {
          return std::nullopt;
        }

        // .next() does a bit  more than just find the next boundary point, so we differ from the spec here
        // slightly.
        auto newStart = MakeBoundaryPoint(MakeContainerOffsetPosition(prefixMatch->Start).Next());
        if (!newStart)
        {
          return std::nullopt;
        }

        searchRange.Start = *newStart;
        matchRange = SimpleRange {prefixMatch->End, searchRange.End};
        matchRange = AdvanceRangeStartToNextNonWhitespace(*matchRange);
        if (!matchRange)
        {
          return std::nullopt;
        }

        if (matchRange->Collapsed())
        {
          return std::nullopt;
        }

        potentialMatch = RangeOfStringInRange(parsedTextDirective.StartText, *matchRange, WordBounded::No,
                                              mustEndAtWordBoundary);

        if (!potentialMatch)
        {
          return std::nullopt;
        }
        if (potentialMatch->Start != matchRange->Start)
        {
          continue;
        }
      }
      else
      {
        potentialMatch = RangeOfStringInRange(parsedTextDirective.StartText, searchRange, WordBounded::Yes,
                                              mustEndAtWordBoundary);
        if (!potentialMatch)
        {
          return std::nullopt;
        }

        // .next() does a bit  more than just find the next boundary point, so we differ from the spec here
        // slightly.
        auto newStart = MakeBoundaryPoint(MakeContainerOffsetPosition(potentialMatch->Start).Next());
        if (!newStart)
        {
          return std::nullopt;
        }
        searchRange.Start = *newStart;
      }

      SimpleRange rangeEndSearchRange {potentialMatch->End, searchRange.End};
      while (!rangeEndSearchRange.Collapsed())
      {
        if (!parsedTextDirective.EndText.empty())
        {
          mustEndAtWordBoundary = parsedTextDirective.Suffix.empty() ? WordBounded::Yes : WordBounded::No;
          auto endTextMatch = RangeOfStringInRange(parsedTextDirective.EndText, rangeEndSearchRange,
                                                   WordBounded::Yes, mustEndAtWordBoundary);
          if (!endTextMatch)
          {
            return std::nullopt;
          }

          potentialMatch->End = endTextMatch->End;
        }
        // FIXME: Assert: potentialMatch represents a range exactly containing an instance of matching text.
        if (!potentialMatch || potentialMatch->Collapsed())
        {
          return std::nullopt;
        }

        if (parsedTextDirective.Suffix.empty())
        {
          return potentialMatch;
        }

        Maybe<SimpleRange> suffixRange = SimpleRange {potentialMatch->End, searchRange.End};
        suffixRange = AdvanceRangeStartToNextNonWhitespace(*suffixRange);
        if (!suffixRange)
        {
          return std::nullopt;
        }
        auto suffixMatch =
          RangeOfStringInRange(parsedTextDirective.Suffix, *suffixRange, WordBounded::No, WordBounded::Yes);

        if (!suffixMatch)
        {
          return std::nullopt;
        }

        if (suffixMatch->Start == suffixRange->Start)
        {
          return potentialMatch;
        }

        if (parsedTextDirective.EndText.empty())
        {
          break;
        }

        rangeEndSearchRange.Start = potentialMatch->End;
      }

      if (rangeEndSearchRange.Collapsed())
      {
        krys_debug_assert(!parsedTextDirective.EndText.empty());
        return std::nullopt;
      }
    }
    return std::nullopt;
  }

  List<SimpleRange> FragmentDirectiveRangeFinder::FindRangesFromTextDirectives(
    const List<ParsedTextDirective> &parsedTextDirectives, Document &document) noexcept
  {
    List<SimpleRange> ranges;

    for (auto &directive : parsedTextDirectives)
    {
      auto range = FindRangeFromTextDirective(directive, document);
      if (range && !range->Collapsed())
      {
        ranges.push_back(*range);
      }
    }

    return ranges;
  }
}
