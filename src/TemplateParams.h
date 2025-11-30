#pragma once

#include "Node.h"
#include "Variable.h"

class TemplateParams {
private:
	std::vector<Variable*> tpl_parameters;

public:
	TemplateParams();
	void append(Variable *fp);
	void insert(int at, Variable *fp);
	
	unsigned getNumCodedParams();
	DataType getTemplateParamType (int position);
	const string getTemplateParamName (int position);

	std::vector<Variable*> const& getTemplateParameters() {
		return tpl_parameters;
	}	
};
