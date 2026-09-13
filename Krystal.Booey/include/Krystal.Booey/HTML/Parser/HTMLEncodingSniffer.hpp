#pragma once

#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::html
{
  struct HTMLEncodingSniffer
  {
    HTMLEncodingSniffer() = delete;
    ~HTMLEncodingSniffer() = delete;

    KRYS_NODISCARD static krys::Text::ICodec *Detect(Span<const byte> bytes,
                                                     const krys::Text::CodecRegistry &codecRegistry) noexcept
    {
      for (size_t i = 0uz; i < bytes.size(); i++)
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
          Maybe<bool> needPragma;
          string charset;

          AttributeResult attribute;
          Set<string> attributeList;
          do
          {
            attribute = GetAttribute(bytes, i);
            if (attribute.Name.empty() || attributeList.contains(attribute.Name))
            {
              continue;
            }

            attributeList.insert(attribute.Name);
            if (attribute.Name == "http-equiv")
            {
              if (!gotPragma)
              {
                gotPragma = attribute.Value == "content-type";
              }
            }
            else if (attribute.Name == "content")
            {
              if (charset.empty())
              {
                charset = ExtractCharacterEncodingFromMeta(attribute.Value, codecRegistry);
                if (!charset.empty())
                {
                  needPragma = true;
                }
              }
            }
            else if (attribute.Name == "charset")
            {
              charset = attribute.Value;
              needPragma = false;
            }

          } while (!attribute.Name.empty());

          if (!needPragma.has_value() || charset.empty() || (needPragma.value() && !gotPragma))
          {
            continue;
          }

          using namespace krys::Text;

          ASCIILiteral canonicalName = codecRegistry.LabelToName(charset);
          if (canonicalName == "UTF-16LE"_s || canonicalName == "UTF-16BE"_s)
          {
            canonicalName = "UTF-8"_s;
          }
          else if (canonicalName == "x-user-defined"_s)
          {
            canonicalName = "windows-1252"_s;
          }

          return codecRegistry.Find(canonicalName);
        }

        // We're reading some other element that's not a meta tag, skip it.
        if (bytes[i] == byte {'<'})
        {
          if (i + 1uz >= bytes.size())
          {
            break;
          }

          if (krys::Text::IsASCIIAlpha(bytes[i + 1uz]))
          {
            for (i += 2uz; i < bytes.size(); i++)
            {
              if (krys::Text::IsASCIIWhitespace(bytes[i]) || bytes[i] == byte {'>'})
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

  private:
    struct AttributeResult
    {
      string Name;
      string Value;
    };

    /// @brief Checks if 'bytes' is the start of a comment ('<!--'), using 'i' as an offset.
    KRYS_NODISCARD static bool IsCommentStart(Span<const byte> bytes, size_t i) noexcept
    {
      if ((i + 3uz) >= bytes.size())
      {
        return false;
      }

      return bytes[i + 0uz] == byte {'<'} && bytes[i + 1uz] == byte {'!'} && bytes[i + 2uz] == byte {'-'}
             && bytes[i + 3uz] == byte {'-'};
    }

    /// @brief Checks if 'bytes' is the end of a comment ('-->'), using 'i' as an offset.
    KRYS_NODISCARD static bool IsCommentEnd(Span<const byte> bytes, size_t i) noexcept
    {
      if (i < 2uz || i >= bytes.size())
      {
        return false;
      }

      return bytes[i - 2uz] == byte {'-'} && bytes[i - 1uz] == byte {'-'} && bytes[i - 0uz] == byte {'>'};
    }

    /// @brief Checks if 'bytes' is the end of a meta tag ('<meta( |/)'), using 'i' as an offset.
    KRYS_NODISCARD static bool IsMetaTagStart(Span<const byte> bytes, size_t i) noexcept
    {
      if ((i + 5uz) >= bytes.size())
      {
        return false;
      }

      // case-insensitive "<meta" + space or /
      return bytes[i + 0uz] == byte {'<'} && krys::text::ToASCIILower(bytes[i + 1uz]) == byte {'m'}
             && krys::text::ToASCIILower(bytes[i + 2uz]) == byte {'e'}
             && krys::text::ToASCIILower(bytes[i + 3uz]) == byte {'t'}
             && krys::text::ToASCIILower(bytes[i + 4uz]) == byte {'a'}
             && (krys::text::IsASCIIWhitespace(bytes[i + 5uz]) || bytes[i + 5uz] == byte {'/'});
    }

    KRYS_NODISCARD static AttributeResult GetAttribute(Span<const byte> bytes, size_t &i)
    {
      string name;
      string value;

      for (; i < bytes.size() && !krys::text::IsASCIIAlpha(bytes[i]); i++)
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
        name += static_cast<char8>(krys::text::ToASCIILower(bytes[i]));
        i++;
      } while (i < bytes.size() && krys::text::IsASCIIAlpha(bytes[i]));

      krys::text::SkipASCIIWhitespace(bytes, i);
      if (i >= bytes.size())
      {
        return {name, value};
      }

      // return early if no '=' follows or tag closed
      if (bytes[i] != byte {'='})
      {
        return {name, value};
      }
      i++;

      krys::text::SkipASCIIWhitespace(bytes, i);
      if (i >= bytes.size() || bytes[i] == byte {'/'} || bytes[i] == byte {'>'})
      {
        return {name, value};
      }

      if (bytes[i] == byte {'\''} || bytes[i] == byte {'"'})
      {
        byte closeQuote = bytes[i];

        for (i++; i < bytes.size() && (krys::text::IsASCIIAlphanumeric(bytes[i]) || bytes[i] == byte {'-'});
             i++)
        {
          value += static_cast<char8>(krys::text::ToASCIILower(bytes[i]));
        }

        if (i < bytes.size() && bytes[i] == closeQuote)
        {
          i++;
        }

        return {name, value};
      }

      for (; i < bytes.size(); i++)
      {
        if (krys::text::IsASCIIWhitespace(bytes[i]) || bytes[i] == byte {'>'})
        {
          return {name, value};
        }

        if (krys::text::IsASCIIAlpha(bytes[i]))
        {
          value += static_cast<char8>(krys::text::ToASCIILower(bytes[i]));
        }
      }

      return {name, value};
    }

    /// @see https://html.spec.whatwg.org/#extracting-character-encodings-from-meta-elements
    KRYS_NODISCARD static string
      ExtractCharacterEncodingFromMeta(string &meta, const krys::text::CodecRegistry &codecRegistry)
    {
      for (char &ch : meta)
      {
        ch = krys::text::ToASCIILower(ch);
      }

      const string charset = "charset";
      size_t position = 0uz;
      while (true)
      {
        size_t indexOfCharset = meta.find(charset, position);
        if (indexOfCharset == string::npos)
        {
          return "";
        }

        position += indexOfCharset + charset.size();
        krys::text::SkipASCIIWhitespace(Span<const char>(meta), position);
        if (position >= meta.size())
        {
          return "";
        }

        if (meta[position] != '=')
        {
          position = indexOfCharset + 1;
          continue;
        }

        position++;
        krys::text::SkipASCIIWhitespace(Span<const char>(meta), position);
        break;
      }

      if (position >= meta.size())
      {
        return "";
      }

      if (meta[position] == '"' || meta[position] == '\'')
      {
        const size_t closeQuote = meta.find(meta[position], position + 1);
        if (closeQuote != string::npos)
        {
          return meta.substr(position + 1, closeQuote);
        }
      }

      return "";
    }
  };
}