#pragma once

#include "Node.h"
#include "Identifier.h"
#include "ParamsCall.h"
#include "Variable.h"

class TemplateCall : public Node
{
private:
    Identifier ident_;                      // nome base da função template (ex: "swap")
    std::vector<std::string> templateArgs_; // ex: ["int8", ...]
    ParamsCall *args_;                      // <-- MANTER os argumentos reais aqui!
    Variable *leftValue_ = nullptr;
    DataType dt_ = BuildTypes::undefinedType;

public:
    TemplateCall(const Identifier &id,
                 std::vector<std::string> &&tplArgs,
                 ParamsCall *args, // <-- receba do .y
                 location_t loc)
        : Node(loc), ident_(id), templateArgs_(std::move(tplArgs)), args_(args)
    {
    }

    const Identifier &getIdent() const { return ident_; }
    const std::vector<std::string> &getTemplateArgs() const { return templateArgs_; }
    ParamsCall *getArgs() const { return args_; } // <-- acessor

    void setLeftValue(Variable *lv) { leftValue_ = lv; }
    Variable *getLeftValue() const { return leftValue_; }

    Node *instantiateAndLower(); // <-- lowering
    Value *generate(FunctionImpl *, BasicBlock *, BasicBlock *) override;
    DataType getDataType() override { return dt_; }
    Node *accept(Visitor &v) override;
};
