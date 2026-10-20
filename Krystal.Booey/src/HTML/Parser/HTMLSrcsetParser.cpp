#include "Krystal.Booey/HTML/Parser/HTMLSrcsetParser.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo::html
{
  namespace
  {
    enum class DescriptorTokenizerState : uint8
    {
      Initial,
      InParenthesis,
      AfterToken,
    };

    KRYS_NODISCARD bool IsHTMLSpaceOrComma(char8 c) noexcept
    {
      return text::IsASCIIWhitespace(c) || c == u8',';
    }

    void TokenizeDescriptors(dom::DOMStringView attribute, size_t &position,
                             List<dom::DOMStringView> &descriptors) noexcept
    {
      auto state = DescriptorTokenizerState::Initial;
      Maybe<size_t> currentDescriptorStart = position;

      auto appendDescriptorAndReset = [&](size_t end)
      {
        if (currentDescriptorStart && end > *currentDescriptorStart)
        {
          descriptors.push_back(attribute.substr(*currentDescriptorStart, end - *currentDescriptorStart));
        }
        currentDescriptorStart = null;
      };
      // Called appendCharacter to match the spec's terminology. Since we don't copy the tokens, this just
      // sets the point where the descriptor tokens start.
      auto appendCharacter = [&](size_t at)
      {
        if (!currentDescriptorStart)
        {
          currentDescriptorStart = at;
        }
      };

      for (;; ++position)
      {
        bool atEnd = position >= attribute.size();
        switch (state)
        {
          case DescriptorTokenizerState::Initial:
            if (atEnd)
            {
              appendDescriptorAndReset(attribute.size());
              return;
            }
            if (attribute[position] == u8',')
            {
              appendDescriptorAndReset(position);
              ++position;
              return;
            }
            if (text::IsASCIIWhitespace(attribute[position]))
            {
              appendDescriptorAndReset(position);
              currentDescriptorStart = position + 1;
              state = DescriptorTokenizerState::AfterToken;
            }
            else if (attribute[position] == u8'(')
            {
              appendCharacter(position);
              state = DescriptorTokenizerState::InParenthesis;
            }
            else
            {
              appendCharacter(position);
            }
            break;
          case DescriptorTokenizerState::InParenthesis:
            if (atEnd)
            {
              appendDescriptorAndReset(attribute.size());
              return;
            }
            appendCharacter(position);
            if (attribute[position] == u8')')
            {
              state = DescriptorTokenizerState::Initial;
            }
            break;
          case DescriptorTokenizerState::AfterToken:
            if (atEnd)
            {
              return;
            }
            if (!text::IsASCIIWhitespace(attribute[position]))
            {
              // Reconsume the character in the initial state.
              state = DescriptorTokenizerState::Initial;
              currentDescriptorStart = position;
              --position;
            }
            break;
        }
      }
    }

    KRYS_NODISCARD bool ParseDescriptors(const List<dom::DOMStringView> &descriptors,
                                         DescriptorParsingResult &result) noexcept
    {
      constexpr uint64 MaxInteger = static_cast<uint64>(std::numeric_limits<int32>::max());
      for (auto descriptor : descriptors)
      {
        if (descriptor.empty())
        {
          continue;
        }
        auto descriptorChar = descriptor.back();
        descriptor.remove_suffix(1);
        if (descriptorChar == u8'x')
        {
          if (result.HasDensity() || result.HasHeight() || result.HasWidth())
          {
            return false;
          }
          auto density = Numbers::ParseValidFloatingPoint(descriptor);
          if (!density.Success() || density.Value < 0)
          {
            return false;
          }
          result.SetDensity(static_cast<float>(density.Value));
        }
        else if (descriptorChar == u8'w')
        {
          if (result.HasDensity() || result.HasWidth())
          {
            return false;
          }
          auto resourceWidth = Numbers::ParseValidNonNegativeInteger(descriptor);
          if (!resourceWidth.Success() || !resourceWidth.Value || resourceWidth.Value > MaxInteger)
          {
            return false;
          }
          result.SetResourceWidth(static_cast<int32>(resourceWidth.Value));
        }
        else if (descriptorChar == u8'h')
        {
          // This is here only for future compat purposes.
          // The value of the 'h' descriptor is not used.
          if (result.HasDensity() || result.HasHeight())
          {
            return false;
          }
          auto resourceHeight = Numbers::ParseValidNonNegativeInteger(descriptor);
          if (!resourceHeight.Success() || !resourceHeight.Value || resourceHeight.Value > MaxInteger)
          {
            return false;
          }
          result.SetResourceHeight(static_cast<int32>(resourceHeight.Value));
        }
        else
        {
          return false;
        }
      }
      return !result.HasHeight() || result.HasWidth();
    }

    KRYS_NODISCARD ImageCandidate PickBestImageCandidate(float deviceScaleFactor,
                                                         List<ImageCandidate> &imageCandidates,
                                                         float sourceSize) noexcept
    {
      bool ignoreSrc = false;
      if (imageCandidates.empty())
      {
        return {};
      }

      // http://picture.responsiveimages.org/#normalize-source-densities
      for (auto &candidate : imageCandidates)
      {
        if (candidate.ResourceWidth > 0)
        {
          candidate.Density = static_cast<float>(candidate.ResourceWidth) / sourceSize;
          ignoreSrc = true;
        }
        else if (candidate.Density < 0)
        {
          candidate.Density = DefaultDensityValue;
        }
      }

      std::ranges::stable_sort(imageCandidates, [](const ImageCandidate &first, const ImageCandidate &second)
                               { return first.Density < second.Density; });

      size_t i;
      for (i = 0; i < imageCandidates.size() - 1; ++i)
      {
        if ((imageCandidates[i].Density >= deviceScaleFactor)
            && (!ignoreSrc || !imageCandidates[i].SrcOrigin()))
        {
          break;
        }
      }

      if (imageCandidates[i].SrcOrigin() && ignoreSrc)
      {
        krys_debug_assert(i > 0);
        --i;
      }
      float winningDensity = imageCandidates[i].Density;

      size_t winner = i;
      // 16. If an entry b in candidates has the same associated ... pixel density as an earlier entry a in
      // candidates, then remove entry b
      while ((i > 0) && (imageCandidates[--i].Density == winningDensity))
      {
        winner = i;
      }

      return imageCandidates[winner];
    }
  }

  // http://picture.responsiveimages.org/#parse-srcset-attr
  List<ImageCandidate> ParseImageCandidatesFromSrcsetAttribute(dom::DOMStringView attribute) noexcept
  {
    List<ImageCandidate> imageCandidates;

    size_t position = 0;
    while (position < attribute.size())
    {
      // 4. Splitting loop: Collect a sequence of characters that are space characters or U+002C COMMA
      // characters.
      while (position < attribute.size() && IsHTMLSpaceOrComma(attribute[position]))
      {
        ++position;
      }
      if (position >= attribute.size())
      {
        // Contrary to spec language - descriptor parsing happens on each candidate, so when we reach the
        // attributeEnd, we can exit.
        break;
      }

      // 6. Collect a sequence of characters that are not space characters, and let that be url.
      auto imageURLStart = position;
      while (position < attribute.size() && !text::IsASCIIWhitespace(attribute[position]))
      {
        ++position;
      }
      auto imageURL = attribute.substr(imageURLStart, position - imageURLStart);

      DescriptorParsingResult result;

      // 8. If url ends with a U+002C COMMA character (,)
      if (imageURL.back() == u8',')
      {
        // Remove all trailing U+002C COMMA characters from url.
        while (!imageURL.empty() && imageURL.back() == u8',')
        {
          imageURL.remove_suffix(1);
        }

        // If url is empty, then jump to the step labeled splitting loop.
        if (imageURL.empty())
        {
          continue;
        }
      }
      else
      {
        while (position < attribute.size() && text::IsASCIIWhitespace(attribute[position]))
        {
          ++position;
        }
        List<dom::DOMStringView> descriptorTokens;
        TokenizeDescriptors(attribute, position, descriptorTokens);
        // Contrary to spec language - descriptor parsing happens on each candidate.
        // This is a black-box equivalent, to avoid storing descriptor lists for each candidate.
        if (!ParseDescriptors(descriptorTokens, result))
        {
          continue;
        }
      }

      krys_debug_assert(!imageURL.empty());
      imageCandidates.emplace_back(dom::DOMString(imageURL), result, ImageCandidate::OriginAttribute::Srcset);
      // 11. Return to the step labeled splitting loop.
    }
    return imageCandidates;
  }

  ImageCandidate BestFitSourceForImageAttributes(
    float deviceScaleFactor, Maybe<dom::DOMStringView> srcAttribute,
    Maybe<dom::DOMStringView> srcsetAttribute, float sourceSize,
    const Func<bool(const ImageCandidate &)> &shouldIgnoreCandidateCallback) noexcept
  {
    if (!srcsetAttribute)
    {
      if (!srcAttribute)
      {
        return {};
      }
      return ImageCandidate(dom::DOMString(*srcAttribute), DescriptorParsingResult(),
                            ImageCandidate::OriginAttribute::Src);
    }

    auto imageCandidates = ParseImageCandidatesFromSrcsetAttribute(*srcsetAttribute);

    if (srcAttribute && !srcAttribute->empty())
    {
      imageCandidates.emplace_back(dom::DOMString(*srcAttribute), DescriptorParsingResult(),
                                   ImageCandidate::OriginAttribute::Src);
    }

    if (shouldIgnoreCandidateCallback)
    {
      std::erase_if(imageCandidates, shouldIgnoreCandidateCallback);
    }

    return PickBestImageCandidate(deviceScaleFactor, imageCandidates, sourceSize);
  }
}
