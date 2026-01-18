#pragma once

#include "Krystal.HTML/Parser/HTMLInputStream.hpp"
#include "Krystal.HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/Parser/TokenizerState.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class NextTokenPtr
  {
  private:
    RawPtr<HTMLToken> _token {nullptr};

  public:
    NextTokenPtr() noexcept = default;
    NextTokenPtr &operator=(NextTokenPtr &&) = delete;

    ~NextTokenPtr() noexcept
    {
      if (_token != nullptr)
      {
        _token->Clear();
      }
    }

    NextTokenPtr(NextTokenPtr &&other) noexcept : _token(other._token)
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

    explicit NextTokenPtr(RawPtr<HTMLToken> token) noexcept : _token(token)
    {
    }
  };

  class HTMLTokenizer
  {

  private:
    TokenizerState _state {TokenizerState::Data};
    HTMLToken _token;

  public:
    HTMLTokenizer() noexcept
    {
    }

    NextTokenPtr NextToken() noexcept
    {
      return NextTokenPtr(ProcessToken() ? &_token : nullptr);
    }

  private:
    bool ProcessToken() noexcept
    {
    }
  };
}