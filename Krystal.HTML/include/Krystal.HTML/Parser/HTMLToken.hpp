#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  struct DoctypeData
  {
    List<char16> PublicIdentifier;
    List<char16> SystemIdentifier;
    bool HasPublicIdentifier : 1 {false};
    bool HasSystemIdentifier : 1 {false};
    bool ForceQuirks : 1 {false};
  };

  class HTMLToken
  {
  public:
    enum class Type : uint8
    {
      Uninitialized,
      DOCTYPE,
      StartTag,
      EndTag,
      Comment,
      Character,
      EndOfFile,
    };

    struct Attribute
    {
      List<char32> Name;
      List<char32> Value;
    };

    using AttributeList = List<Attribute>;
    using DataBuffer = List<char32>;

  private:
    Type _type : 7 {Type::Uninitialized};

    // For start/end tag tokens.
    bool _selfClosing : 1 {false};

    // For DOCTYPE only
    UniquePtr<DoctypeData> _doctypeData;

    // For start/end tag tokens only
    AttributeList _attributes;
    RawPtr<Attribute> _currentAttribute {nullptr};

    // The name for DOCTYPE/start/end tokens, text data for character tokens, comment data for comment tokens.
    DataBuffer _data;

  public:
    HTMLToken() = default;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    void Clear() noexcept
    {
      _type = Type::Uninitialized;
      _data.clear();
    }

    void SetAsEOF() noexcept
    {
      assert(_type == Type::Uninitialized);
      _type = Type::EndOfFile;
    }

    void AddCharacter(char32 character) noexcept
    {
      assert(_type == Type::Uninitialized || _type == Type::Character);
      _type = Type::Character;
      _data.push_back(character);
    }

    void AddCharacters(Span<char32> characters) noexcept
    {
      assert(_type == Type::Uninitialized || _type == Type::Character);
      _type = Type::Character;
      _data.insert(_data.end(), characters.begin(), characters.end());
    }

    void BeginComment() noexcept
    {
      assert(_type == Type::Uninitialized);
      _type = Type::Comment;
    }

    void BeginStartTag(char8 character) noexcept
    {
      assert(character);
      assert(_type == Type::Uninitialized);

      _type = Type::StartTag;
      _selfClosing = false;
      _attributes.clear();

#if KRYS_ENV(DEV)
      _currentAttribute = nullptr;
#endif

      _data.push_back(character);
    }

    void BeginEndTag(char8 character) noexcept
    {
      assert(character);
      assert(_type == Type::Uninitialized);

      _type = Type::EndTag;
      _selfClosing = false;
      _attributes.clear();

#if KRYS_ENV(DEV)
      _currentAttribute = nullptr;
#endif

      _data.push_back(character);
    }

    void AppendToName(char32 character) noexcept
    {
      assert((_type == Type::StartTag || _type == Type::EndTag) && character);
      _data.push_back(character);
    }

    utf32_string GetName() const noexcept
    {
      assert(_type == Type::StartTag || _type == Type::EndTag || _type == Type::DOCTYPE);
      return utf32_string(_data.begin(), _data.end());
    }
  };

  class HTMLTokenizer
  {
    class TokenPtr
    {
    private:
      RawPtr<HTMLToken> _token {nullptr};

    public:
      TokenPtr() noexcept = default;
      TokenPtr &operator=(TokenPtr &&) = delete;

      ~TokenPtr() noexcept
      {
        if (_token != nullptr)
        {
          _token->Clear();
        }
      }

      TokenPtr(TokenPtr &&other) noexcept : _token(other._token)
      {
        other._token = nullptr;
      }

      void Clear() noexcept
      {
        if (_token != nullptr)
        {
          _token->Clear();
          _token = nullptr;
        }
      }

      operator bool() const noexcept
      {
        return _token != nullptr;
      }

      HTMLToken &operator*() const noexcept
      {
        assert(_token != nullptr);
        return *_token;
      }

      RawPtr<HTMLToken> operator->() const noexcept
      {
        assert(_token != nullptr);
        return _token;
      }

    private:
      friend class HTMLTokenizer;

      explicit TokenPtr(RawPtr<HTMLToken> token) noexcept : _token(token)
      {
      }
    };

  private:
    TokenizerState _state {TokenizerState::Data};
    HTMLToken _token;

  public:
    HTMLTokenizer() noexcept
    {
    }

    TokenPtr NextToken() noexcept
    {
      return TokenPtr(ProcessToken() ? &_token : nullptr);
    }

  private:
    bool ProcessToken() noexcept
    {
    }
  };
}