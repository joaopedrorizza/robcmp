
#pragma once

#include "Node.h"
#include "ParamsCall.h"
#include "Identifier.h"
#include "Variable.h"

class TemplateCall : public Node {
private:
    Identifier ident;                       // nome da função (ex: swap)
    std::vector<std::string> templateParams; // lista dos tipos do template
    Node *symbol = NULL; 
    Variable *leftValue = NULL;

public:
    TemplateCall(const std::string& name,
                         std::vector<std::string> tparams,
                         ParamsCall *pc,
                         location_t loc)
        : Node(loc), ident(name, loc), templateParams(std::move(tparams)) {
        if (pc) {
            node_children.reserve(pc->getNumParams());
            node_children.insert(end(node_children),
                pc->getParameters().begin(), pc->getParameters().end());
            delete pc;
        }
    }

    // acesso aos parâmetros de template
    const std::vector<std::string>& getTemplateParams() const {
        return templateParams;
    }

    Identifier& getIdent() {
        return ident;
    }

    const std::string getName() const override {
        return ident.getFullName();
    }

    std::vector<Node *>& getParameters() {
        return node_children;
    }

    virtual void setLeftValue(Variable *symbol) override {
        leftValue = symbol;
    }

    // ainda não resolver tipos aqui
    virtual DataType getDataType() override {
        return dt;
    }

    // geração vai instanciar a função concreta
    virtual Value *generate(FunctionImpl *func,
                            BasicBlock *block,
                            BasicBlock *allocblock) override;

    Node* accept(Visitor& v) override;
};
