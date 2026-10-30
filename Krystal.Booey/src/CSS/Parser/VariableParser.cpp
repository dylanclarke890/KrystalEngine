#include "Krystal.Booey/CSS/Parser/VariableParser.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::css
{
  bool VariableParser::IsValidVariableName(const Token &token) noexcept
  {
    if (token.Type() != TokenType::Ident)
    {
      return false;
    }

    return IsCustomPropertyName(token.IdentCodePoints());
  }

  namespace
  {
    KRYS_NODISCARD bool IsValidConstantName(const Token &token) noexcept
    {
      return token.Type() == TokenType::Ident;
    }

    KRYS_NODISCARD bool IsValidVariableReference(TokenRange range,
                                                 const ParserContext &parserContext) noexcept;
    KRYS_NODISCARD bool IsValidConstantReference(TokenRange range,
                                                 const ParserContext &parserContext) noexcept;
    KRYS_NODISCARD bool IsValidDashedFunction(TokenRange range, const ParserContext &parserContext) noexcept;

    struct ClassifyBlockResult
    {
      bool HasReferences {false};
      bool HasTopLevelBraceBlockMixedWithOtherValues {false};
      bool HasEmptyTopLevelBraceBlock {false};
    };

    KRYS_NODISCARD Maybe<ClassifyBlockResult> ClassifyBlock(TokenRange range,
                                                            const ParserContext &parserContext) noexcept
    {
      struct ClassifyBlockState
      {
        TokenRange Range;
        bool IsTopLevelBlock = true;
        bool HasOtherValues = false;
        uint32 TopLevelBraceBlocks = 0u;
        bool DoneWithThisRange = false;
      };

      List<ClassifyBlockState> stack;
      stack.push_back(ClassifyBlockState {.Range = range});

      auto result = ClassifyBlockResult {};

      while (!stack.empty())
      {
        auto &current = stack.back();
        if (current.DoneWithThisRange)
        {
          // If there is a top level brace block, the value should contains only that.
          if (current.TopLevelBraceBlocks > 1u
              || (current.TopLevelBraceBlocks == 1u && current.HasOtherValues))
          {
            result.HasTopLevelBraceBlockMixedWithOtherValues = true;
          }
          stack.pop_back();
          continue;
        }

        if (current.Range.IsAtEnd())
        {
          current.DoneWithThisRange = true;
          continue;
        }

        if (current.IsTopLevelBlock)
        {
          auto tokenType = current.Range.Peek().Type();
          if (tokenType != TokenType::Whitespace)
          {
            if (tokenType == TokenType::OpenCurly)
            {
              current.TopLevelBraceBlocks++;
            }
            else
            {
              current.HasOtherValues = true;
            }
          }
        }

        if (current.Range.Peek().IsBlockStart())
        {
          const Token &token = current.Range.Peek();
          TokenRange block = current.Range.ConsumeBlock();
          block.DiscardWhitespace();

          if (token.Type() == TokenType::OpenCurly && current.IsTopLevelBlock && block.IsAtEnd())
          {
            result.HasEmptyTopLevelBraceBlock = true;
          }

          if (token.FunctionId() == ValueId::Var)
          {
            if (!IsValidVariableReference(block, parserContext))
            {
              return {};
            }
            result.HasReferences = true;
            continue;
          }
          if (token.FunctionId() == ValueId::Env)
          {
            if (!IsValidConstantReference(block, parserContext))
            {
              return {};
            }
            result.HasReferences = true;
            continue;
          }
          if (token.Type() == TokenType::Function && IsCustomPropertyName(token.IdentCodePoints()))
          {
            // https://drafts.csswg.org/css-mixins/#typedef-dashed-function
            if (!IsValidDashedFunction(block, parserContext))
            {
              return {};
            }
            result.HasReferences = true;
            continue;
          }
          // Nested block, not top-level. This invalidates `current`.
          stack.push_back(ClassifyBlockState {.Range = block, .IsTopLevelBlock = false});
          continue;
        }

        krys_debug_assert(!current.Range.Peek().IsBlockEnd());

        const Token &token = current.Range.Consume();
        switch (token.Type())
        {
          case TokenType::AtKeyword: break;
          case TokenType::Delim:
          {
            if (token.Delimiter() == U'!' && current.IsTopLevelBlock)
            {
              return {};
            }
            break;
          }
          case TokenType::CloseParen:
          case TokenType::CloseCurly:
          case TokenType::CloseSquare:
          case TokenType::BadString:
          case TokenType::BadUrl:      return {};
          case TokenType::Semicolon:
          {
            if (current.IsTopLevelBlock)
            {
              return {};
            }
            break;
          }
          default: break;
        }
      }

      return result;
    }

    bool IsValidVariableReference(TokenRange range, const ParserContext &parserContext) noexcept
    {
      range.DiscardWhitespace();
      if (!VariableParser::IsValidVariableName(range.ConsumeIncludingWhitespace()))
      {
        return false;
      }
      if (range.IsAtEnd())
      {
        return true;
      }

      if (!PropertyParserHelpers::ConsumeComma(range))
      {
        return false;
      }
      if (range.IsAtEnd())
      {
        return true;
      }

      return ClassifyBlock(range, parserContext).has_value();
    }

    bool IsValidConstantReference(TokenRange range, const ParserContext &parserContext) noexcept
    {
      range.DiscardWhitespace();
      if (!IsValidConstantName(range.ConsumeIncludingWhitespace()))
      {
        return false;
      }
      if (range.IsAtEnd())
      {
        return true;
      }

      if (!PropertyParserHelpers::ConsumeComma(range))
      {
        return false;
      }
      if (range.IsAtEnd())
      {
        return true;
      }

      return ClassifyBlock(range, parserContext).has_value();
    }

    bool IsValidDashedFunction(TokenRange range, const ParserContext &parserContext) noexcept
    {
      // <dashed-function> --*( <declaration-value>#? )
      range.DiscardWhitespace();

      auto validateArgument = [&](TokenRange argumentRange)
      {
        if (argumentRange.IsAtEnd())
        {
          return false;
        }

        // https://drafts.csswg.org/css-values-5/#component-function-commas
        // Empty brace blocks are just empty values.
        auto result = ClassifyBlock(argumentRange, parserContext);
        return result && !result->HasTopLevelBraceBlockMixedWithOtherValues
               && !result->HasEmptyTopLevelBraceBlock;
      };

      size_t index = 0uz;
      while (auto argumentRange = PropertyParserHelpers::ConsumeArgument(range, index))
      {
        if (!validateArgument(*argumentRange))
        {
          return false;
        }
        ++index;
      }
      return true;
    }

    struct VariableType
    {
      Maybe<GlobalKeyword> Keyword {};
      css::ClassifyBlockResult ClassifyBlockResult {};
    };

    KRYS_NODISCARD Maybe<VariableType> ClassifyVariableRange(TokenRange range,
                                                             const ParserContext &parserContext) noexcept
    {
      range.DiscardWhitespace();

      if (range.Peek().Type() == TokenType::Ident)
      {
        auto rangeCopy = range;
        auto id = range.ConsumeIncludingWhitespace().ValueId();
        if (auto keyword = ParseGlobalKeyword(id); range.IsAtEnd() && keyword)
        {
          return VariableType {.Keyword = *keyword};
        }
        // No fast path, restart with the complete range.
        range = rangeCopy;
      }

      auto classifyBlockResult = ClassifyBlock(range, parserContext);
      if (!classifyBlockResult)
      {
        return {};
      }

      return VariableType {.ClassifyBlockResult = *classifyBlockResult};
    }
  }

  bool VariableParser::ContainsValidVariableReferences(TokenRange range,
                                                       const ParserContext &parserContext) noexcept
  {
    auto type = ClassifyVariableRange(range, parserContext);
    if (!type)
    {
      return false;
    }

    return type->ClassifyBlockResult.HasReferences
           && !type->ClassifyBlockResult.HasTopLevelBraceBlockMixedWithOtherValues;
  }

  RefPtr<CustomPropertyValue>
    VariableParser::ParseDeclarationValue(const CSSOMStringAtom &variableName, TokenRange range,
                                          const ParserContext &parserContext) noexcept
  {
    auto type = ClassifyVariableRange(range, parserContext);
    if (!type)
    {
      return nullptr;
    }

    if (type->Keyword)
    {
      return CustomPropertyValue::CreateWithGlobalKeyword(variableName, *type->Keyword);
    }

    if (type->ClassifyBlockResult.HasReferences)
    {
      return CustomPropertyValue::CreateUnresolved(variableName,
                                                   VariableReferenceValue::Create(range, parserContext));
    }

    return CustomPropertyValue::CreateSyntaxAll(variableName, VariableData::Create(range, parserContext));
  }

  RefPtr<const style::CustomProperty>
    VariableParser::ParseInitialValueForUniversalSyntax(const CSSOMStringAtom &variableName,
                                                        TokenRange range) noexcept
  {
    auto type = ClassifyVariableRange(range, StrictParserContext());

    if (!type || type->Keyword || type->ClassifyBlockResult.HasReferences)
    {
      return nullptr;
    }

    return style::CustomProperty::CreateForVariableData(variableName, VariableData::Create(range));
  }
}
