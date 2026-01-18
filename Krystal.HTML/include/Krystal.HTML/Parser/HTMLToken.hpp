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
    bool HasPublicIdentifier {false};
    bool HasSystemIdentifier {false};
    bool ForceQuirks {false};
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
    Type _type {Type::Uninitialized};
    bool _selfClosing;
    DataBuffer _data;
    AttributeList _attributes;
    RawPtr<Attribute> _currentAttribute;

    // For DOCTYPE only
    UniquePtr<DoctypeData> _doctypeData;

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