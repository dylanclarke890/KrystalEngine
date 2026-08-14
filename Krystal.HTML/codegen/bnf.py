import enum
import re
import collections
from utils import stringify_iterable, StringEqualingEnum

BNF_ILLEGAL_TOKEN = "ILLEGAL"
BNF_EOF_TOKEN = "EOF"

BNFTokenInfo = collections.namedtuple("BNFTokens", ["name", "value"])


class BNFToken(StringEqualingEnum):
    # Numbers.
    FLOAT = re.compile(r"\-?\d+\.\d+")
    INT = re.compile(r"\-?\d+")

    # Brackets.
    LPAREN = re.compile(r"\(")
    RPAREN = re.compile(r"\)")
    LBRACE = re.compile(r"\{")
    RBRACE = re.compile(r"\}")
    LSQUARE = re.compile(r"\[")
    RSQUARE = re.compile(r"\]")
    LTLT = re.compile(r"<<")
    GTGT = re.compile(r">>")
    LT = re.compile(r"<")
    GT = re.compile(r">")
    SQUOTE = re.compile(r"\'")
    ATPAREN = re.compile(r"@\(")

    # Multipliers.
    HASH = re.compile(r"#")
    PLUS = re.compile(r"\+")
    STAR = re.compile(r"\*")
    NOT = re.compile(r"!")
    QMARK = re.compile(r"\?")

    # Combinators.
    OROR = re.compile(r"\|\|")
    OR = re.compile(r"\|")
    ANDAND = re.compile(r"&&")
    COMMA = re.compile(r",")

    # Literals
    SLASH = re.compile(r"/")
    EQUAL = re.compile(r"=")

    # Identifiers.
    FUNC = re.compile(r"[_a-zA-Z\-][_a-zA-Z0-9\-]*\(")
    ID = re.compile(r"[_a-zA-Z\-][_a-zA-Z0-9\-]*")

    # Whitespace.
    WHITESPACE = re.compile(r"(\t|\n|\s|\r)+")


def BNFLexer(data):
    position = 0
    while position < len(data):
        for token_id in BNFToken:
            match = token_id.value.match(data, position)
            if match:
                position = match.end(0)
                if token_id == BNFToken.WHITESPACE:
                    # ignore whitespace
                    break
                yield BNFTokenInfo(token_id.name, match.group(0))
                break
        else:
            # in case pattern doesn't match send the character as illegal
            yield BNFTokenInfo(BNF_ILLEGAL_TOKEN, data[position])
            position += 1
    yield BNFTokenInfo(BNF_EOF_TOKEN, "\x00")


class BNFRepetitionModifier:
    class Kind(enum.Enum):
        EXACT = "{A}"
        AT_LEAST = "{A,}"
        BETWEEN = "{A,B}"

    def __init__(self):
        self.kind = None
        self.min = None
        self.max = None

    def __str__(self):
        if self.kind is None:
            return "[UNSET RepetitionModifier]"
        elif self.kind == BNFRepetitionModifier.Kind.EXACT:
            return "{" + str(self.min) + "}"
        elif self.kind == BNFRepetitionModifier.Kind.AT_LEAST:
            return "{" + str(self.min) + ",}"
        elif self.kind == BNFRepetitionModifier.Kind.BETWEEN:
            return "{" + str(self.min) + "," + str(self.max) + "}"
        raise Exception(f"Unknown repetition kind: {self.kind}")


# BNFAnnotations are introduced by trailing '@(foo=bar,baz bat)' and are an
# extension to the syntax used by CSS, added to allow passing additional
# metadata to the code generators.
class BNFAnnotation:
    class Directive:
        def __init__(self, name):
            self.name = name
            self.value = []

        def __str__(self):
            if self.value:
                return str(self.name) + "=" + ",".join(stringify_iterable(self.value))
            return str(self.name)

    def __init__(self):
        self.directives = []

    def __str__(self):
        return "@(" + " ".join(stringify_iterable(self.directives)) + ")"

    def add_directive(self, directive):
        self.directives.append(directive)


# Node multipliers are introduced by trailing symbols like '#', '+', '*', and '{1,4}'.
# https://drafts.csswg.org/css-values-4/#component-multipliers
class BNFNodeMultiplier:
    range: BNFRepetitionModifier

    class Kind(enum.Enum):
        ZERO_OR_ONE = "?"
        SPACE_SEPARATED_ZERO_OR_MORE = "*"
        SPACE_SEPARATED_ONE_OR_MORE = "+"
        SPACE_SEPARATED_EXACT = "{A}"
        SPACE_SEPARATED_AT_LEAST = "{A,}"
        SPACE_SEPARATED_BETWEEN = "{A,B}"
        COMMA_SEPARATED_ONE_OR_MORE = "#"
        COMMA_SEPARATED_EXACT = "#{A}"
        COMMA_SEPARATED_AT_LEAST = "#{A,}"
        COMMA_SEPARATED_BETWEEN = "#{A,B}"

    def __init__(self):
        self.kind = None
        self.range = None  # type: ignore
        self.annotation = None

    def __str__(self):
        if self.annotation:
            return self.stringified_without_annotation + str(self.annotation)
        return self.stringified_without_annotation

    @property
    def stringified_without_annotation(self):
        if self.kind == BNFNodeMultiplier.Kind.ZERO_OR_ONE:
            return "?"
        elif self.kind == BNFNodeMultiplier.Kind.SPACE_SEPARATED_ZERO_OR_MORE:
            return "*"
        elif self.kind == BNFNodeMultiplier.Kind.SPACE_SEPARATED_ONE_OR_MORE:
            return "+"
        elif self.kind == BNFNodeMultiplier.Kind.SPACE_SEPARATED_EXACT:
            return "{" + str(self.range.min) + "}"
        elif self.kind == BNFNodeMultiplier.Kind.SPACE_SEPARATED_AT_LEAST:
            return "{" + str(self.range.min) + ",}"
        elif self.kind == BNFNodeMultiplier.Kind.SPACE_SEPARATED_BETWEEN:
            return "{" + str(self.range.min) + "," + str(self.range.max) + "}"
        elif self.kind == BNFNodeMultiplier.Kind.COMMA_SEPARATED_ONE_OR_MORE:
            return "#"
        elif self.kind == BNFNodeMultiplier.Kind.COMMA_SEPARATED_EXACT:
            return "#" + "{" + str(self.range.min) + "}"
        elif self.kind == BNFNodeMultiplier.Kind.COMMA_SEPARATED_AT_LEAST:
            return "#" + "{" + str(self.range.min) + ",}"
        elif self.kind == BNFNodeMultiplier.Kind.COMMA_SEPARATED_BETWEEN:
            return "#" + "{" + str(self.range.min) + "," + str(self.range.max) + "}"
        return ""

    def add(self, multiplier):
        if self.annotation:
            raise Exception(
                "Invalid to stack another multiplier on top of a multiplier that has already received an annotation."
            )

        if self.kind is None:
            if isinstance(multiplier, BNFRepetitionModifier):
                if multiplier.kind == BNFRepetitionModifier.Kind.EXACT:
                    self.kind = BNFNodeMultiplier.Kind.SPACE_SEPARATED_EXACT
                    self.range = multiplier
                elif multiplier.kind == BNFRepetitionModifier.Kind.AT_LEAST:
                    self.kind = BNFNodeMultiplier.Kind.SPACE_SEPARATED_AT_LEAST
                    self.range = multiplier
                elif multiplier.kind == BNFRepetitionModifier.Kind.BETWEEN:
                    self.kind = BNFNodeMultiplier.Kind.SPACE_SEPARATED_BETWEEN
                    self.range = multiplier
            else:
                self.kind = BNFNodeMultiplier.Kind(multiplier)
        elif self.kind == BNFNodeMultiplier.Kind.ZERO_OR_ONE:
            raise Exception("Invalid to stack another multiplier on top of '?'")
        elif self.kind == BNFNodeMultiplier.Kind.SPACE_SEPARATED_ZERO_OR_MORE:
            raise Exception("Invalid to stack another multiplier on top of '*'")
        elif self.kind == BNFNodeMultiplier.Kind.SPACE_SEPARATED_ONE_OR_MORE:
            raise Exception("Invalid to stack another multiplier on top of '+'")
        elif self.kind == BNFNodeMultiplier.Kind.SPACE_SEPARATED_EXACT:
            raise Exception("Invalid to stack another multiplier on top of a range.")
        elif self.kind == BNFNodeMultiplier.Kind.SPACE_SEPARATED_AT_LEAST:
            raise Exception("Invalid to stack another multiplier on top of a range.")
        elif self.kind == BNFNodeMultiplier.Kind.SPACE_SEPARATED_BETWEEN:
            raise Exception("Invalid to stack another multiplier on top of a range.")
        elif self.kind == BNFNodeMultiplier.Kind.COMMA_SEPARATED_ONE_OR_MORE:
            if isinstance(multiplier, BNFRepetitionModifier):
                if multiplier.kind == BNFRepetitionModifier.Kind.EXACT:
                    self.kind = BNFNodeMultiplier.Kind.COMMA_SEPARATED_EXACT
                    self.range = multiplier
                elif multiplier.kind == BNFRepetitionModifier.Kind.AT_LEAST:
                    self.kind = BNFNodeMultiplier.Kind.COMMA_SEPARATED_AT_LEAST
                    self.range = multiplier
                elif multiplier.kind == BNFRepetitionModifier.Kind.BETWEEN:
                    self.kind = BNFNodeMultiplier.Kind.COMMA_SEPARATED_BETWEEN
                    self.range = multiplier
            else:
                raise Exception("Invalid to stack a non-range multiplier on top of '#'.")
        elif self.kind == BNFNodeMultiplier.Kind.COMMA_SEPARATED_EXACT:
            raise Exception("Invalid to stack another multiplier on top of a comma modifier range multiplier.")
        elif self.kind == BNFNodeMultiplier.Kind.COMMA_SEPARATED_AT_LEAST:
            raise Exception("Invalid to stack another multiplier on top of a comma modifier range multiplier.")
        elif self.kind == BNFNodeMultiplier.Kind.COMMA_SEPARATED_BETWEEN:
            raise Exception("Invalid to stack another multiplier on top of a comma modifier range multiplier.")

    def add_annotation(self, annotation):
        if self.annotation:
            raise Exception("Invalid to add an annotation to a multiplier node that already has an annotation.")

        SUPPORTED_DIRECTIVES = {
            "no-single-item-opt": {
                BNFNodeMultiplier.Kind.SPACE_SEPARATED_ZERO_OR_MORE,
                BNFNodeMultiplier.Kind.SPACE_SEPARATED_ONE_OR_MORE,
                BNFNodeMultiplier.Kind.SPACE_SEPARATED_AT_LEAST,
                BNFNodeMultiplier.Kind.SPACE_SEPARATED_BETWEEN,
                BNFNodeMultiplier.Kind.COMMA_SEPARATED_ONE_OR_MORE,
                BNFNodeMultiplier.Kind.COMMA_SEPARATED_AT_LEAST,
                BNFNodeMultiplier.Kind.COMMA_SEPARATED_BETWEEN,
            },
            "type": {
                BNFNodeMultiplier.Kind.SPACE_SEPARATED_ZERO_OR_MORE,
                BNFNodeMultiplier.Kind.SPACE_SEPARATED_ONE_OR_MORE,
                BNFNodeMultiplier.Kind.SPACE_SEPARATED_BETWEEN,
                BNFNodeMultiplier.Kind.SPACE_SEPARATED_EXACT,
                BNFNodeMultiplier.Kind.COMMA_SEPARATED_ONE_OR_MORE,
                BNFNodeMultiplier.Kind.COMMA_SEPARATED_BETWEEN,
                BNFNodeMultiplier.Kind.COMMA_SEPARATED_EXACT,
            },
            "default": {
                BNFNodeMultiplier.Kind.SPACE_SEPARATED_BETWEEN,
                BNFNodeMultiplier.Kind.COMMA_SEPARATED_BETWEEN,
            },
            "settings-flag": "*",
        }

        for directive in annotation.directives:
            if directive.name not in SUPPORTED_DIRECTIVES:
                raise Exception(f"Unknown annotation directive '{directive}' for multiplier '{self}'.")
            if SUPPORTED_DIRECTIVES[directive.name] != "*" and self.kind not in SUPPORTED_DIRECTIVES[directive.name]:
                raise Exception(f"Unsupported annotation directive '{directive}' for multiplier '{self}'.")

        self.annotation = annotation


# https://drafts.csswg.org/css-values-4/#component-combinators
class BNFGroupingNode:
    class Kind(enum.Enum):
        MATCH_ALL_ORDERED = " "  # [ <length>    <integer>    <percentage> ]
        MATCH_ONE = "|"  # [ <length>  | <integer>  | <percentage> ]
        MATCH_ALL_ANY_ORDER = "&&"  # [ <length> && <integer> && <percentage> ]
        MATCH_ONE_OR_MORE_ANY_ORDER = "||"  # [ <length> || <integer> || <percentage> ]

    def __init__(self, *, is_initial=False):
        self.kind = BNFGroupingNode.Kind.MATCH_ALL_ORDERED
        self.members = []
        self.multiplier = BNFNodeMultiplier()
        self.is_initial = is_initial
        self.annotation = None

    def __str__(self):
        return self.stringified_without_multipliers + str(self.multiplier)

    @property
    def stringified_without_multipliers(self):
        if self.is_initial:
            return self.stringified_without_brackets_or_multipliers
        return "[ " + self.stringified_without_brackets_or_multipliers + " ]"

    @property
    def stringified_without_brackets_or_multipliers(self):
        if self.kind != BNFGroupingNode.Kind.MATCH_ALL_ORDERED:
            join_string = " " + self.kind.value + " "
        else:
            join_string = " "

        return join_string.join(stringify_iterable(self.members))

    def add(self, member):
        self.members.append(member)

    def add_annotation(self, annotation):
        if self.annotation:
            raise Exception("Invalid to add an annotation to a grouping node that already has an annotation.")

        SUPPORTED_DIRECTIVES = {
            "no-single-item-opt": {
                BNFGroupingNode.Kind.MATCH_ALL_ORDERED,
                BNFGroupingNode.Kind.MATCH_ALL_ANY_ORDER,
                BNFGroupingNode.Kind.MATCH_ONE_OR_MORE_ANY_ORDER,
            },
            "preserve-order": {
                BNFGroupingNode.Kind.MATCH_ALL_ANY_ORDER,
                BNFGroupingNode.Kind.MATCH_ONE_OR_MORE_ANY_ORDER,
            },
            "type": {
                BNFGroupingNode.Kind.MATCH_ALL_ORDERED,
                BNFGroupingNode.Kind.MATCH_ALL_ANY_ORDER,
                BNFGroupingNode.Kind.MATCH_ONE_OR_MORE_ANY_ORDER,
            },
            "settings-flag": "*",
        }

        for directive in annotation.directives:
            if directive.name not in SUPPORTED_DIRECTIVES:
                raise Exception(f"Unknown annotation directive '{directive}' for grouping '{self}'.")
            if SUPPORTED_DIRECTIVES[directive.name] != "*" and self.kind not in SUPPORTED_DIRECTIVES[directive.name]:
                raise Exception(f"Unsupported annotation directive '{directive}' for grouping '{self}'.")

        self.annotation = annotation


# https://drafts.csswg.org/css-values-4/#functional-notation
class BNFFunctionNode:
    def __init__(self, name):
        self.name = name
        self.parameter_group = BNFGroupingNode()
        self.multiplier = BNFNodeMultiplier()
        self.annotation = None

    def __str__(self):
        return self.stringified_without_multipliers + str(self.multiplier)

    @property
    def stringified_without_multipliers(self):
        return self.name + "(" + self.parameter_group.stringified_without_brackets_or_multipliers + ")"

    @property
    def kind(self):
        return self.parameter_group.kind

    @kind.setter
    def kind(self, kind):
        self.parameter_group.kind = kind

    def add(self, member):
        self.parameter_group.add(member)

    def add_annotation(self, annotation):
        if self.annotation:
            raise Exception("Invalid to add an annotation to a function node that already has an annotation.")

        SUPPORTED_DIRECTIVES = {"settings-flag"}

        for directive in annotation.directives:
            if directive.name not in SUPPORTED_DIRECTIVES:
                raise Exception(f"Unknown annotation directive '{directive}' for function node '{self}'.")

        self.annotation = annotation


class BNFReferenceNode:
    class StringAttribute:
        def __init__(self, name):
            self.name = name
            self.value = []

        def __str__(self):
            if self.value:
                return str(self.name) + "=" + str(self.value)
            return str(self.name)

    class RangeAttribute:
        def __init__(self):
            self.min = None
            self.max = None

        def __str__(self):
            return "[" + str(self.min) + "," + str(self.max) + "]"

    def __init__(self, *, is_internal=False):
        self.name = None
        self.is_internal = is_internal
        self.is_function_reference = False
        self.attributes = []
        self.multiplier = BNFNodeMultiplier()
        self.annotation = None

    def __str__(self):
        return self.stringified_without_multipliers + str(self.multiplier)

    @property
    def stringified_without_multipliers(self):
        if self.is_internal:
            prefix = "<<"
            suffix = ">>"
        else:
            prefix = "<"
            suffix = ">"

        if self.is_function_reference:
            name = str(self.name) + "()"
        else:
            name = self.name

        if self.attributes:
            return prefix + str(name) + " " + " ".join(stringify_iterable(self.attributes)) + suffix
        return prefix + str(name) + suffix

    def add_attribute(self, attribute):
        self.attributes.append(attribute)

    def add_annotation(self, annotation):
        if self.annotation:
            raise Exception("Invalid to add an annotation to a reference node that already has an annotation.")

        SUPPORTED_DIRECTIVES = {"settings-flag"}

        for directive in annotation.directives:
            if directive.name not in SUPPORTED_DIRECTIVES:
                raise Exception(f"Unknown annotation directive '{directive}' for reference node '{self}'.")

        self.annotation = annotation


class BNFKeywordNode:
    def __init__(self, keyword):
        self.keyword = keyword
        self.multiplier = BNFNodeMultiplier()
        self.annotation = None

    def __str__(self):
        return self.stringified_without_multipliers + str(self.multiplier)

    @property
    def stringified_without_multipliers(self):
        return self.keyword

    def add_annotation(self, annotation):
        if self.annotation:
            raise Exception("Invalid to add an annotation to a keyword node that already has an annotation.")

        SUPPORTED_DIRECTIVES = {
            "aliased-to",
            "settings-flag",
        }

        for directive in annotation.directives:
            if directive.name not in SUPPORTED_DIRECTIVES:
                raise Exception(f"Unknown annotation directive '{directive}' for keyword '{self}'.")

        self.annotation = annotation


class BNFLiteralNode:
    def __init__(self, value=None):
        self.value = value
        self.multiplier = BNFNodeMultiplier()
        self.annotation = None

    def __str__(self):
        return self.stringified_without_multipliers + str(self.multiplier)

    @property
    def stringified_without_multipliers(self):
        return str(self.value)

    def add_annotation(self, annotation):
        if self.annotation:
            raise Exception("Invalid to add an annotation to a literal node that already has an annotation.")

        SUPPORTED_DIRECTIVES = {}

        for directive in annotation.directives:
            if directive.name not in SUPPORTED_DIRECTIVES:
                raise Exception(f"Unknown annotation directive '{directive}' for literal '{self}'.")

        self.annotation = annotation


class BNFParserState(enum.Enum):
    UNKNOWN_GROUPING_INITIAL = enum.auto()
    UNKNOWN_GROUPING_SEEN_TERM = enum.auto()
    KNOWN_ORDERED_GROUPING = enum.auto()
    KNOWN_COMBINATOR_GROUPING_TERM_REQUIRED = enum.auto()
    KNOWN_COMBINATOR_GROUPING_COMBINATOR_OR_CLOSE_REQUIRED = enum.auto()
    INTERNAL_REFERENCE_INITIAL = enum.auto()
    INTERNAL_REFERENCE_SEEN_ID = enum.auto()
    REFERENCE_INITIAL = enum.auto()
    REFERENCE_SEEN_QUOTE_OPEN = enum.auto()
    REFERENCE_SEEN_QUOTE_AND_ID = enum.auto()
    REFERENCE_SEEN_FUNCTION_OPEN = enum.auto()
    REFERENCE_SEEN_ID_OR_FUNCTION = enum.auto()
    REFERENCE_STRING_ATTRIBUTE_INITIAL = enum.auto()
    REFERENCE_STRING_ATTRIBUTE_SEEN_EQUAL = enum.auto()
    REFERENCE_STRING_ATTRIBUTE_SEEN_VALUE = enum.auto()
    REFERENCE_RANGE_ATTRIBUTE_INITIAL = enum.auto()
    REFERENCE_RANGE_ATTRIBUTE_SEEN_MIN = enum.auto()
    REFERENCE_RANGE_ATTRIBUTE_SEEN_MIN_AND_COMMA = enum.auto()
    REFERENCE_RANGE_ATTRIBUTE_SEEN_MAX = enum.auto()
    REPETITION_MODIFIER_INITIAL = enum.auto()
    REPETITION_MODIFIER_SEEN_MIN = enum.auto()
    REPETITION_MODIFIER_SEEN_MIN_AND_COMMA = enum.auto()
    REPETITION_MODIFIER_SEEN_MAX = enum.auto()
    QUOTED_LITERAL_INITIAL = enum.auto()
    QUOTED_LITERAL_SEEN_ID = enum.auto()
    ANNOTATION_INITIAL = enum.auto()
    ANNOTATION_SEEN_ID = enum.auto()
    ANNOTATION_SEEN_EQUAL_OR_COMMA = enum.auto()
    ANNOTATION_SEEN_VALUE = enum.auto()
    DONE = enum.auto()


BNFParserStateInfo = collections.namedtuple("BNFParserStates", ["state", "node", "node_owner"])


class BNFParser:
    COMBINATOR_FOR_TOKEN = {
        BNFToken.OR.name: BNFGroupingNode.Kind.MATCH_ONE,
        BNFToken.OROR.name: BNFGroupingNode.Kind.MATCH_ONE_OR_MORE_ANY_ORDER,
        BNFToken.ANDAND.name: BNFGroupingNode.Kind.MATCH_ALL_ANY_ORDER,
    }

    SIMPLE_MULTIPLIERS = {
        BNFToken.HASH.name,
        BNFToken.PLUS.name,
        BNFToken.STAR.name,
        BNFToken.NOT.name,
        BNFToken.QMARK.name,
    }

    SUPPORTED_UNQUOTED_LITERALS = {
        BNFToken.COMMA.name,
        BNFToken.SLASH.name,
    }

    DEBUG_PRINT_STATE = 0
    DEBUG_PRINT_TOKENS = 0

    def __init__(self, parsing_context, key_path, data):
        self.parsing_context = parsing_context
        self.key_path = key_path
        self.data = data
        self.root = BNFGroupingNode(is_initial=True)
        self.state_stack = []
        self.multiplier_target = None
        self.annotation_target = None
        self.enter_initial_grouping()

    def parse(self):
        PARSER_THUNKS = {
            BNFParserState.UNKNOWN_GROUPING_INITIAL: BNFParser.parse_UNKNOWN_GROUPING_INITIAL,
            BNFParserState.UNKNOWN_GROUPING_SEEN_TERM: BNFParser.parse_UNKNOWN_GROUPING_SEEN_TERM,
            BNFParserState.KNOWN_ORDERED_GROUPING: BNFParser.parse_KNOWN_ORDERED_GROUPING,
            BNFParserState.KNOWN_COMBINATOR_GROUPING_TERM_REQUIRED: BNFParser.parse_KNOWN_COMBINATOR_GROUPING_TERM_REQUIRED,
            BNFParserState.KNOWN_COMBINATOR_GROUPING_COMBINATOR_OR_CLOSE_REQUIRED: BNFParser.parse_KNOWN_COMBINATOR_GROUPING_COMBINATOR_OR_CLOSE_REQUIRED,
            BNFParserState.INTERNAL_REFERENCE_INITIAL: BNFParser.parse_INTERNAL_REFERENCE_INITIAL,
            BNFParserState.INTERNAL_REFERENCE_SEEN_ID: BNFParser.parse_INTERNAL_REFERENCE_SEEN_ID,
            BNFParserState.REFERENCE_INITIAL: BNFParser.parse_REFERENCE_INITIAL,
            BNFParserState.REFERENCE_SEEN_QUOTE_OPEN: BNFParser.parse_REFERENCE_SEEN_QUOTE_OPEN,
            BNFParserState.REFERENCE_SEEN_QUOTE_AND_ID: BNFParser.parse_REFERENCE_SEEN_QUOTE_AND_ID,
            BNFParserState.REFERENCE_SEEN_FUNCTION_OPEN: BNFParser.parse_REFERENCE_SEEN_FUNCTION_OPEN,
            BNFParserState.REFERENCE_SEEN_ID_OR_FUNCTION: BNFParser.parse_REFERENCE_SEEN_ID_OR_FUNCTION,
            BNFParserState.REFERENCE_STRING_ATTRIBUTE_INITIAL: BNFParser.parse_REFERENCE_STRING_ATTRIBUTE_INITIAL,
            BNFParserState.REFERENCE_STRING_ATTRIBUTE_SEEN_EQUAL: BNFParser.parse_REFERENCE_STRING_ATTRIBUTE_SEEN_EQUAL,
            BNFParserState.REFERENCE_STRING_ATTRIBUTE_SEEN_VALUE: BNFParser.parse_REFERENCE_STRING_ATTRIBUTE_SEEN_VALUE,
            BNFParserState.REFERENCE_RANGE_ATTRIBUTE_INITIAL: BNFParser.parse_REFERENCE_RANGE_ATTRIBUTE_INITIAL,
            BNFParserState.REFERENCE_RANGE_ATTRIBUTE_SEEN_MIN: BNFParser.parse_REFERENCE_RANGE_ATTRIBUTE_SEEN_MIN,
            BNFParserState.REFERENCE_RANGE_ATTRIBUTE_SEEN_MIN_AND_COMMA: BNFParser.parse_REFERENCE_RANGE_ATTRIBUTE_SEEN_MIN_AND_COMMA,
            BNFParserState.REFERENCE_RANGE_ATTRIBUTE_SEEN_MAX: BNFParser.parse_REFERENCE_RANGE_ATTRIBUTE_SEEN_MAX,
            BNFParserState.REPETITION_MODIFIER_INITIAL: BNFParser.parse_REPETITION_MODIFIER_INITIAL,
            BNFParserState.REPETITION_MODIFIER_SEEN_MIN: BNFParser.parse_REPETITION_MODIFIER_SEEN_MIN,
            BNFParserState.REPETITION_MODIFIER_SEEN_MIN_AND_COMMA: BNFParser.parse_REPETITION_MODIFIER_SEEN_MIN_AND_COMMA,
            BNFParserState.REPETITION_MODIFIER_SEEN_MAX: BNFParser.parse_REPETITION_MODIFIER_SEEN_MAX,
            BNFParserState.QUOTED_LITERAL_INITIAL: BNFParser.parse_QUOTED_LITERAL_INITIAL,
            BNFParserState.QUOTED_LITERAL_SEEN_ID: BNFParser.parse_QUOTED_LITERAL_SEEN_ID,
            BNFParserState.ANNOTATION_INITIAL: BNFParser.parse_ANNOTATION_INITIAL,
            BNFParserState.ANNOTATION_SEEN_ID: BNFParser.parse_ANNOTATION_SEEN_ID,
            BNFParserState.ANNOTATION_SEEN_EQUAL_OR_COMMA: BNFParser.parse_ANNOTATION_SEEN_EQUAL_OR_COMMA,
            BNFParserState.ANNOTATION_SEEN_VALUE: BNFParser.parse_ANNOTATION_SEEN_VALUE,
        }

        for token in BNFLexer(self.data):
            if token.name == BNF_ILLEGAL_TOKEN:
                raise Exception(f"Illegal token found while parsing grammar definition: {token}")

            state = self.state_stack[-1]

            if BNFParser.DEBUG_PRINT_STATE:
                print("STATE: " + state.state.name + " " + str(state.node))
            if BNFParser.DEBUG_PRINT_TOKENS:
                print("TOKEN: " + str(token))
            PARSER_THUNKS[state.state](self, token, state)

        if self.state_stack[-1].state != BNFParserState.DONE:
            raise Exception(f"Unexpected state '{state.state.name}' after processing all tokens") # type: ignore

        return self.root

    def transition_top(self, *, to):
        self.state_stack[-1] = BNFParserStateInfo(to, self.state_stack[-1].node, self.state_stack[-1].node_owner)

    def push(self, new_state, new_node, node_owner):
        self.state_stack.append(BNFParserStateInfo(new_state, new_node, node_owner))

    def pop(self):
        self.state_stack.pop()

    @property
    def top(self):
        return self.state_stack[-1]

    def unexpected(self, token, state):
        return Exception(
            f"Unexpected token '{token}' found while in state '{state.state.name}' while parsing '{self.data}'"
        )

    # COMMON ACTIONS.

    # Root BNFGroupingNode. Syntactically isn't surrounded by square brackets.
    def enter_initial_grouping(self):
        self.push(BNFParserState.UNKNOWN_GROUPING_INITIAL, self.root, None)
        return self.top

    def exit_initial_grouping(self, token, state):
        if isinstance(state.node, BNFGroupingNode) and state.node.is_initial:
            self.transition_top(to=BNFParserState.DONE)
            return self.top
        raise self.unexpected(token, state)

    # Non-initial BNFGroupingNode. e.g. "[foo bar]", "[foo | bar]", etc.
    def enter_new_grouping(self, token, state):
        self.push(BNFParserState.UNKNOWN_GROUPING_INITIAL, BNFGroupingNode(), self.top.node)
        self.multiplier_target = None
        self.annotation_target = None
        return self.top

    def exit_grouping(self, token, state):
        if isinstance(state.node, BNFGroupingNode) and not state.node.is_initial:
            self.pop()
            state.node_owner.add(state.node)
            self.multiplier_target = state.node
            self.annotation_target = state.node
            return self.top
        raise self.unexpected(token, state)

    # BNFFunctionNode. e.g. "foo(<bar>)"
    def enter_new_function(self, token, state):
        self.push(BNFParserState.UNKNOWN_GROUPING_INITIAL, BNFFunctionNode(token.value[:-1]), self.top.node)
        self.multiplier_target = None
        self.annotation_target = None
        return self.top

    def exit_function(self, token, state):
        if isinstance(state.node, BNFFunctionNode):
            self.pop()
            state.node_owner.add(state.node)
            self.multiplier_target = state.node
            self.annotation_target = state.node
            return self.top
        raise self.unexpected(token, state)

    # Internal BNFReferenceNodes. e.g. "<<values>>"
    def enter_new_internal_reference(self, token, state):
        self.push(BNFParserState.INTERNAL_REFERENCE_INITIAL, BNFReferenceNode(is_internal=True), self.top.node)
        self.multiplier_target = None
        self.annotation_target = None
        return self.top

    def exit_internal_reference(self, token, state):
        if isinstance(state.node, BNFReferenceNode) and state.node.is_internal:
            self.pop()
            state.node_owner.add(state.node)
            self.multiplier_target = state.node
            self.annotation_target = state.node
            return self.top
        raise self.unexpected(token, state)

    # Non-internal BNFReferenceNodes. e.g. "<length>"
    def enter_new_reference(self, token, state):
        self.push(BNFParserState.REFERENCE_INITIAL, BNFReferenceNode(), self.top.node)
        self.multiplier_target = None
        self.annotation_target = None
        return self.top

    def exit_reference(self, token, state):
        if isinstance(state.node, BNFReferenceNode) and not state.node.is_internal:
            self.pop()
            state.node_owner.add(state.node)
            self.multiplier_target = state.node
            self.annotation_target = state.node
            return self.top
        raise self.unexpected(token, state)

    # BNFRepetitionModifier. e.g. {A,B}
    def enter_new_repetition_modifier(self, token, state):
        self.push(BNFParserState.REPETITION_MODIFIER_INITIAL, BNFRepetitionModifier(), self.multiplier_target)
        self.annotation_target = None
        return self.top

    def exit_repetition_modifier(self, token, state):
        if isinstance(state.node, BNFRepetitionModifier):
            self.pop()
            state.node_owner.multiplier.add(state.node)
            self.multiplier_target = None
            self.annotation_target = state.node_owner.multiplier
            return self.top
        raise self.unexpected(token, state)

    # BNFReferenceNode.StringAttribute. e.g. allows-quirks or excludes=auto,none
    def enter_new_string_attribute(self, token, state):
        self.push(
            BNFParserState.REFERENCE_STRING_ATTRIBUTE_INITIAL,
            BNFReferenceNode.StringAttribute(token.value),
            self.top.node,
        )
        self.multiplier_target = None
        self.annotation_target = None
        return self.top

    def exit_string_attribute(self, token, state):
        if isinstance(state.node, BNFReferenceNode.StringAttribute):
            self.pop()
            state.node_owner.add_attribute(state.node)
            self.multiplier_target = None
            self.annotation_target = None  # FIXME: Consider adding support for annotations to attributes.
            return self.top
        raise self.unexpected(token, state)

    # BNFReferenceNode.RangeAttribute. e.g. [0,inf]
    def enter_new_range_attribute(self, token, state):
        self.multiplier_target = None
        self.annotation_target = None
        self.push(BNFParserState.REFERENCE_RANGE_ATTRIBUTE_INITIAL, BNFReferenceNode.RangeAttribute(), self.top.node)
        return self.top

    def exit_range_attribute(self, token, state):
        if isinstance(state.node, BNFReferenceNode.RangeAttribute):
            self.pop()
            state.node_owner.add_attribute(state.node)
            self.multiplier_target = None
            self.annotation_target = None  # FIXME: Consider adding support for annotations to attributes.
            return self.top
        raise self.unexpected(token, state)

    # BNFLiteralNode. e.g. '['
    def enter_new_quoted_literal(self, token, state):
        self.push(BNFParserState.QUOTED_LITERAL_INITIAL, BNFLiteralNode(), self.top.node)
        self.multiplier_target = None
        self.annotation_target = None
        return self.top

    def exit_quoted_literal(self, token, state):
        if isinstance(state.node, BNFLiteralNode):
            self.pop()
            state.node_owner.add(state.node)
            self.multiplier_target = state.node
            self.annotation_target = state.node
            return self.top
        raise self.unexpected(token, state)

    # BNFAnnotation. e.g. @(foo=bar,baz bat)
    def enter_new_annotation(self, token, state):
        self.push(BNFParserState.ANNOTATION_INITIAL, BNFAnnotation(), self.annotation_target)
        self.annotation_target = None
        # NOTE: self.multiplier_target is not cleared here, as you may have `<foo>@(bar){2}` where the {2} associates to <foo> not @(bar).
        return self.top

    def exit_annotation(self, token, state):
        if isinstance(state.node, BNFAnnotation):
            self.pop()
            state.node_owner.add_annotation(state.node)
            self.annotation_target = None
            return self.top
        raise self.unexpected(token, state)

    # BNFAnnotation.Directive. e.g. no-single-item-opt or settings-flag=cssFooEnabled
    def enter_new_directive(self, token, state):
        self.push(BNFParserState.ANNOTATION_SEEN_ID, BNFAnnotation.Directive(token.value), self.top.node)
        return self.top

    def exit_directive(self, token, state):
        if isinstance(state.node, BNFAnnotation.Directive):
            self.pop()
            state.node_owner.add_directive(state.node)
            return self.top
        raise self.unexpected(token, state)

    def process_keyword(self, token, state):
        keyword = BNFKeywordNode(token.value)
        state.node.add(keyword)
        self.multiplier_target = keyword
        self.annotation_target = keyword

    def process_unquoted_literal(self, token, state):
        literal = BNFLiteralNode(token.value)
        state.node.add(literal)
        self.multiplier_target = literal
        self.annotation_target = literal

    def process_simple_multiplier(self, token, state):
        self.multiplier_target.multiplier.add(token.value) # type: ignore
        self.annotation_target = self.multiplier_target.multiplier # type: ignore

    def process_combinator(self, token, state, known_kind):
        if known_kind and known_kind != BNFParser.COMBINATOR_FOR_TOKEN[token.name]:
            raise Exception(f"Unexpected token '{token}'. Did you mean '{state.node.kind.name}'?.")

        state.node.kind = BNFParser.COMBINATOR_FOR_TOKEN[token.name]
        self.multiplier_target = None
        self.annotation_target = None

    # MARK: Parsing Thunks.

    def parse_UNKNOWN_GROUPING_INITIAL(self, token, state):
        if token.name == BNFToken.LSQUARE:
            self.transition_top(to=BNFParserState.UNKNOWN_GROUPING_SEEN_TERM)
            self.enter_new_grouping(token, state)
            return

        if token.name == BNFToken.LTLT:
            self.transition_top(to=BNFParserState.UNKNOWN_GROUPING_SEEN_TERM)
            self.enter_new_internal_reference(token, state)
            return

        if token.name == BNFToken.LT:
            self.transition_top(to=BNFParserState.UNKNOWN_GROUPING_SEEN_TERM)
            self.enter_new_reference(token, state)
            return

        if token.name == BNFToken.FUNC:
            self.transition_top(to=BNFParserState.UNKNOWN_GROUPING_SEEN_TERM)
            self.enter_new_function(token, state)
            return

        if token.name == BNFToken.ID:
            self.transition_top(to=BNFParserState.UNKNOWN_GROUPING_SEEN_TERM)
            self.process_keyword(token, state)
            return

        if token.name == BNFToken.SQUOTE:
            self.transition_top(to=BNFParserState.UNKNOWN_GROUPING_SEEN_TERM)
            self.enter_new_quoted_literal(token, state)
            return

        if token.name in BNFParser.SUPPORTED_UNQUOTED_LITERALS:
            self.transition_top(to=BNFParserState.UNKNOWN_GROUPING_SEEN_TERM)
            self.process_unquoted_literal(token, state)
            return

        if token.name == BNFToken.RPAREN:
            self.transition_top(to=BNFParserState.UNKNOWN_GROUPING_SEEN_TERM)
            self.exit_function(token, state)
            return

        raise self.unexpected(token, state)

    def parse_UNKNOWN_GROUPING_SEEN_TERM(self, token, state):
        if token.name == BNFToken.RSQUARE:
            self.exit_grouping(token, state)
            return

        if token.name == BNF_EOF_TOKEN:
            self.exit_initial_grouping(token, state)
            return

        if token.name == BNFToken.RPAREN:
            self.exit_function(token, state)
            return

        if token.name == BNFToken.LSQUARE:
            self.transition_top(to=BNFParserState.KNOWN_ORDERED_GROUPING)
            self.enter_new_grouping(token, state)
            return

        if token.name == BNFToken.LTLT:
            self.transition_top(to=BNFParserState.KNOWN_ORDERED_GROUPING)
            self.enter_new_internal_reference(token, state)
            return

        if token.name == BNFToken.LT:
            self.transition_top(to=BNFParserState.KNOWN_ORDERED_GROUPING)
            self.enter_new_reference(token, state)
            return

        if token.name == BNFToken.FUNC:
            self.transition_top(to=BNFParserState.KNOWN_ORDERED_GROUPING)
            self.enter_new_function(token, state)
            return

        if token.name == BNFToken.ID:
            self.transition_top(to=BNFParserState.KNOWN_ORDERED_GROUPING)
            self.process_keyword(token, state)
            return

        if token.name == BNFToken.SQUOTE:
            self.transition_top(to=BNFParserState.KNOWN_ORDERED_GROUPING)
            self.enter_new_quoted_literal(token, state)
            return

        if token.name in BNFParser.SUPPORTED_UNQUOTED_LITERALS:
            self.transition_top(to=BNFParserState.KNOWN_ORDERED_GROUPING)
            self.process_unquoted_literal(token, state)
            return

        if token.name in BNFParser.COMBINATOR_FOR_TOKEN:
            self.transition_top(to=BNFParserState.KNOWN_COMBINATOR_GROUPING_TERM_REQUIRED)
            self.process_combinator(token, state, None)
            return

        if token.name in BNFParser.SIMPLE_MULTIPLIERS:
            self.process_simple_multiplier(token, state)
            return

        if token.name == BNFToken.ATPAREN:
            self.enter_new_annotation(token, state)
            return

        if token.name == BNFToken.LBRACE:
            self.enter_new_repetition_modifier(token, state)
            return

        raise self.unexpected(token, state)

    def parse_KNOWN_ORDERED_GROUPING(self, token, state):
        if token.name == BNFToken.RSQUARE:
            self.exit_grouping(token, state)
            return

        if token.name == BNF_EOF_TOKEN:
            self.exit_initial_grouping(token, state)
            return

        if token.name == BNFToken.RPAREN:
            self.exit_function(token, state)
            return

        if token.name == BNFToken.LSQUARE:
            self.enter_new_grouping(token, state)
            return

        if token.name == BNFToken.LTLT:
            self.enter_new_internal_reference(token, state)
            return

        if token.name == BNFToken.LT:
            self.enter_new_reference(token, state)
            return

        if token.name == BNFToken.ID:
            self.process_keyword(token, state)
            return

        if token.name == BNFToken.SQUOTE:
            self.enter_new_quoted_literal(token, state)
            return

        if token.name in BNFParser.SUPPORTED_UNQUOTED_LITERALS:
            self.process_unquoted_literal(token, state)
            return

        if token.name == BNFToken.FUNC:
            self.enter_new_function(token, state)
            return

        if token.name == BNFToken.LBRACE:
            self.enter_new_repetition_modifier(token, state)
            return

        if token.name in BNFParser.SIMPLE_MULTIPLIERS:
            self.process_simple_multiplier(token, state)
            return

        if token.name == BNFToken.ATPAREN:
            self.enter_new_annotation(token, state)
            return

        raise self.unexpected(token, state)

    def parse_KNOWN_COMBINATOR_GROUPING_TERM_REQUIRED(self, token, state):
        if token.name == BNFToken.LSQUARE:
            self.transition_top(to=BNFParserState.KNOWN_COMBINATOR_GROUPING_COMBINATOR_OR_CLOSE_REQUIRED)
            self.enter_new_grouping(token, state)
            return

        if token.name == BNFToken.LTLT:
            self.transition_top(to=BNFParserState.KNOWN_COMBINATOR_GROUPING_COMBINATOR_OR_CLOSE_REQUIRED)
            self.enter_new_internal_reference(token, state)
            return

        if token.name == BNFToken.LT:
            self.transition_top(to=BNFParserState.KNOWN_COMBINATOR_GROUPING_COMBINATOR_OR_CLOSE_REQUIRED)
            self.enter_new_reference(token, state)
            return

        if token.name == BNFToken.FUNC:
            self.transition_top(to=BNFParserState.KNOWN_COMBINATOR_GROUPING_COMBINATOR_OR_CLOSE_REQUIRED)
            self.enter_new_function(token, state)
            return

        if token.name == BNFToken.ID:
            self.transition_top(to=BNFParserState.KNOWN_COMBINATOR_GROUPING_COMBINATOR_OR_CLOSE_REQUIRED)
            self.process_keyword(token, state)
            return

        # FIXME: Does it make any sense to support literals here? e.g. [ <foo> && , ]

        if token.name == BNFToken.RSQUARE or token.name == BNFToken.FUNC or token.name == BNF_EOF_TOKEN:
            raise Exception(f"Unexpected token '{token}'. Groupings can't end in a combinator.")
        raise self.unexpected(token, state)

    def parse_KNOWN_COMBINATOR_GROUPING_COMBINATOR_OR_CLOSE_REQUIRED(self, token, state):
        if token.name == BNFToken.RSQUARE:
            self.exit_grouping(token, state)
            return

        if token.name == BNF_EOF_TOKEN:
            self.exit_initial_grouping(token, state)
            return

        if token.name == BNFToken.RPAREN:
            self.exit_function(token, state)
            return

        if token.name in BNFParser.COMBINATOR_FOR_TOKEN:
            self.transition_top(to=BNFParserState.KNOWN_COMBINATOR_GROUPING_TERM_REQUIRED)
            self.process_combinator(token, state, state.node.kind)
            return

        if token.name in BNFParser.SIMPLE_MULTIPLIERS:
            self.process_simple_multiplier(token, state)
            return

        if token.name == BNFToken.ATPAREN:
            self.enter_new_annotation(token, state)
            return

        if token.name == BNFToken.LBRACE:
            self.enter_new_repetition_modifier(token, state)
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_INITIAL(self, token, state):
        if token.name == BNFToken.FUNC:
            self.transition_top(to=BNFParserState.REFERENCE_SEEN_FUNCTION_OPEN)
            state.node.is_function_reference = True
            state.node.name = token.value[:-1]
            return

        if token.name == BNFToken.ID:
            self.transition_top(to=BNFParserState.REFERENCE_SEEN_ID_OR_FUNCTION)
            state.node.name = token.value
            return

        if token.name == BNFToken.SQUOTE:
            self.transition_top(to=BNFParserState.REFERENCE_SEEN_QUOTE_OPEN)
            return

        if token.name == BNFToken.ID:
            self.transition_top(to=BNFParserState.REFERENCE_SEEN_ID_OR_FUNCTION)
            state.node.name = token.value
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_SEEN_QUOTE_OPEN(self, token, state):
        if token.name == BNFToken.ID:
            self.transition_top(to=BNFParserState.REFERENCE_SEEN_QUOTE_AND_ID)
            state.node.name = "'" + token.value + "'"
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_SEEN_QUOTE_AND_ID(self, token, state):
        if token.name == BNFToken.SQUOTE:
            self.transition_top(to=BNFParserState.REFERENCE_SEEN_ID_OR_FUNCTION)
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_SEEN_FUNCTION_OPEN(self, token, state):
        if token.name == BNFToken.RPAREN:
            self.transition_top(to=BNFParserState.REFERENCE_SEEN_ID_OR_FUNCTION)
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_SEEN_ID_OR_FUNCTION(self, token, state):
        if token.name == BNFToken.ID:
            self.enter_new_string_attribute(token, state)
            return

        if token.name == BNFToken.LSQUARE:
            self.enter_new_range_attribute(token, state)
            return

        if token.name == BNFToken.GT:
            self.exit_reference(token, state)
            return

        raise self.unexpected(token, state)

    def parse_INTERNAL_REFERENCE_INITIAL(self, token, state):
        if token.name == BNFToken.ID:
            self.transition_top(to=BNFParserState.INTERNAL_REFERENCE_SEEN_ID)
            state.node.name = token.value
            return

        raise self.unexpected(token, state)

    def parse_INTERNAL_REFERENCE_SEEN_ID(self, token, state):
        if token.name == BNFToken.ID:
            self.enter_new_string_attribute(token, state)
            return

        if token.name == BNFToken.LSQUARE:
            self.enter_new_range_attribute(token, state)
            return

        if token.name == BNFToken.GTGT:
            self.exit_internal_reference(token, state)
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_STRING_ATTRIBUTE_INITIAL(self, token, state):
        if token.name == BNFToken.EQUAL:
            self.transition_top(to=BNFParserState.REFERENCE_STRING_ATTRIBUTE_SEEN_EQUAL)
            return

        state = self.exit_string_attribute(token, state)

        if token.name == BNFToken.ID:
            self.enter_new_string_attribute(token, state)
            return

        if token.name == BNFToken.LSQUARE:
            self.enter_new_range_attribute(token, state)
            return

        if token.name == BNFToken.GT:
            self.exit_reference(token, state)
            return

        if token.name == BNFToken.GTGT:
            self.exit_internal_reference(token, state)
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_STRING_ATTRIBUTE_SEEN_EQUAL(self, token, state):
        if token.name == BNFToken.ID:
            self.transition_top(to=BNFParserState.REFERENCE_STRING_ATTRIBUTE_SEEN_VALUE)
            state.node.value.append(token.value)
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_STRING_ATTRIBUTE_SEEN_VALUE(self, token, state):
        if token.name == BNFToken.COMMA:
            self.transition_top(to=BNFParserState.REFERENCE_STRING_ATTRIBUTE_SEEN_EQUAL)
            return

        state = self.exit_string_attribute(token, state)

        if token.name == BNFToken.ID:
            self.enter_new_string_attribute(token, state)
            return

        if token.name == BNFToken.LSQUARE:
            self.enter_new_range_attribute(token, state)
            return

        if token.name == BNFToken.GT:
            self.exit_reference(token, state)
            return

        if token.name == BNFToken.GTGT:
            self.exit_internal_reference(token, state)
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_RANGE_ATTRIBUTE_INITIAL(self, token, state):
        if (
            token.name == BNFToken.INT
            or token.name == BNFToken.FLOAT
            or (token.name == BNFToken.ID and token.value == "-inf")
        ):
            self.transition_top(to=BNFParserState.REFERENCE_RANGE_ATTRIBUTE_SEEN_MIN)
            state.node.min = token.value
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_RANGE_ATTRIBUTE_SEEN_MIN(self, token, state):
        if token.name == BNFToken.COMMA:
            self.transition_top(to=BNFParserState.REFERENCE_RANGE_ATTRIBUTE_SEEN_MIN_AND_COMMA)
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_RANGE_ATTRIBUTE_SEEN_MIN_AND_COMMA(self, token, state):
        if (
            token.name == BNFToken.INT
            or token.name == BNFToken.FLOAT
            or (token.name == BNFToken.ID and token.value == "inf")
        ):
            self.transition_top(to=BNFParserState.REFERENCE_RANGE_ATTRIBUTE_SEEN_MAX)
            state.node.max = token.value
            return

        raise self.unexpected(token, state)

    def parse_REFERENCE_RANGE_ATTRIBUTE_SEEN_MAX(self, token, state):
        if token.name == BNFToken.RSQUARE:
            self.exit_range_attribute(token, state)
            return

        raise self.unexpected(token, state)

    def parse_REPETITION_MODIFIER_INITIAL(self, token, state):
        if token.name == BNFToken.INT:
            self.transition_top(to=BNFParserState.REPETITION_MODIFIER_SEEN_MIN)
            state.node.kind = BNFRepetitionModifier.Kind.EXACT
            state.node.min = int(token.value)
            return

        raise self.unexpected(token, state)

    def parse_REPETITION_MODIFIER_SEEN_MIN(self, token, state):
        if token.name == BNFToken.COMMA:
            self.transition_top(to=BNFParserState.REPETITION_MODIFIER_SEEN_MIN_AND_COMMA)
            state.node.kind = BNFRepetitionModifier.Kind.AT_LEAST
            return

        if token.name == BNFToken.RBRACE:
            self.exit_repetition_modifier(token, state)
            return

        raise self.unexpected(token, state)

    def parse_REPETITION_MODIFIER_SEEN_MIN_AND_COMMA(self, token, state):
        if token.name == BNFToken.INT:
            self.transition_top(to=BNFParserState.REPETITION_MODIFIER_SEEN_MAX)
            state.node.kind = BNFRepetitionModifier.Kind.BETWEEN
            state.node.max = int(token.value)
            return

        if token.name == BNFToken.RBRACE:
            self.exit_repetition_modifier(token, state)
            return

        raise self.unexpected(token, state)

    def parse_REPETITION_MODIFIER_SEEN_MAX(self, token, state):
        if token.name == BNFToken.RBRACE:
            self.exit_repetition_modifier(token, state)
            return

        raise self.unexpected(token, state)

    def parse_QUOTED_LITERAL_INITIAL(self, token, state):
        # Take the value regardless of token name.
        self.transition_top(to=BNFParserState.QUOTED_LITERAL_SEEN_ID)
        state.node.value = token.value

    def parse_QUOTED_LITERAL_SEEN_ID(self, token, state):
        if token.name == BNFToken.SQUOTE:
            self.exit_quoted_literal(token, state)
            return

        # Append the value regardless of the token value.
        state.node.value = state.node.value + token.value

    def parse_ANNOTATION_INITIAL(self, token, state):
        if token.name == BNFToken.ID:
            self.enter_new_directive(token, state)
            return

        raise self.unexpected(token, state)

    def parse_ANNOTATION_SEEN_ID(self, token, state):
        if token.name == BNFToken.EQUAL:
            self.transition_top(to=BNFParserState.ANNOTATION_SEEN_EQUAL_OR_COMMA)
            return

        state = self.exit_directive(token, state)

        if token.name == BNFToken.ID:
            self.enter_new_directive(token, state)
            return

        if token.name == BNFToken.RPAREN:
            self.exit_annotation(token, state)
            return

        raise self.unexpected(token, state)

    def parse_ANNOTATION_SEEN_EQUAL_OR_COMMA(self, token, state):
        if token.name == BNFToken.ID:
            state.node.value.append(token.value)
            self.transition_top(to=BNFParserState.ANNOTATION_SEEN_VALUE)
            return

        raise self.unexpected(token, state)

    def parse_ANNOTATION_SEEN_VALUE(self, token, state):
        if token.name == BNFToken.COMMA:
            self.transition_top(to=BNFParserState.ANNOTATION_SEEN_EQUAL_OR_COMMA)
            return

        state = self.exit_directive(token, state)

        if token.name == BNFToken.ID:
            self.enter_new_directive(token, state)
            return

        if token.name == BNFToken.RPAREN:
            self.exit_annotation(token, state)
            return

        raise self.unexpected(token, state)
