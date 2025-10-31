#pragma once

#include "PropagateTypes.h"
#include "Visitor.h"
#include "Identifier.h"
#include "TemplateImpl.h"
#include "TemplateCall.h"
#include "FunctionCall.h"
#include "Program.h"
#include "BuildTypes.h"
#include "Variable.h"
#include "Return.h"
#include "ParamsCall.h"
#include <map>
#include <iostream>

// ... includes iguais
class ExpandTemplates : public PropagateTypes
{
    // mapeamento T -> tipo concreto (já existente no seu código)
    std::map<std::string, DataType> *subMap = nullptr;

public:
    ExpandTemplates() = default;
    explicit ExpandTemplates(std::map<std::string, DataType> *m) : subMap(m) {}

    // *** NÃO mais visit(TemplateCall) aqui ***

    Node *visit(Return &r) override
    {
        Node *newExpr = nullptr;

        // Se o Return tiver expressão (dependendo de como você a armazena; abaixo assumo children()[0])
        if (!r.children().empty() && r.children()[0])
        {
            Node *exp = r.children()[0]->accept(*this);
            newExpr = exp ? exp : r.children()[0];
        }

        if (newExpr)
        {
            // Usa o construtor Return(Node*) — sem clonagem
            return new Return(newExpr);
        }
        else
        {
            // Return sem expressão: constrói só com a localização original
            return new Return(r.getLoc());
        }
    }

    // Ex.: se precisar tocar em Variable para trocar tipos T->concreto:
    Node *visit(Variable &v) override
    {
        if (!subMap)
            return nullptr;
        // nome do tipo atual
        std::string tyName;
        yyerrorcpp("Expanding variable of type: " + tyName = buildTypes->name(v.getDataType()), &v);
        auto it = subMap->find(tyName);
        if (it == subMap->end())
            return nullptr; // sem substituição → mantém original

        DataType concrete = it->second;
        auto *nv = new Variable(v.getIdent().getFullName(), concrete, v.getLoc());
        return nv;
    }

    // mantenha os demais visits necessários (Assignment, Decl, etc.), sempre criando nós novos

    Node *visit(TemplateCall &n) override
    {
        // 1) localizar TemplateImpl base
        std::string baseName = n.getIdent().getFullName();
        Node *sym = n.getScope()->findSymbol(baseName);
        auto *tpl = dynamic_cast<TemplateImpl *>(sym);
        if (!tpl)
        {
            yyerrorcpp("Template not found: " + baseName, &n);
            return nullptr;
        }

        // 2) instanciar com os args de template
        const auto &targs = n.getTemplateArgs(); // ex.: ["int8"]
        std::vector<std::string> concreteTypes(targs.begin(), targs.end());

        Node *inst = tpl->generateFor(concreteTypes);
        auto *concreteFunc = dynamic_cast<FunctionImpl *>(inst);

        if (!concreteFunc)
        {
            yyerrorcpp("Template instantiation failed for: " + baseName, &n);
            return nullptr;
        }

        // 3) visite argumentos reais e construa FunctionCall
        ParamsCall *pc = n.getArgs();
        ParamsCall *newPc = new ParamsCall();
        if (pc)
        {
            for (auto *arg : pc->getParameters())
            {
                Node *a2 = arg ? arg->accept(*this) : nullptr;
                if (a2)
                    newPc->append(a2);
            }
        }
        auto *call = new FunctionCall(concreteFunc->getName(), newPc, n.getLoc());
        return call;
    }
};
