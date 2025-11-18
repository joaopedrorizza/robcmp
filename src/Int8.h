
#pragma once

#include "Node.h"

class Int8: public Cloneable<Int8> {
private:
	int8_t number;

public:
	Int8(int8_t n, location_t loc): Cloneable<Int8>(loc), number(n) {}

	Int8(const Int8& n, TypeSubs& ts): Cloneable<Int8>(n.getLoc()), number(n.number) {}

	int8_t getNumber() const { return number; }
	void setNumber(int8_t n) { number = n; }

	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;
	
	virtual DataType getDataType() override {
		return tint8;
	}

	virtual bool isConstExpr() override {
		return true;
	}
};

class UInt8: public Node {
private:
	uint8_t number;

public:
	UInt8(uint8_t n, location_t loc): Node(loc), number(n) {
		dt = tint8u;
	}

	uint8_t getNumber() const { return number; }
	void setNumber(uint8_t n) { number = n; }

	virtual Value *generate(FunctionImpl *func, BasicBlock *block, BasicBlock *allocblock) override;
	
	virtual bool isConstExpr() override {
		return true;
	}
};