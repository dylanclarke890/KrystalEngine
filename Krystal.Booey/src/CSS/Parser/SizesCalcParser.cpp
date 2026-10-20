#include "Krystal.Booey/CSS/Parser/SizesCalcParser.hpp"
#include "Krystal.Booey/CSS/Parser/SizesAttributeParser.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <algorithm>

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD bool OperatorPriority(char32 cc, bool &highPriority) noexcept
    {
      if (cc == U'+' || cc == U'-')
      {
        highPriority = false;
      }
      else if (cc == U'*' || cc == U'/')
      {
        highPriority = true;
      }
      else
      {
        return false;
      }
      return true;
    }

    KRYS_NODISCARD bool OperateOnStack(List<SizesCalcValue> &stack, char32 operation) noexcept
    {
      if (stack.size() < 2)
      {
        return false;
      }
      SizesCalcValue rightOperand = stack.back();
      stack.pop_back();
      SizesCalcValue leftOperand = stack.back();
      stack.pop_back();
      bool isLength;
      switch (operation)
      {
        case U'+':
          if (rightOperand.IsLength != leftOperand.IsLength)
          {
            return false;
          }
          isLength = (rightOperand.IsLength && leftOperand.IsLength);
          stack.emplace_back(leftOperand.Value + rightOperand.Value, isLength);
          break;
        case U'-':
          if (rightOperand.IsLength != leftOperand.IsLength)
          {
            return false;
          }
          isLength = (rightOperand.IsLength && leftOperand.IsLength);
          stack.emplace_back(leftOperand.Value - rightOperand.Value, isLength);
          break;
        case U'*':
          if (rightOperand.IsLength && leftOperand.IsLength)
          {
            return false;
          }
          isLength = (rightOperand.IsLength || leftOperand.IsLength);
          stack.emplace_back(leftOperand.Value * rightOperand.Value, isLength);
          break;
        case U'/':
          if (rightOperand.IsLength || !rightOperand.Value)
          {
            return false;
          }
          stack.emplace_back(leftOperand.Value / rightOperand.Value, leftOperand.IsLength);
          break;
        default: return false;
      }
      return true;
    }
  }

  SizesCalcParser::SizesCalcParser(TokenRange range, const dom::Document &document) noexcept
      : _document(document)
  {
    _isValid = CalcToReversePolishNotation(range) && Calculate();
  }

  float SizesCalcParser::Result() const noexcept
  {
    krys_debug_assert(_isValid);
    return _result;
  }

  bool SizesCalcParser::HandleOperator(List<Token> &stack, const Token &token) noexcept
  {
    // If the token is not an operator, then return. Else determine the
    // precedence of the new operator (op1).
    bool incomingOperatorPriority;
    if (!OperatorPriority(token.Delimiter(), incomingOperatorPriority))
    {
      return false;
    }
    while (!stack.empty())
    {
      // While there is an operator (op2) at the top of the stack,
      // determine its precedence, and...
      if (stack.back().Type() != TokenType::Delim)
      {
        break;
      }
      bool stackOperatorPriority;
      if (!OperatorPriority(stack.back().Delimiter(), stackOperatorPriority))
      {
        return false;
      }
      // ...if op1 is left-associative (all currently supported
      // operators are) and its precedence is equal to that of op2, or
      // op1 has precedence less than that of op2, ...
      if (incomingOperatorPriority && !stackOperatorPriority)
      {
        break;
      }
      // ...pop op2 off the stack and add it to the output queue.
      AppendOperator(stack.back());
      stack.pop_back();
    }
    // Push op1 onto the stack.
    stack.push_back(token);
    return true;
  }

  void SizesCalcParser::AppendNumber(const Token &token) noexcept
  {
    SizesCalcValue value;
    value.Value = token.NumericValue();
    _valueList.push_back(value);
  }

  bool SizesCalcParser::AppendLength(const Token &token) noexcept
  {
    auto lengthUnit = ToLengthUnit(ParseUnitType(token.Unit()));
    if (!lengthUnit)
    {
      return false;
    }
    SizesCalcValue value;
    value.Value = SizesAttributeParser::ComputeLength(token.NumericValue(), *lengthUnit, _document);
    value.IsLength = true;
    _valueList.push_back(value);
    return true;
  }

  void SizesCalcParser::AppendOperator(const Token &token) noexcept
  {
    SizesCalcValue value;
    value.Operation = token.Delimiter();
    _valueList.push_back(value);
  }

  bool SizesCalcParser::CalcToReversePolishNotation(TokenRange range) noexcept
  {
    // This method implements the shunting yard algorithm, to turn the calc syntax into a reverse polish
    // notation. http://en.wikipedia.org/wiki/Shunting-yard_algorithm

    List<Token> stack;
    while (!range.IsAtEnd())
    {
      const Token &token = range.Consume();
      switch (token.Type())
      {
        case TokenType::Number: AppendNumber(token); break;
        case TokenType::Dimension:
          if (!AppendLength(token))
          {
            return false;
          }
          break;
        case TokenType::Delim:
          if (!HandleOperator(stack, token))
          {
            return false;
          }
          break;
        case TokenType::Function:
          if (token.FunctionId() != ValueId::Calc)
          {
            return false;
          }
          // "calc(" is the same as "("
          [[fallthrough]];
        case TokenType::OpenParen:
          // If the token is a left parenthesis, then push it onto the stack.
          stack.push_back(token);
          break;
        case TokenType::CloseParen:
          // If the token is a right parenthesis:
          // Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto
          // the output queue.
          while (!stack.empty() && stack.back().Type() != TokenType::OpenParen
                 && stack.back().Type() != TokenType::Function)
          {
            AppendOperator(stack.back());
            stack.pop_back();
          }
          // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
          if (stack.empty())
          {
            return false;
          }
          // Pop the left parenthesis from the stack, but not onto the output queue.
          stack.pop_back();
          break;
        case TokenType::Whitespace:
        case TokenType::EndOfFile:  break;
        default:                    return false;
      }
    }

    // When there are no more tokens to read:
    // While there are still operator tokens in the stack:
    while (!stack.empty())
    {
      // If the operator token on the top of the stack is a parenthesis, then there are unclosed parentheses.
      TokenType type = stack.back().Type();
      if (type != TokenType::OpenParen && type != TokenType::Function)
      {
        // Pop the operator onto the output queue.
        AppendOperator(stack.back());
      }
      stack.pop_back();
    }
    return true;
  }

  bool SizesCalcParser::Calculate() noexcept
  {
    List<SizesCalcValue> stack;
    for (const auto &value : _valueList)
    {
      if (!value.Operation)
      {
        stack.push_back(value);
      }
      else
      {
        if (!OperateOnStack(stack, value.Operation))
        {
          return false;
        }
      }
    }
    if (stack.size() == 1 && stack.back().IsLength)
    {
      _result = std::max(ClampTo<float>(stack.back().Value), 0.f);
      return true;
    }
    return false;
  }
}
