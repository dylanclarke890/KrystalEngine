import pathlib
import textwrap
import io
from typing import Any
from utils import EnumValue, GeneratorContext, get_validated_json, kebab_case_to_pascal_case, generator_metadata
from collections import defaultdict
from bnf import BNFParser, BNFGroupingNode, BNFFunctionNode, BNFReferenceNode, BNFKeywordNode, BNFLiteralNode

GENERATOR_NAME = "css-properties.generator"

#region TermGenerator Classes (Code Emission)

class TermGenerator:
    """Base class for term generators. Each term type has a generator."""

    @staticmethod
    def make(term: Term, keyword_fast_path_generator = None) -> "TermGenerator":
        """Factory method to create appropriate generator for a term."""
        if isinstance(term, GroupingTerm):
            return TermGeneratorGroupingTerm(term)
        elif isinstance(term, FunctionTerm):
            return TermGeneratorFunctionTerm(term)
        elif isinstance(term, ReferenceTerm):
            return TermGeneratorReferenceTerm(term)
        elif isinstance(term, KeywordTerm):
            return TermGeneratorKeywordTerm(term)
        elif isinstance(term, LiteralTerm):
            return TermGeneratorLiteralTerm(term)
        else:
            raise Exception(f"Unknown term type: {type(term)}")

    def generate_conditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        """Generate conditional code (if statement)."""
        raise NotImplementedError

    def generate_unconditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        """Generate unconditional code (direct return)."""
        raise NotImplementedError

    def has_unconditional_return(self) -> bool:
        """Check if generate_unconditional produces an unconditional return statement."""
        # By default, assume generators produce conditional code
        return False


class TermGeneratorReferenceTerm(TermGenerator):
    """Generate code for reference terms like <color>."""

    def __init__(self, term: ReferenceTerm, all_properties: dict[str, Any]|None = None):
        self.term = term
        self.all_properties = all_properties or {}

    def generate_conditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        consumer_call = self._generate_consumer_call(range_string, state_string)
        to.write(f"if (auto result = {consumer_call})")
        with to.indent():
            to.write("return result;")

    def generate_unconditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        consumer_call = self._generate_consumer_call(range_string, state_string)
        to.write(f"return {consumer_call};")

    def has_unconditional_return(self) -> bool:
        """Reference terms always produce an unconditional return."""
        return True

    def _generate_consumer_call(self, range_string: str, state_string: str) -> str:
        """Generate the appropriate consumer call based on reference type."""
        # Handle builtin types (like <length-percentage>)
        if self.term.is_builtin:
            return self.term.builtin.get_consumer_call(range_string, state_string) # type: ignore

        # Handle property references with quotes (like <'margin-top'>)
        # These have quotes in the name itself
        if self.term.name.startswith("'") and self.term.name.endswith("'"):
            # Internal references like <'margin-top'> should call the property's parser
            # Strip quotes from the name
            property_name = self.term.name.strip("'")
            pascal_name = kebab_case_to_pascal_case(property_name)
            return f"ParseCSS{pascal_name}Value({range_string}, {state_string})"

        # Handle internal references (like <<values>>)
        if self.term.is_internal:
            # These reference internal grammar rules
            formatted_name = kebab_case_to_pascal_case(self.term.name)
            return f"Consume{formatted_name}({range_string}, {state_string})"

        # Handle regular references (non-builtin, non-internal)
        # These reference grammar rules or other generated parsers
        formatted_name = kebab_case_to_pascal_case(self.term.name)
        return f"Consume{formatted_name}({range_string}, {state_string})"

    def _format_name(self) -> str:
        """Format the reference name for use in C++."""
        return kebab_case_to_pascal_case(self.term.name)


class TermGeneratorKeywordTerm(TermGenerator):
    """Generate code for keyword terms like 'auto', 'none'."""

    def __init__(self, term: KeywordTerm):
        self.term = term

    def generate_conditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        # Use ConsumeIdent template for single keywords
        keyword_id = kebab_case_to_pascal_case(self.term.value)
        to.write(f"// {self.term.value}")
        to.write(f"if (auto result = ConsumeIdent<CSSValueId::{keyword_id}>({range_string}))")
        with to.indent():
            to.write("return result;")

    def generate_unconditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        keyword_id = kebab_case_to_pascal_case(self.term.value)
        to.write(f"// {self.term.value}")
        to.write(f"return ConsumeIdent<CSSValueId::{keyword_id}>({range_string});")

    def has_unconditional_return(self) -> bool:
        """Keyword terms always produce an unconditional return."""
        return True


class TermGeneratorLiteralTerm(TermGenerator):
    """Generate code for literal terms like '/'."""
    
    def __init__(self, term: LiteralTerm):
        self.term = term

    def generate_conditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        to.write(f"if (ConsumeLiteral<'{self.term.value}'>({range_string}))")
        with to.indent():
            to.write("// Matched literal")

    def generate_unconditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        to.write(f"ConsumeLiteral<'{self.term.value}'>({range_string});")

    def has_unconditional_return(self) -> bool:
        """Literal terms don't produce unconditional returns."""
        return False


class TermGeneratorFunctionTerm(TermGenerator):
    """Generate code for function terms."""
    
    def __init__(self, term: FunctionTerm):
        self.term = term

    def generate_conditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        func_name = kebab_case_to_pascal_case(self.term.name)
        to.write(f"if (auto result = Consume{func_name}({range_string}, {state_string}))")
        with to.indent():
            to.write("return result;")

    def generate_unconditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        func_name = kebab_case_to_pascal_case(self.term.name)
        to.write(f"return Consume{func_name}({range_string}, {state_string});")

    def has_unconditional_return(self) -> bool:
        """Function terms always produce an unconditional return."""
        return True


class TermGeneratorGroupingTerm(TermGenerator):
    """Generate code for grouping terms."""

    def __init__(self, term: GroupingTerm):
        self.term = term
        self.subgenerators = [TermGenerator.make(subterm) for subterm in term.subterms]

    def _is_all_keywords(self) -> bool:
        """Check if all subterms are keywords."""
        return all(isinstance(subterm, KeywordTerm) for subterm in self.term.subterms)

    def _get_keywords(self) -> list[str]:
        """Get list of keyword values from subterms."""
        return [subterm.value for subterm in self.term.subterms if isinstance(subterm, KeywordTerm)]

    def generate_conditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        if self.term.kind == '|':
            # Match one - optimize if all keywords
            if self._is_all_keywords():
                keywords = self._get_keywords()
                keyword_ids = ', '.join(f'CSSValueId::{kebab_case_to_pascal_case(kw)}' for kw in keywords)
                to.write(f"// {' | '.join(keywords)}")
                to.write(f"if (auto result = ConsumeIdent<{keyword_ids}>({range_string}))")
                with to.indent():
                    to.write("return result;")
            else:
                # Mixed terms - generate individual conditionals
                for gen in self.subgenerators:
                    gen.generate_conditional(to=to, range_string=range_string, state_string=state_string)
        elif self.term.kind == ' ':
            # Match all ordered
            to.write(f"// Match all ordered")
            for gen in self.subgenerators:
                gen.generate_unconditional(to=to, range_string=range_string, state_string=state_string)

    def generate_unconditional(self, *, to: Writer, range_string: str, state_string: str) -> None:
        if self.term.kind == '|':
            if self._is_all_keywords():
                keywords = self._get_keywords()
                keyword_ids = ', '.join(f'CSSValueId::{kebab_case_to_pascal_case(kw)}' for kw in keywords)
                to.write(f"// {' | '.join(keywords)}")
                to.write(f"return ConsumeIdent<{keyword_ids}>({range_string});")
            else:
                for gen in self.subgenerators:
                    gen.generate_conditional(to=to, range_string=range_string, state_string=state_string)
        elif self.term.kind == ' ':
            for gen in self.subgenerators:
                gen.generate_unconditional(to=to, range_string=range_string, state_string=state_string)

    def has_unconditional_return(self) -> bool:
        """Check if this grouping produces an unconditional return."""
        if self.term.kind == '|':
            # Match one: only if all keywords (optimized path)
            return self._is_all_keywords()
        elif self.term.kind == ' ':
            # Match all ordered: only if ALL subterms have unconditional returns
            return all(gen.has_unconditional_return() for gen in self.subgenerators)
        return False


class KeywordFastPathGenerator:
    """Generates optimized fast-path for keyword-only properties."""
    
    def __init__(self, function_name: str, keywords: list[str]):
        self.function_name = function_name
        self.keywords = keywords

    def generate_definition(self, *, to: Writer) -> None:
        """Generate the keyword validation function."""
        to.write(f"KRYS_NODISCARD inline bool {self.function_name}(CSSValueId keyword) noexcept")
        to.write("{")
        with to.indent():
            to.write("switch (keyword) {")
            with to.indent():
                for keyword in self.keywords:
                    keyword_id = kebab_case_to_pascal_case(keyword)
                    to.write(f"case CSSValueId::{keyword_id}:")
                to.write("    return true;")
            to.write("default:")
            with to.indent():
                to.write("return false;")
            to.write("}")
        to.write("}")

#endregion

#region Code Generation Infrastructure (from WebKit)

class Writer:
    """Utility for writing formatted C++ code with proper indentation."""
    TAB_SIZE = 4

    def __init__(self, output: io.StringIO | None = None):
        self.output = output or io.StringIO()
        self._indentation_level = 0

    @property
    def _current_indent(self) -> str:
        return (self._indentation_level * Writer.TAB_SIZE) * ' '

    def write(self, text: str) -> "Writer":
        """Write text with current indentation and newline."""
        self.output.write(self._current_indent)
        self.output.write(text)
        self.output.write('\n')
        return self

    def write_block(self, text: str) -> "Writer":
        """Write a block of text with dedent and indentation."""
        self.output.write(textwrap.indent(textwrap.dedent(text), self._current_indent))
        self.output.write('\n')
        return self

    def write_lines(self, iterable) -> "Writer":
        """Write multiple lines."""
        for line in iterable:
            self.write(line)
        return self

    class Indent:
        """Context manager for indentation."""
        def __init__(self, writer: "Writer"):
            self.writer = writer

        def __enter__(self):
            self.writer._indentation_level += 1
            return self

        def __exit__(self, exc_type, exc_value, traceback):
            self.writer._indentation_level -= 1

    def indent(self) -> Indent:
        """Return an indent context manager."""
        return Writer.Indent(self)

    def get_content(self) -> str:
        """Get the written content."""
        return self.output.getvalue()

#endregion

#region Grammar/Parsing Term Classes (from WebKit)

class Term:
    """Base class for all grammar terms."""
    
    @staticmethod
    def from_node(node) -> "Term":
        """Create a Term from a BNF node."""
        if isinstance(node, BNFGroupingNode):
            return GroupingTerm.from_node(node)
        elif isinstance(node, BNFFunctionNode):
            return FunctionTerm.from_node(node)
        elif isinstance(node, BNFReferenceNode):
            return ReferenceTerm.from_node(node)
        elif isinstance(node, BNFKeywordNode):
            return KeywordTerm.from_node(node)
        elif isinstance(node, BNFLiteralNode):
            return LiteralTerm.from_node(node)
        else:
            raise Exception(f"Unknown BNF node type: {type(node)}")


# Builtin type definitions and mappings
class BuiltinType:
    """Base class for builtin CSS types that map to specific C++ consumers."""
    def __init__(self, name: str, parameters: dict[str, Any]):
        self.name = name
        self.parameters = parameters
        self.value_range = self._build_range()

    def _build_range(self) -> str:
        """Build the CSS::Range template parameter from parsed range."""
        range_param = self.parameters.get('range')
        if not range_param:
            return ""  # Default range

        min_val, max_val = range_param

        # Use predefined constants when possible
        # Convert to comparable form (handle both numbers and strings)
        min_cmp = float(min_val) if isinstance(min_val, (int, float)) or (isinstance(min_val, str) and min_val not in ['-inf', 'inf']) else min_val
        max_cmp = max_val

        if (min_cmp == 0 or min_cmp == 0.0) and max_val == 'inf':
            return "NonNegative"
        elif min_val == '-inf' and max_val == 'inf':
            return "All"

        # Otherwise, build custom range
        min_str = '-CSSRange::Inf' if min_val == '-inf' else str(min_val)
        max_str = 'CSSRange::Inf' if max_val == 'inf' else str(max_val)

        return f"CSSRange{{{min_str}, {max_str}}}"

    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        """Generate the C++ consumer call for this builtin type."""
        raise NotImplementedError()


class BuiltinLengthPercentage(BuiltinType):
    """Builtin for <length-percentage> with optional range."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        range_template = f"<{self.value_range}>" if self.value_range else "<>"
        type_param = f"LengthPercentage{range_template}"
        return f"CSSPrimitiveValueResolver<{type_param}>::ConsumeAndResolve({range_string}, {state_string})"


class BuiltinLength(BuiltinType):
    """Builtin for <length> with optional range."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        range_template = f"<{self.value_range}>" if self.value_range else "<>"
        type_param = f"Length{range_template}"
        return f"CSSPrimitiveValueResolver<{type_param}>::ConsumeAndResolve({range_string}, {state_string})"


class BuiltinPercentage(BuiltinType):
    """Builtin for <percentage> with optional range."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        range_template = f"<{self.value_range}>" if self.value_range else "<>"
        type_param = f"Percentage{range_template}"
        return f"CSSPrimitiveValueResolver<{type_param}>::ConsumeAndResolve({range_string}, {state_string})"


class BuiltinNumber(BuiltinType):
    """Builtin for <number> with optional range."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        range_template = f"<{self.value_range}>" if self.value_range else "<>"
        type_param = f"Number{range_template}"
        return f"CSSPrimitiveValueResolver<{type_param}>::ConsumeAndResolve({range_string}, {state_string})"


class BuiltinInteger(BuiltinType):
    """Builtin for <integer> with optional range."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        range_template = f"<{self.value_range}>" if self.value_range else "<>"
        type_param = f"Integer{range_template}"
        return f"CSSPrimitiveValueResolver<{type_param}>::ConsumeAndResolve({range_string}, {state_string})"


class BuiltinAngle(BuiltinType):
    """Builtin for <angle> with optional range."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        range_template = f"<{self.value_range}>" if self.value_range else "<>"
        type_param = f"Angle{range_template}"
        return f"CSSPrimitiveValueResolver<{type_param}>::ConsumeAndResolve({range_string}, {state_string})"


class BuiltinTime(BuiltinType):
    """Builtin for <time> with optional range."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        range_template = f"<{self.value_range}>" if self.value_range else "<>"
        type_param = f"Time{range_template}"
        return f"CSSPrimitiveValueResolver<{type_param}>::ConsumeAndResolve({range_string}, {state_string})"


class BuiltinResolution(BuiltinType):
    """Builtin for <resolution> with optional range."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        range_template = f"<{self.value_range}>" if self.value_range else "<>"
        type_param = f"Resolution{range_template}"
        return f"CSSPrimitiveValueResolver<{type_param}>::ConsumeAndResolve({range_string}, {state_string})"


class BuiltinColor(BuiltinType):
    """Builtin for <color>."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        return f"ConsumeColor({range_string}, {state_string})"


class BuiltinImage(BuiltinType):
    """Builtin for <image>."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        return f"ConsumeImage({range_string}, {state_string})"


class BuiltinUrl(BuiltinType):
    """Builtin for <url>."""
    def get_consumer_call(self, range_string: str, state_string: str) -> str:
        return f"ConsumeUrl({range_string}, {state_string})"

#endregion

# Map of builtin type names to their handler classes
BUILTIN_TYPES = {
    'length-percentage': BuiltinLengthPercentage,
    'length': BuiltinLength,
    'percentage': BuiltinPercentage,
    'number': BuiltinNumber,
    'integer': BuiltinInteger,
    'angle': BuiltinAngle,
    'time': BuiltinTime,
    'resolution': BuiltinResolution,
    'color': BuiltinColor,
    'image': BuiltinImage,
    'url': BuiltinUrl,
}


class ReferenceTerm(Term):
    """Represents a reference to another rule or builtin (e.g., <color>, <length>)."""

    def __init__(self, name: str, is_internal: bool = False, is_function_reference: bool = False, 
                 parameters: list[Any]|None = None, builtin: BuiltinType|None = None):
        self.name = name
        self.is_internal = is_internal
        self.is_function_reference = is_function_reference
        self.parameters = parameters or []
        self.builtin = builtin

        # Check if this is a builtin type
        if not is_internal and name in BUILTIN_TYPES:
            # Parse parameters for builtins (e.g., range specifications)
            param_dict = {}
            for param in self.parameters:
                if hasattr(param, 'min') and hasattr(param, 'max'):
                    param_dict['range'] = (param.min, param.max)
            self.builtin = BUILTIN_TYPES[name](name, param_dict)

    def __str__(self) -> str:
        if self.is_internal:
            return f"<<{self.name}>>"
        return f"<{self.name}>"

    @property
    def is_builtin(self) -> bool:
        return self.builtin is not None

    @staticmethod
    def from_node(node: BNFReferenceNode) -> "ReferenceTerm":
        """Create from BNF node."""
        return ReferenceTerm(
            name=str(node.name),
            is_internal=node.is_internal,
            is_function_reference=node.is_function_reference,
            parameters=node.attributes
        )


class LiteralTerm(Term):
    """Represents a literal value (e.g., comma, slash)."""
    
    def __init__(self, value: str):
        self.value = value

    def __str__(self) -> str:
        return f"'{self.value}'"

    @staticmethod
    def from_node(node: BNFLiteralNode) -> "LiteralTerm":
        return LiteralTerm(str(node.value))


class KeywordTerm(Term):
    """Represents a CSS keyword (e.g., auto, none)."""
    
    def __init__(self, value: str):
        self.value = value

    def __str__(self) -> str:
        return self.value

    @staticmethod
    def from_node(node: BNFKeywordNode) -> "KeywordTerm":
        return KeywordTerm(node.keyword)


class GroupingTerm(Term):
    """Represents a grouping of terms with a combinator."""
    
    def __init__(self, kind: str, subterms: list[Term]):
        self.kind = kind
        self.subterms = subterms

    def __str__(self) -> str:
        if self.kind == ' ':
            separator = ' '
        else:
            separator = f' {self.kind} '
        return f"[ {separator.join(str(t) for t in self.subterms)} ]"

    @staticmethod
    def from_node(node: BNFGroupingNode) -> "GroupingTerm":
        """Create from BNF node."""
        subterms = [Term.from_node(member) for member in node.members]
        kind_map = {
            BNFGroupingNode.Kind.MATCH_ALL_ORDERED: ' ',
            BNFGroupingNode.Kind.MATCH_ONE: '|',
            BNFGroupingNode.Kind.MATCH_ALL_ANY_ORDER: '&&',
            BNFGroupingNode.Kind.MATCH_ONE_OR_MORE_ANY_ORDER: '||',
        }
        return GroupingTerm(kind_map.get(node.kind, ' '), subterms)


class FunctionTerm(Term):
    """Represents a function term (e.g., rgb())."""
    
    def __init__(self, name: str, parameter_group_term: Term):
        self.name = name
        self.parameter_group_term = parameter_group_term

    def __str__(self) -> str:
        return f"{self.name}(...)"

    @staticmethod
    def from_node(node: BNFFunctionNode) -> "FunctionTerm":
        param_term = Term.from_node(node.parameter_group)
        return FunctionTerm(node.name, param_term)


class Grammar:
    """Represents a parsed CSS grammar."""
    
    def __init__(self, name: str, root_term: Term):
        self.name = name
        self.root_term = root_term

    def __str__(self) -> str:
        return f"Grammar({self.name})"

    @staticmethod
    def from_string(parsing_context: Any, key_path: str, name: str, grammar_string: str) -> "Grammar":
        """Parse a grammar string into a Grammar object."""
        parser = BNFParser(parsing_context, key_path, grammar_string)
        root_node = parser.parse()
        return Grammar(name, Term.from_node(root_node))


# ============================================================================
# Code Generation Support Classes
# ============================================================================

class FunctionParameter:
    """Represents a function parameter."""
    
    def __init__(self, param_type: str, name: str):
        self.param_type = param_type
        self.name = name

    def __str__(self) -> str:
        return f"{self.param_type} {self.name}"


class FunctionSignature:
    """Represents a function signature."""
    
    def __init__(self, result_type: str, name: str, parameters: list[FunctionParameter], scope: str|None = None):
        self.result_type = result_type
        self.name = name
        self.parameters = parameters
        self.scope = scope

    @property
    def declaration_string(self) -> str:
        """Generate declaration string."""
        params = ", ".join(str(p) for p in self.parameters)
        if self.scope:
            return f"{self.result_type} {self.scope}::{self.name}({params})"
        return f"{self.result_type} {self.name}({params})"

    @property
    def definition_string(self) -> str:
        """Generate definition string (same as declaration for now)."""
        return self.declaration_string

    def generate_call_string(self, arguments: list[str]) -> str:
        """Generate a call to this function."""
        args = ", ".join(arguments)
        return f"{self.name}({args})"


class CSSProperty:
    id: str
    id_pascal_case: str
    initial: str | None
    inherited: bool
    parser_syntax: str | None
    longhand_names: list[str] | None
    specification_module: str
    specification_url: str
    grammar: Grammar | None

    @staticmethod
    def from_json_dict(property_name: str, property_data: dict) -> "CSSProperty":
        property = CSSProperty()
        property.id = property_name
        property.id_pascal_case = kebab_case_to_pascal_case(property_name)
        property.initial = property_data.get("initial")
        property.inherited = property_data.get("inherited", False)
        
        codegen_props = property_data.get("codegen-properties", {})
        property.parser_syntax = codegen_props.get("parser-syntax")
        property.longhand_names = codegen_props.get("longhands")
        
        spec = property_data.get("specification", {})
        property.specification_module = spec.get("module", "")
        property.specification_url = spec.get("url", "")
        
        # Parse grammar if parser-grammar is specified
        property.grammar = None
        if property.parser_syntax is not None:
            try:
                property.grammar = Grammar.from_string(
                    parsing_context=None,
                    key_path=f"properties/{property_name}/codegen-properties/parser-grammar",
                    name=property.id_pascal_case,
                    grammar_string=property.parser_syntax
                )
            except Exception as e:
                print(f"Warning: Failed to parse grammar for {property_name}: {e}")
        
        return property

    def get_enum_value(self) -> "EnumValue":
        comment = f"/// @syntax {self.parser_syntax}\n    /// @specification {self.specification_module} ({self.specification_url})"
        return EnumValue(self.id, self.id_pascal_case, comment)
    
    def has_parser(self) -> bool:
        """Check if this property should have a parser generated."""
        return bool(self.parser_syntax)
    
    def generate_parser_function(self, *, to: Writer) -> None:
        """Generate the parser function for this property."""
        if not self.grammar:
            return

        func_name = f"ParseCSS{self.id_pascal_case}Value"

        # Write function signature
        to.write(f"KRYS_NODISCARD Maybe<RefPtr<CSSValue>> {func_name}(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept")
        to.write("{")

        with to.indent():
            to.write("auto range = tokens;")
            to.write("")

            # Generate code for the grammar
            generator = TermGenerator.make(self.grammar.root_term)
            generator.generate_unconditional(to=to, range_string="range", state_string="state")

            # Only add return nullptr if the generator doesn't produce an unconditional return
            if not generator.has_unconditional_return():
                to.write("")
                to.write("return nullptr;")

        to.write("}")

def __generate_css_property_id_enum(
    context: GeneratorContext,
    all_properties: list[CSSProperty],
    shorthand_property_count: int,
):
    model: dict[str, object] = {
        "enum_comment": "/// @brief An enumeration of all CSS properties, including both shorthand and longhand properties.",
        "enum_name": "CSSPropertyId",
        "enum_values": [prop.get_enum_value() for prop in all_properties],
        "enum_type": "uint16",
        "enum_default": "Invalid",
        "enum_default_comment": "/// @brief Represents an invalid or unrecognized CSS property.",
        "additional_data": [
            "constexpr size_t FirstShorthandCSSPropertyId = 1uz;",
            f"constexpr size_t LastShorthandCSSPropertyId = {shorthand_property_count}uz;",
            f"constexpr size_t FirstLonghandCSSPropertyId = {1 + shorthand_property_count}uz;",
            f"constexpr size_t LastLonghandCSSPropertyId = {len(all_properties) - 1}uz;",
        ],
    }
    model.update(generator_metadata(GENERATOR_NAME))

    template = context.template_env.get_template("enum.hpp.jinja")
    output = template.render(model=model)

    output_filepath = (
        context.project_root / "include" / "Krystal.HTML" / "CSS" / "Properties" / "Enums" / "CSSPropertyId.hpp"
    )
    with open(output_filepath, "w") as hpp_file:
        hpp_file.write(output)

class ShorthandToLonghandData:
    shorthand_name: str
    longhand_names: list[str]

    def __init__(self, shorthand: str, longhands: list[str]):
        self.shorthand_name = shorthand
        self.longhand_names = longhands

class LonghandsToShorthandData:
    longhand_names: list[str]
    shorthand_names: list[str]

    def __init__(self, longhand: list[str], shorthands: list[str]):
        self.longhand_names = longhand
        self.shorthand_names = shorthands

def __generate_css_property_shorthand_files(
    context: GeneratorContext, shorthand_properties: list[CSSProperty], longhand_properties: list[CSSProperty]
):
    model: dict[str, object] = generator_metadata(GENERATOR_NAME)

    shorthand_to_longhand_data: dict[str, list[CSSProperty]] = {}
    longhand_to_shorthand_data: dict[str, list[CSSProperty]] = {}
    shorthand_to_longhand_count: dict[CSSProperty, int] = {}
    longhand_properties_by_name: dict[str, CSSProperty] = {prop.id: prop for prop in longhand_properties}

    for property in shorthand_properties:
        shorthand_to_longhand_count[property] = 0
        for longhand_name in property.longhand_names or []:
            if longhand_name == "all":
                for inner_property in longhand_properties:
                    if inner_property.id == "direction" or inner_property.id == "unicode-bidi":
                        continue
                    longhand_to_shorthand_data.setdefault(inner_property.id_pascal_case, []).append(property)
                    shorthand_to_longhand_count[property] += 1
            else:
                id = longhand_properties_by_name[longhand_name].id_pascal_case
                longhand_to_shorthand_data.setdefault(id, []).append(property)
                shorthand_to_longhand_count[property] += 1

        if property.id == "all":
            shorthand_to_longhand_data[property.id_pascal_case] = [
                inner_property
                for inner_property in longhand_properties
                if inner_property.id != "direction" and inner_property.id != "unicode-bidi"
            ]
        else:
            shorthand_to_longhand_data[property.id_pascal_case] = [
                longhand_properties_by_name[name] for name in property.longhand_names or []
            ]

    # https://drafts.csswg.org/cssom/#concept-shorthands-preferred-order
    def preferred_order_for_shorthands(x):
        return (-shorthand_to_longhand_count[x], x.id.startswith("-"), not x.id.startswith("-webkit-"), x.id)

    longhands_to_shorthand_data: defaultdict[tuple[CSSProperty, ...], list[str]] = defaultdict(list)
    for longhand, shorthands in longhand_to_shorthand_data.items():
        longhands_to_shorthand_data[tuple(sorted(shorthands, key=preferred_order_for_shorthands))].append(longhand)

    model.update(
        {
            "shorthand_to_longhand_data": [
                ShorthandToLonghandData(shorthand, [longhand.id_pascal_case for longhand in longhands])
                for shorthand, longhands in shorthand_to_longhand_data.items()
            ],
            "longhands_to_shorthand_data": [
                LonghandsToShorthandData(longhands, [shorthand.id_pascal_case for shorthand in shorthands])
                for shorthands, longhands in longhands_to_shorthand_data.items()
            ],
            "shorthand_to_longhand_count": shorthand_to_longhand_count,
        }
    )

    template = context.template_env.get_template("css-property-shorthand.cpp.jinja")
    output = template.render(model=model)

    output_filepath = context.project_root / "src" / "CSS" / "Properties" / "CSSPropertyShorthand.cpp"
    with open(output_filepath, "w") as cpp_file:
        cpp_file.write(output)

def __generate__css_property_consumer_file(context: GeneratorContext, all_properties: list[CSSProperty]):
    model: dict[str, object] = generator_metadata(GENERATOR_NAME)
    template = context.template_env.get_template("css-property-consumer.cpp.jinja")
    output = template.render(model=model)

    output_filepath = context.project_root / "src" / "CSS" / "Properties" / "CSSPropertyConsumer.cpp"
    with open(output_filepath, "w") as cpp_file:
        cpp_file.write(output)

def __generate_css_property_parsers_file(context: GeneratorContext, all_properties: list[CSSProperty]):
    properties_with_parsers = [prop for prop in all_properties if prop.has_parser()]

    if not properties_with_parsers:
        print("No properties with parsers found. Skipping CSSPropertyParsers generation.")
        return

    # Generate header file
    writer_hpp = Writer()
    writer_hpp.write("#pragma once")
    writer_hpp.write("")
    writer_hpp.write("#include \"Krystal.Lib/Pointers/RefPtr.hpp\"")
    writer_hpp.write("#include \"Krystal.Lib/Types/Maybe.hpp\"")
    writer_hpp.write("")
    writer_hpp.write("namespace Krys::HTML {")
    writer_hpp.write("")
    writer_hpp.write("class CSSTokenRange;")
    writer_hpp.write("struct CSSPropertyParserState;")
    writer_hpp.write("class CSSValue;")
    writer_hpp.write("")

    # Declare each parser function
    for prop in properties_with_parsers:
        func_name = f"ParseCSS{prop.id_pascal_case}Value"
        writer_hpp.write(f"KRYS_NODISCARD Maybe<RefPtr<CSSValue>> {func_name}(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;")

    writer_hpp.write("")
    writer_hpp.write("}")

    output_filepath_hpp = context.project_root / "include" / "Krystal.HTML" / "CSS" / "Properties" / "CSSPropertyParsers.hpp"
    with open(output_filepath_hpp, "w") as hpp_file:
        hpp_file.write(writer_hpp.get_content())

    # Generate source file
    writer_cpp = Writer()
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/CSSPropertyParsers.hpp\"")
    writer_cpp.write("")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Parser/CSSTokenRange.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/Consumers/Ident.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/Consumers/LengthDefinitions.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/Consumers/PercentageDefinitions.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/Consumers/IntegerDefinitions.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/Consumers/AngleDefinitions.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/Consumers/TimeDefinitions.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Properties/Consumers/ResolutionDefinitions.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Values/CSSValue.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumeric.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRange.hpp\"")
    writer_cpp.write("#include \"Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericRaw.hpp\"")
    writer_cpp.write("#include \"Krystal.Lib/Pointers/RefPtr.hpp\"")
    writer_cpp.write("#include \"Krystal.Lib/Types/Maybe.hpp\"")
    writer_cpp.write("")
    writer_cpp.write("namespace Krys::HTML {")
    writer_cpp.write("")

    # Generate each parser function implementation
    for prop in properties_with_parsers:
        prop.generate_parser_function(to=writer_cpp)
        writer_cpp.write("")

    writer_cpp.write("}")

    output_filepath_cpp = context.project_root / "src" / "CSS" / "Properties" / "CSSPropertyParsers.cpp"
    with open(output_filepath_cpp, "w") as cpp_file:
        cpp_file.write(writer_cpp.get_content())


def generate(context: GeneratorContext):
    properties_json = get_validated_json(pathlib.Path(__file__).parent)

    shorthand_properties: list[CSSProperty] = []
    longhand_properties: list[CSSProperty] = []
    for property_name, property_data in properties_json["properties"].items():
        css_property = CSSProperty.from_json_dict(property_name, property_data)
        if css_property.longhand_names is not None:
            shorthand_properties.append(css_property)
        else:
            longhand_properties.append(css_property)
    all_properties = shorthand_properties + longhand_properties

    __generate_css_property_id_enum(context, all_properties, len(shorthand_properties))

    __generate_css_property_shorthand_files(context, shorthand_properties, longhand_properties)

    __generate__css_property_consumer_file(context, all_properties)
    
    __generate_css_property_parsers_file(context, all_properties)
