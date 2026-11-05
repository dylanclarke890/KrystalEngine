#include "Krystal.Booey/MathML/Internals/MathMLElementFactory.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLFractionElement.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLMathElement.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLMencloseElement.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLOperatorElement.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLPaddedElement.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLRootElement.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLScriptsElement.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLSpaceElement.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLTokenElement.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLUnderOverElement.hpp"

namespace krys::boo::mathml
{
  RefPtr<MathMLElement> MathMLElementFactory::TryCreate(dom::Document &document,
                                                        html::TagName elementName) noexcept
  {
    using html::TagName;
    using ScriptType = MathMLScriptsElement::ScriptType;

    switch (elementName)
    {
      case TagName::annotation:
      case TagName::annotation_xml:
      case TagName::mprescripts:
      case TagName::mtable:
      case TagName::mtd:
      case TagName::mtr:
      case TagName::none:
      {
        return CreateRefPtr<MathMLPresentationElement>(document);
      }
      case TagName::maction:
      case TagName::merror:
      case TagName::mphantom:
      case TagName::mrow:
      case TagName::mstyle:
      case TagName::semantics:
      {
        return CreateRefPtr<MathMLRowElement>(document);
      }
      case TagName::math:
      {
        return CreateRefPtr<MathMLMathElement>(document);
      }
      case TagName::menclose:
      {
        return CreateRefPtr<MathMLMencloseElement>(document);
      }
      case TagName::mfrac:
      {
        return CreateRefPtr<MathMLFractionElement>(document);
      }
      case TagName::mi:
      case TagName::mn:
      case TagName::ms:
      case TagName::mtext:
      {
        return CreateRefPtr<MathMLTokenElement>(document);
      }
      case TagName::mo:
      {
        return CreateRefPtr<MathMLOperatorElement>(document);
      }
      case TagName::mpadded:
      {
        return CreateRefPtr<MathMLPaddedElement>(document);
      }
      case TagName::mroot:
      {
        return CreateRefPtr<MathMLRootElement>(document, RootType::RootWithIndex);
      }
      case TagName::msqrt:
      {
        return CreateRefPtr<MathMLRootElement>(document, RootType::SquareRoot);
      }
      case TagName::mspace:
      {
        return CreateRefPtr<MathMLSpaceElement>(document);
      }
      case TagName::msub:
      {
        return CreateRefPtr<MathMLScriptsElement>(document, ScriptType::Sub);
      }
      case TagName::msup:
      {
        return CreateRefPtr<MathMLScriptsElement>(document, ScriptType::Super);
      }
      case TagName::msubsup:
      {
        return CreateRefPtr<MathMLScriptsElement>(document, ScriptType::SubSup);
      }
      case TagName::mmultiscripts:
      {
        return CreateRefPtr<MathMLScriptsElement>(document, ScriptType::Multiscripts);
      }
      case TagName::munder:
      {
        return CreateRefPtr<MathMLUnderOverElement>(document, ScriptType::Under);
      }
      case TagName::mover:
      {
        return CreateRefPtr<MathMLUnderOverElement>(document, ScriptType::Over);
      }
      case TagName::munderover:
      {
        return CreateRefPtr<MathMLUnderOverElement>(document, ScriptType::UnderOver);
      }
      default:
      {
        break;
      }
    }

    return nullptr;
  }
}
