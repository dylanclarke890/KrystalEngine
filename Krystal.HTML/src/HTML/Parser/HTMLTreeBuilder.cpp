#include "Krystal.HTML/HTML/Parser/HTMLTreeBuilder.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/HTML/Enums/HTMLTokenType.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  void HTMLTreeBuilder::ProcessToken(HTMLTokenAtom &&token) noexcept
  {
    switch (_insertionMode)
    {
      case InsertionMode::Initial:
        ProcessTokenInInitialMode(token);
        break;
      case InsertionMode::BeforeHTML:
        ProcessTokenInBeforeHTMLMode(token);
        break;
      case InsertionMode::BeforeHead:
        ProcessTokenInBeforeHeadMode(token);
        break;
      case InsertionMode::InHead:
        ProcessTokenInHeadMode(token);
        break;
      case InsertionMode::InHeadNoscript:
        ProcessTokenInInHeadNoscriptMode(token);
        break;
      case InsertionMode::AfterHead:
        ProcessTokenInAfterHeadMode(token);
        break;
      case InsertionMode::InBody:
        ProcessTokenInBodyMode(token);
        break;
      case InsertionMode::Text:
        ProcessTokenInTextMode(token);
        break;
      case InsertionMode::InTable:
        ProcessTokenInTableMode(token);
        break;
      default:
        // Handle other modes as they are implemented
        break;
    }
  }

  void HTMLTreeBuilder::ProcessTokenInInitialMode(const HTMLTokenAtom &token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        // Characters that are followed by whitespace or EOF should be ignored
        auto data = token.Data();
        bool isWhitespaceOnly = true;
        for (auto ch : data)
        {
          if (!Krys::Text::IsASCIIWhitespace(ch))
          {
            isWhitespaceOnly = false;
            break;
          }
        }
        if (isWhitespaceOnly)
        {
          // Ignore whitespace
          return;
        }
        // Non-whitespace character: act as if we received EOF, switch to BeforeHTML mode
        ChangeInsertionMode(InsertionMode::BeforeHTML);
        ProcessTokenInBeforeHTMLMode(token);
        break;
      }
      case HTMLTokenType::Comment:
        // Insert comment into document
        // TODO: Implement comment node creation
        break;
      case HTMLTokenType::DOCTYPE:
      {
        // Create doctype node and append to document
        // TODO: Implement DOCTYPE node creation
        ChangeInsertionMode(InsertionMode::BeforeHTML);
        break;
      }
      case HTMLTokenType::EndOfFile:
        // Stop parsing
        break;
      case HTMLTokenType::StartTag:
      case HTMLTokenType::EndTag:
        // Parse error: unexpected token in initial mode
        ChangeInsertionMode(InsertionMode::BeforeHTML);
        ProcessTokenInBeforeHTMLMode(token);
        break;
      default:
        break;
    }
  }

  void HTMLTreeBuilder::ProcessTokenInBeforeHTMLMode(const HTMLTokenAtom &token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::DOCTYPE:
        // Parse error: DOCTYPE in BeforeHTML mode
        break;
      case HTMLTokenType::Comment:
        // Insert comment into document
        // TODO: Implement comment node creation
        break;
      case HTMLTokenType::Character:
      {
        // Ignore whitespace characters
        auto data = token.Data();
        bool isWhitespaceOnly = true;
        for (auto ch : data)
        {
          if (!Krys::Text::IsASCIIWhitespace(ch))
          {
            isWhitespaceOnly = false;
            break;
          }
        }
        if (isWhitespaceOnly)
        {
          return;
        }
        // Non-whitespace: process as StartTag for <html>
        ChangeInsertionMode(InsertionMode::BeforeHead);
        break;
      }
      case HTMLTokenType::StartTag:
      {
        // Only <html> is special
        auto name = token.Name();
        if (name == DOMStringAtom {u8"html"})
        {
          // TODO: Create html element and push to stack
          // TODO: Change to BeforeHead mode
          ChangeInsertionMode(InsertionMode::BeforeHead);
        }
        else
        {
          // Other start tags: error, but continue with implicit html
          ChangeInsertionMode(InsertionMode::BeforeHead);
          ProcessTokenInBeforeHeadMode(token);
        }
        break;
      }
      case HTMLTokenType::EndTag:
        // Parse error: end tag in BeforeHTML mode
        break;
      case HTMLTokenType::EndOfFile:
        // Stop parsing
        break;
      default:
        break;
    }
  }

  void HTMLTreeBuilder::ProcessTokenInBeforeHeadMode(const HTMLTokenAtom &token) noexcept
  {
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        // Ignore whitespace only
        auto data = token.Data();
        bool isWhitespaceOnly = true;
        for (auto ch : data)
        {
          if (!Krys::Text::IsASCIIWhitespace(ch))
          {
            isWhitespaceOnly = false;
            break;
          }
        }
        if (isWhitespaceOnly)
        {
          return;
        }
        // Non-whitespace: process as start tag
        ChangeInsertionMode(InsertionMode::InHead);
        ProcessTokenInHeadMode(token);
        break;
      }
      case HTMLTokenType::Comment:
        // Insert comment
        // TODO: Implement comment node creation
        break;
      case HTMLTokenType::DOCTYPE:
        // Parse error
        break;
      case HTMLTokenType::StartTag:
      {
        auto name = token.Name();
        if (name == DOMStringAtom {u8"head"})
        {
          // TODO: Create head element, push to stack, save as _head, transition to InHead
          ChangeInsertionMode(InsertionMode::InHead);
        }
        else
        {
          // Other elements: error, implicitly create head and reprocess
          ChangeInsertionMode(InsertionMode::InHead);
          ProcessTokenInHeadMode(token);
        }
        break;
      }
      case HTMLTokenType::EndTag:
        // Most end tags are errors in this mode
        ChangeInsertionMode(InsertionMode::InHead);
        break;
      case HTMLTokenType::EndOfFile:
        // Stop parsing
        break;
      default:
        break;
    }
  }

  void HTMLTreeBuilder::ProcessTokenInHeadMode(const HTMLTokenAtom &token) noexcept
  {
    // TODO: Implement full InHead mode
    switch (token.Type())
    {
      case HTMLTokenType::Character:
      {
        // Whitespace only: append to current node
        auto data = token.Data();
        bool isWhitespaceOnly = true;
        for (auto ch : data)
        {
          if (!Krys::Text::IsASCIIWhitespace(ch))
          {
            isWhitespaceOnly = false;
            break;
          }
        }
        if (isWhitespaceOnly)
        {
          // TODO: Insert character data to current node
          return;
        }
        // Non-whitespace: implicitly close head
        ChangeInsertionMode(InsertionMode::AfterHead);
        ProcessTokenInAfterHeadMode(token);
        break;
      }
      case HTMLTokenType::EndTag:
      {
        auto name = token.Name();
        if (name == DOMStringAtom {u8"head"})
        {
          // TODO: Pop head element from stack
          ChangeInsertionMode(InsertionMode::AfterHead);
        }
        break;
      }
      case HTMLTokenType::EndOfFile:
        // Stop parsing
        break;
      default:
        break;
    }
  }

  void HTMLTreeBuilder::ProcessTokenInInHeadNoscriptMode(const HTMLTokenAtom &token) noexcept
  {
    // TODO: Implement InHeadNoscript mode
  }

  void HTMLTreeBuilder::ProcessTokenInAfterHeadMode(const HTMLTokenAtom &token) noexcept
  {
    // TODO: Implement AfterHead mode
  }

  void HTMLTreeBuilder::ProcessTokenInBodyMode(const HTMLTokenAtom &token) noexcept
  {
    // TODO: Implement InBody mode
  }

  void HTMLTreeBuilder::ProcessTokenInTextMode(const HTMLTokenAtom &token) noexcept
  {
    // TODO: Implement Text mode
  }

  void HTMLTreeBuilder::ProcessTokenInTableMode(const HTMLTokenAtom &token) noexcept
  {
    // TODO: Implement InTable mode
  }
}
