#include "TemplateParamNode.h"
#include "semantic/Visitor.h"

Node* TemplateParamNode::accept(Visitor &v) {
    return v.visit(*this);
}

const std::string TemplateParamNode::getName() const {
    return name;
}
