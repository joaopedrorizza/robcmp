#include "TemplateCall.h"
#include "TemplateImpl.h"
#include "FunctionImpl.h"
#include "FunctionCall.h"
#include "FunctionDecl.h"
#include "Program.h"
#include "BuildTypes.h"
#include "semantic/Visitor.h"
#include "semantic/PropagateTypes.h"
#include "ParamsCall.h"
#include <iostream>

extern Program *program;
extern std::unique_ptr<BuildTypes> buildTypes;


DataType TemplateCall::getDataType() {
    
    if (dt == BuildTypes::undefinedType) {
        // is a constructor? this can occur while running PropagateTypes.
        dt = buildTypes->getType(ident.getFullName());
        if (node_children.size() <= 1 && dt != BuildTypes::undefinedType) {
            return dt;
        }

        if (!symbol)
            symbol = ident.getSymbol(getScope());

        if (symbol)
            dt = symbol->getDataType();
    }
    return dt;
}

static TemplateImpl *lookupTemplateImpl(Node *scope, const std::string &baseName, location_t loc)
{
    if (!scope)
        return nullptr;

    // 1) tenta resolver no escopo atual
    Identifier templ_ident(baseName, loc);
    if (Node *sym = templ_ident.getSymbol(scope))
    {
        if (auto *ti = dynamic_cast<TemplateImpl *>(sym))
            return ti;
    }

    // 2) fallback global no Program
    if (program)
    {
        Identifier glob(baseName, loc);
        if (Node *s = glob.getSymbol(program))
        {
            if (auto *ti2 = dynamic_cast<TemplateImpl *>(s))
                return ti2;
        }
    }
    return nullptr;
}

/*const std::string TemplateCall::instantiate(const string& baseName)
{

       ParamsCall *newParams = new ParamsCall();
    for (Node *param : this->getParameters())
    {
        newParams->append(param);
    }
   //const std::string baseName = ident.getFullName();

    // 1) lookup do TemplateImpl 
    TemplateImpl *templImpl = lookupTemplateImpl(getScope(), baseName, getLoc());
    if (!templImpl)
    {
        // NÃO é template → vira FunctionCall do nome base, usando os MESMOS argumentos
        auto *call = new FunctionCall(baseName, newParams, getLoc()); // <-- AQUI
        call->setScope(getScope());
        if (leftValue)
            call->setLeftValue(leftValue);
        return call;
    }

    // 2) instanciar
    Node *instNode = templImpl->generateFor(this->getTemplateArgs());

    auto *concreteFunc = dynamic_cast<FunctionImpl *>(instNode);
    if (!concreteFunc)
    {
        yyerrorcpp("Template instantiation did not produce a FunctionImpl for '" + baseName + "'.", this);
        return nullptr;
    }

    //FunctionCall *call = new FunctionCall(concreteFunc->getName(), newParams, getLoc()); // <-- AQUI
    //call->setScope(getScope());

    //(log opcional)
    std::cerr << "[TemplateCall] Lowered " << baseName << " to "
              << concreteFunc->getName() << " with params from ParamsCall\n";

    return concreteFunc->getName();
}*/

Value *TemplateCall::generate(FunctionImpl *, BasicBlock *, BasicBlock *)
{
    // segurança: TemplateCall nunca deve chegar na geração de código
    yyerrorcpp("TemplateCall must be lowered to FunctionCall before codegen.", this);
    return nullptr;
}

Node *TemplateCall::accept(Visitor &v) { return v.visit(*this); }
