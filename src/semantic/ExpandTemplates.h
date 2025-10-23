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
        std::string tyName = buildTypes->name(v.getDataType());
        auto it = subMap->find(tyName);
        if (it == subMap->end())
            return nullptr; // sem substituição → mantém original

        DataType concrete = it->second;
        auto *nv = new Variable(v.getIdent().getFullName(), concrete, v.getLoc());
        return nv;
    }

    // mantenha os demais visits necessários (Assignment, Decl, etc.), sempre criando nós novos
};
