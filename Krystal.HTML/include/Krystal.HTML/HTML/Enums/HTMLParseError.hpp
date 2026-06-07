#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class HTMLParseError : uint8
  {
    UnexpectedNullCharacter,
    UnexpectedQuestionMarkInsteadOfTagName,
    EOFBeforeTagName,
    InvalidFirstCharacterOfTagName,
    MissingEndTagName,
    EOFInTag,
    EOFInScriptHTMLCommentLikeText,
    UnexpectedEqualsSignBeforeAttributeName,
    UnexpectedCharacterInAttributeName,
    DuplicateAttribute,
    MissingAttributeValue,
    UnexpectedCharacterInUnquotedAttributeValue,
    MissingWhitespaceBetweenAttributes,
    UnexpectedSolidusInTag,
    CDATAInHTMLContent,
    IncorrectlyOpenedComment,
    AbruptClosingOfEmptyComment,
    EOFInComment,
    NestedComment,
    IncorrectlyClosedComment,
    EOFInDOCTYPE,
    MissingWhitespaceBeforeDOCTYPEName,
    InvalidCharacterSequenceAfterDOCTYPEName,
    MissingDOCTYPEName,
    MissingWhitespaceAfterDOCTYPEPublicKeyword,
    MissingDOCTYPEPublicIdentifier,
    MissingQuoteBeforeDOCTYPEPublicIdentifier,
    AbruptDOCTYPEPublicIdentifier,
    MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers,
    MissingQuoteBeforeDOCTYPESystemIdentifier,
    MissingWhitespaceAfterDOCTYPESystemKeyword,
    MissingDOCTYPESystemIdentifier,
    AbruptDOCTYPESystemIdentifier,
    UnexpectedCharacterAfterDOCTYPESystemIdentifier,
    EOFInCDATA,
    UnknownNamedCharacterReference,
    AbsenceOfDigitsInNumericCharacterReference,
    MissingSemicolonAfterCharacterReference,
    NullCharacterReference,
    CharacterReferenceOutsideUnicodeRange,
    SurrogateCharacterReference,
    NonCharacterCharacterReference,
    ControlCharacterReference,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::HTMLParseError, 43uz);