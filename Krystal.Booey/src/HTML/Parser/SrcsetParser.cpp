/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/Parser/SrcsetParser.hpp"
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

    // Steps 9 to 14 of parsing a srcset attribute, the descriptor parser: whether error is still no.
    KRYS_NODISCARD bool ParseDescriptors(const List<dom::DOMStringView> &descriptors,
                                         DescriptorParsingResult &result) noexcept
    {
      // 9. Descriptor parser: Let error be no.
      // 10. Let width be absent.
      // 11. Let density be absent.
      // 12. Let future-compat-h be absent.
      // 13. For each descriptor in descriptors, run the appropriate set of steps from the following list:
      // (Widths and heights over the largest int32 are errors.)
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
      // 14. If future-compat-h is not absent and width is absent, let error be yes.
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

      // Normalize the source densities: https://html.spec.whatwg.org/#normalise-the-source-densities
      // 1. Let source size be source set's source size.
      // 2. For each image source in source set:
      for (auto &candidate : imageCandidates)
      {
        // 2a. If the image source has a pixel density descriptor, continue to the next image source.
        // 2b. Otherwise, if the image source has a width descriptor, replace the width descriptor with a
        // pixel
        //     density descriptor with a value of the width descriptor value divided by source size and a unit
        //     of x.
        // (The src attribute's source is then ignored: a source set has no default source with widths.)
        if (candidate.ResourceWidth > 0)
        {
          candidate.Density = static_cast<float>(candidate.ResourceWidth) / sourceSize;
          ignoreSrc = true;
        }
        // 2c. Otherwise, give the image source a pixel density descriptor of 1x.
        else if (candidate.Density < 0)
        {
          candidate.Density = DefaultDensityValue;
        }
      }

      // Select an image source from a source set:
      // https://html.spec.whatwg.org/#select-an-image-source-from-a-source-set
      // 2. In an implementation-defined manner, choose one image source from sourceSet. Let selectedSource be
      //    this choice.
      // (The lowest density at least the device scale factor, else the highest.)

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
      // 1. If an entry b in sourceSet has the same associated pixel density descriptor as an earlier entry a
      // in
      //    sourceSet, then remove entry b. Repeat this step until none of the entries in sourceSet have the
      //    same associated pixel density descriptor as an earlier entry.
      // (Of the entries with the winning density, the earliest wins.)
      while ((i > 0) && (imageCandidates[--i].Density == winningDensity))
      {
        winner = i;
      }

      // 3. Return selectedSource and its associated pixel density.
      return imageCandidates[winner];
    }
  }

  List<ImageCandidate> ParseImageCandidatesFromSrcsetAttribute(dom::DOMStringView attribute) noexcept
  {
    // 1. Let input be the value passed to this algorithm.
    // 2. Let position be a pointer into input, initially pointing at the start of the string.
    // 3. Let candidates be an initially empty source set.
    List<ImageCandidate> imageCandidates;
    size_t position = 0;
    while (true)
    {
      // 4. Splitting loop: Collect a sequence of code points that are ASCII whitespace or U+002C COMMA
      //    characters from input given position. If any U+002C COMMA characters were collected, that is a
      //    parse error.
      while (position < attribute.size() && IsHTMLSpaceOrComma(attribute[position]))
      {
        ++position;
      }

      // 5. If position is past the end of input, return candidates.
      if (position >= attribute.size())
      {
        return imageCandidates;
      }

      // 6. Collect a sequence of code points that are not ASCII whitespace from input given position, and let
      //    url be the result.
      auto imageURLStart = position;
      while (position < attribute.size() && !text::IsASCIIWhitespace(attribute[position]))
      {
        ++position;
      }
      auto imageURL = attribute.substr(imageURLStart, position - imageURLStart);

      // 7. Let descriptors be a new empty list.
      DescriptorParsingResult result;

      // 8. If url ends with U+002C (,), then:
      if (imageURL.back() == u8',')
      {
        // 8a. Remove all trailing U+002C COMMA characters from url. If this removed more than one character,
        //     that is a parse error.
        // (The splitting loop skipped any leading commas, so url isn't only commas.)
        while (!imageURL.empty() && imageURL.back() == u8',')
        {
          imageURL.remove_suffix(1);
        }
      }
      // Otherwise:
      else
      {
        // 8a. Descriptor tokenizer: Skip ASCII whitespace within input given position.
        while (position < attribute.size() && text::IsASCIIWhitespace(attribute[position]))
        {
          ++position;
        }

        // 8b. Let current descriptor be the empty string.
        // 8c. Let state be in descriptor.
        // 8d. Let c be the character at position. Do the following depending on the value of state.
        List<dom::DOMStringView> descriptorTokens;
        TokenizeDescriptors(attribute, position, descriptorTokens);

        // 9.-14. (The descriptor parser.)
        // 15. If error is still no, then append a new image source to candidates whose URL is url, associated
        //     with a width width if not absent and a pixel density density if not absent. Otherwise, there is
        //     a parse error.
        if (!ParseDescriptors(descriptorTokens, result))
        {
          continue;
        }
      }

      imageCandidates.emplace_back(dom::DOMString(imageURL), result, ImageCandidate::OriginAttribute::Srcset);

      // 16. Return to the step labeled splitting loop.
    }
  }

  ImageCandidate BestFitSourceForImageAttributes(
    float deviceScaleFactor, Maybe<dom::DOMStringView> srcAttribute,
    Maybe<dom::DOMStringView> srcsetAttribute, float sourceSize,
    const Func<bool(const ImageCandidate &)> &shouldIgnoreCandidateCallback) noexcept
  {
    // Create a source set: https://html.spec.whatwg.org/#create-a-source-set
    if (!srcsetAttribute)
    {
      if (!srcAttribute)
      {
        return {};
      }
      return ImageCandidate(dom::DOMString(*srcAttribute), DescriptorParsingResult(),
                            ImageCandidate::OriginAttribute::Src);
    }

    // 1. Let source set be an empty source set.
    // 2. If srcset is not an empty string, then set source set to the result of parsing srcset.
    auto imageCandidates = ParseImageCandidatesFromSrcsetAttribute(*srcsetAttribute);

    // 3. Set source set's source size to the result of parsing sizes with img.
    // (The caller parsed it, as sourceSize.)

    // 4. If default source is not the empty string and source set does not contain an image source with a
    //    pixel density descriptor value of 1, and no image source with a width descriptor, append default
    //    source to source set.
    // (It's appended anyway, and then loses to an earlier 1x entry, or is ignored if there are widths.)
    if (srcAttribute && !srcAttribute->empty())
    {
      imageCandidates.emplace_back(dom::DOMString(*srcAttribute), DescriptorParsingResult(),
                                   ImageCandidate::OriginAttribute::Src);
    }

    if (shouldIgnoreCandidateCallback)
    {
      std::erase_if(imageCandidates, shouldIgnoreCandidateCallback);
    }

    // 5. Normalize the source densities of source set.
    // 6. Return source set.
    // (And select an image from it.)
    return PickBestImageCandidate(deviceScaleFactor, imageCandidates, sourceSize);
  }
}
