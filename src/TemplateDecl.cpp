/*#include "TemplateDecl.h"
#include "Program.h"
#include "FunctionDecl.h"
#include "BackLLVM.h"
#include "FunctionAttributes.h"
#include "Array.h"
#include "Matrix.h"


Value *TemplateDecl::generate(FunctionImpl*, BasicBlock *, BasicBlock *allocblock) {
	
	if (func)
		return func;

	Node *symbol = findSymbol(name);
	if (symbol != NULL && symbol != this) {
		yyerrorcpp("Function/symbol " + name + " already defined.", this);
		yyerrorcpp(name + " was first defined here.", symbol);
		return NULL;
	}

	std::vector<Type*> arg_types;
	if (!validateAndGetArgsTypes(arg_types))
		return NULL;

	Type *xtype = buildTypes->llvmType(dt);
	if (returnIsPointer)
		xtype = PointerType::getUnqual(xtype);
	
	FunctionType *ftype = FunctionType::get(xtype, ArrayRef<Type*>(arg_types), false);
	Function *nfunc = Function::Create(ftype, linkage, codeAddrSpace, getFinalName(), mainmodule);
	nfunc->setCallingConv(CallingConv::C);
	addFunctionAttributes(nfunc);

	if (buildTypes->isUnsignedDataType(dt))
		nfunc->addRetAttr(Attribute::ZExt);
	
	unsigned Idx = 0;
	for (auto &Arg : nfunc->args()) {
		Variable *fp = parameters->getParameters()[Idx];
		DataType ptype = fp->getDataType();

		if (buildTypes->isUnsignedDataType(ptype))
			Arg.addAttr(Attribute::ZExt);

		const string& argname = fp->getName();
		if (argname == "_this") {
			thisArg = &Arg;
		} else if (argname == "_parent") {
			parentArg = &Arg;
		}

		Idx++;
	}

	func = nfunc;
	return func;
}

Value* TemplateDecl::getLLVMValue(Node *, FunctionImpl *) {
	if (!func)
		generate(NULL, NULL, global_alloc);
	return func;
}*/

