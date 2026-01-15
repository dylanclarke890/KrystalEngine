#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/Types/Nullable.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Set.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ByteOrderMark.hpp"
#include "Krystal.Text/EncodingId.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys::HTML::DOM
{
  struct EncodingSniffer : NonCopyMovable<EncodingSniffer>
  {
    EncodingSniffer() = delete;
    ~EncodingSniffer() = delete;

    KRYS_NODISCARD static const Text::EncodingId Detect(Span<const byte> bytes,
                                                       const Text::EncodingRegistry &encodingRegistry,
                                                 const utf8_string &transportEncoding,
                                                 const utf8_string &fallbackEncoding) noexcept
    {
      if (auto encoding = GetBOMEncoding(bytes, encodingRegistry); encoding != nullptr)
      {
        return encoding;
      }

      if (!transportEncoding.empty())
      {
        if (auto encoding = encodingRegistry.GetByLabel(transportEncoding); encoding != nullptr)
        {
          return encoding;
        }
      }

      if (auto encoding = PrescanMetaCharset(bytes, encodingRegistry); encoding != nullptr)
      {
        return encoding;
      }

      return encodingRegistry.GetByLabel(fallbackEncoding);
    }

  private:
    struct AttributeResult
    {
      utf8_string Name;
      utf8_string Value;
    };

    KRYS_NODISCARD static const Text::Encoding *
      GetBOMEncoding(Span<const byte> bytes, const Text::EncodingRegistry &encodingRegistry) noexcept
    {
      auto bom = Text::ByteOrderMarkScanner::Detect(bytes);
      switch (bom)
      {
        case Text::ByteOrderMark::UTF8: return encodingRegistry.GetByLabel(u8"utf-8");
        case Text::ByteOrderMark::UTF16BE: return encodingRegistry.GetByLabel(u8"utf-16be");
        case Text::ByteOrderMark::UTF16LE: return encodingRegistry.GetByLabel(u8"utf-16le");
      }

      return nullptr;
    }

    /// @brief Checks if 'bytes' is the start of a comment ('<!--'), using 'i' as an offset.
    KRYS_NODISCARD static bool IsCommentStart(Span<const byte> bytes, size_t i) noexcept
    {
      return bytes[i + 0] == byte {'<'} && bytes[i + 1] == byte {'!'} && bytes[i + 2] == byte {'-'}
             && bytes[i + 3] == byte {'-'};
    }

    /// @brief Checks if 'bytes' is the end of a comment ('-->'), using 'i' as an offset.
    KRYS_NODISCARD static bool IsCommentEnd(Span<const byte> bytes, size_t i) noexcept
    {
      return bytes[i - 2] == byte {'-'} && bytes[i - 1] == byte {'-'} && bytes[i - 0] == byte {'>'};
    }

    /// @brief Checks if 'bytes' is the end of a meta tag ('<meta( |/)'), using 'i' as an offset.
    KRYS_NODISCARD static bool IsMetaTagStart(Span<const byte> bytes, size_t i) noexcept
    {
      // case-insensitive "meta" + space or /
      return bytes[i + 0] == byte {'<'} && Text::ASCII::ToLower(bytes[i + 1]) == byte {'m'}
             && Text::ASCII::ToLower(bytes[i + 2]) == byte {'e'}
             && Text::ASCII::ToLower(bytes[i + 3]) == byte {'t'}
             && Text::ASCII::ToLower(bytes[i + 4]) == byte {'a'}
             && (Text::ASCII::IsWhiteSpace(bytes[i + 5]) || bytes[i + 5] == byte {'/'});
    }

    KRYS_NODISCARD static const Text::Encoding *
      PrescanMetaCharset(Span<const byte> bytes, const Text::EncodingRegistry &encodingRegistry) noexcept
    {
      for (size_t i = 0; i < bytes.size(); i++)
      {
        // Skip comments
        if (IsCommentStart(bytes, i))
        {
          i += 4;

          while (i < bytes.size() && !IsCommentEnd(bytes, i))
          {
            i++;
          }
          i += 1; // Move past '>'
          continue;
        }

        // Try and parse encoding from meta tag.
        if (IsMetaTagStart(bytes, i))
        {
          i += 6;

          bool gotPragma = false;
          Nullable<bool> needPragma;
          utf8_string charset;

          AttributeResult attribute;
          Set<utf8_string> attributeList;
          do
          {
            attribute = GetAttribute(bytes, i);
            if (attribute.Name.empty() || attributeList.contains(attribute.Name))
            {
              continue;
            }

            attributeList.insert(attribute.Name);
            if (attribute.Name == u8"http-equiv")
            {
              if (!gotPragma)
              {
                gotPragma = attribute.Value == u8"content-type";
              }
            }
            else if (attribute.Name == u8"content")
            {
              if (!charset.empty())
              {
                charset = ExtractCharacterEncodingFromMeta(attribute.Value, encodingRegistry);
                if (!charset.empty())
                {
                  needPragma = true;
                }
              }
            }
            else if (attribute.Name == u8"charset")
            {
              charset = attribute.Value;
              needPragma = false;
            }

          } while (!attribute.Name.empty());

          if (!needPragma.has_value() || charset.empty() || (needPragma.value() && !gotPragma))
          {
            continue;
          }

          utf8_string canonicalName = encodingRegistry.LabelToName(charset);
          if (canonicalName == u8"UTF-16LE" || canonicalName == u8"UTF-16BE")
          {
            canonicalName = u8"UTF-8";
          }
          else if (canonicalName == u8"x-user-defined")
          {
            canonicalName = u8"windows-1252";
          }

          return encodingRegistry.GetByLabel(canonicalName);
        }

        // We're reading some other element that's not a meta tag, skip it.
        if (bytes[i] == byte {'<'})
        {
          if (Text::ASCII::IsAlpha(bytes[i + 1]))
          {
            for (i += 2; i < bytes.size(); i++)
            {
              if (Text::ASCII::IsWhiteSpace(bytes[i]) || bytes[i] == byte {'>'})
              {
                break;
              }
            }

            AttributeResult attribute;
            do
            {
              attribute = GetAttribute(bytes, i);
            } while (!attribute.Name.empty());
          }
          else if (bytes[i + 1] == byte {'!'} || bytes[i + 1] == byte {'/'} || bytes[i + 1] == byte {'?'})
          {
            for (i += 2; i < bytes.size(); i++)
            {
              if (bytes[i] == byte {'>'})
              {
                break;
              }
            }
          }
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD static AttributeResult GetAttribute(Span<const byte> bytes, size_t &i)
    {
      utf8_string name;
      utf8_string value;

      for (; i < bytes.size() && !Text::ASCII::IsAlpha(bytes[i]); i++)
      {
        // Tag closed before we saw name
        if (bytes[i] == byte {'>'})
        {
          i++;
          return {name, value};
        }
      }

      if (i >= bytes.size())
      {
        return {name, value};
      }

      // build up the name
      do
      {
        name += static_cast<char8>(Text::ASCII::ToLower(bytes[i]));
        i++;
      } while (i < bytes.size() && Text::ASCII::IsAlpha(bytes[i]));

      Text::ASCII::SkipWhiteSpace(bytes, i);
      if (i >= bytes.size())
      {
        return {name, value};
      }

      // return early if no '=' follows or tag closed
      if (bytes[i] != byte {'='} || bytes[i] == byte {'/'} || bytes[i] == byte {'>'})
      {
        return {name, value};
      }
      i++;

      Text::ASCII::SkipWhiteSpace(bytes, i);
      if (i >= bytes.size() || bytes[i] == byte {'>'})
      {
        return {name, value};
      }

      if (Text::ASCII::IsQuote(bytes[i]))
      {
        byte closeQuote = bytes[i];

        for (i++; i < bytes.size() && (Text::ASCII::IsAlphaNumeric(bytes[i]) || bytes[i] == byte {'-'}); i++)
        {
          value += static_cast<char8>(Text::ASCII::ToLower(bytes[i]));
        }

        if (i < bytes.size() && bytes[i] == closeQuote)
        {
          i++;
        }

        return {name, value};
      }

      for (; i < bytes.size(); i++)
      {
        if (Text::ASCII::IsWhiteSpace(bytes[i]) || bytes[i] == byte {'>'})
        {
          return {name, value};
        }

        if (Text::ASCII::IsAlpha(bytes[i]))
        {
          value += static_cast<char8>(Text::ASCII::ToLower(bytes[i]));
        }
      }

      return {name, value};
    }

    KRYS_NODISCARD static utf8_string
      ExtractCharacterEncodingFromMeta(const utf8_string &meta,
                                       const Text::EncodingRegistry &encodingRegistry)
    {
      utf8_string data = Text::ASCII::ToLower(meta);
      const utf8_string charset = u8"charset";

      size_t position = 0uz;
      while (true)
      {
        size_t indexOfCharset = data.find(charset, position);
        if (indexOfCharset == utf8_string::npos)
        {
          return u8"";
        }

        position += indexOfCharset + charset.size();
        Text::ASCII::SkipWhiteSpace(data, position);

        if (data[position] != '=')
        {
          position--;
          continue;
        }

        position++;
        Text::ASCII::SkipWhiteSpace(data, position);
        break;
      }

      if (data[position] == '"' || data[position] == '\'')
      {
        const size_t closeQuote = data.find(data[position], position + 1);
        if (closeQuote != utf8_string::npos)
        {
          return data.substr(position + 1, closeQuote);
        }
      }

      return u8"";
    }
  };
}