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

Value *TemplateCall::generate(FunctionImpl *, BasicBlock *, BasicBlock *)
{
    // segurança: TemplateCall nunca deve chegar na geração de código
    yyerrorcpp("TemplateCall must be lowered to FunctionCall before codegen.", this);
    return nullptr;
}

Node *TemplateCall::accept(Visitor &v) { return v.visit(*this); }
