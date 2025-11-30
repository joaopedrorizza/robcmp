#include "TemplateParams.h"

TemplateParams::TemplateParams() {};

void TemplateParams::append(Variable *e) {
	tpl_parameters.push_back(e);
};

void TemplateParams::insert(int at, Variable *tp) {
	tpl_parameters.insert(tpl_parameters.begin() + at, tp);
}
	
unsigned TemplateParams::getNumCodedParams() {
	unsigned num = 0;
	for(auto p : tpl_parameters) {
		// don't count parameters _this, _parent, and other pseudo 
		// parameters like _any_.size, _any_.cols
		if (p->getName() != "_this" && p->getName() != "_parent" &&
			p->getName().find('.') == string::npos)
			num++;
	}
	return num;
};

DataType TemplateParams::getTemplateParamType(int position) {
	return tpl_parameters[position]->getDataType();
}

const string TemplateParams::getTemplateParamName(int position) {
	return tpl_parameters[position]->getName();
}
