#pragma once

#include "CodePos.h"
#include "Error.h"

#include "Lexer.h"

#include <memory>
#include <utility>
#include <vector>

enum class TypeTag {
	Void,
	Bool,
	Number,
	Array,
	Function,
};

enum class ExpressionTag {
	False,           // Expression
	True,            // Expression
	NumberLiteral,   // NumberLiteral
	ArrayLiteral,    // ArrayLiteral
	FunctionLiteral, // FunctionLiteral
	Identifier,      // Identifier

	Unary,           // UnaryOperation
	Binary,          // BinaryOperation

	Call,            // Call
};

/*enum class UnaryOp {
	Not,
	Negate,
	Void,
	ArrayLength,
};

enum class BinaryOp {
	Add,
	Sub,
	Mul,
	Div,
	Mod,
	And,
	Or,
	Xor,
	LessThan,
	GreaterThan,
	LessEquals,
	GreaterEquals,
	EqualsEquals,
	NotEquals,
	ArrayRead,
};*/

enum class StatementTag {
	If,         // IfStatement
	While,      // WhileStatement
	Assignment, // AssignmentStatement
	ArrayWrite, // ArrayWriteStatement
	ArrayPush,  // ArrayPushStatement
	ArrayPop,   // ArrayPopStatement
	Return,     // ExpressionStatement
	Expression, // ExpressionStatement
};

struct Statement;

// --- EXPRESSIONS -------------------------------------------------------------

struct Expression {
	ExpressionTag tag;
	CodePos pos;

	Expression(const ExpressionTag tag, const CodePos pos) : tag{tag}, pos{pos} {}
	virtual ~Expression() = default;
};

struct NumberLiteral : public Expression {
	double value;

	NumberLiteral(const double value, const CodePos pos) : Expression{ExpressionTag::NumberLiteral, pos}, value{value} {}
};

struct FunctionLiteral : public Expression {
	std::vector<std::string> args;
	std::vector<std::unique_ptr<Statement>> statements;

	FunctionLiteral(std::vector<std::string> args, std::vector<std::unique_ptr<Statement>> statements, const CodePos pos) : Expression{ExpressionTag::FunctionLiteral, pos}, args{std::move(args)}, statements{std::move(statements)} {}
};

struct Identifier : public Expression {
	std::string name;

	Identifier(std::string name, const CodePos pos) : Expression{ExpressionTag::Identifier, pos}, name{std::move(name)} {}
};

struct UnaryOperation : public Expression {
	TokenTag op;
	std::unique_ptr<Expression> a;

	UnaryOperation(const TokenTag op, std::unique_ptr<Expression> a, const CodePos pos) : Expression{ExpressionTag::Unary, pos}, op{op}, a{std::move(a)} {}
};

struct BinaryOperation : public Expression {
	TokenTag op;
	std::unique_ptr<Expression> a;
	std::unique_ptr<Expression> b;

	BinaryOperation(const TokenTag op, std::unique_ptr<Expression> a, std::unique_ptr<Expression> b, const CodePos pos) : Expression{ExpressionTag::Binary, pos}, op{op}, a{std::move(a)}, b{std::move(b)} {}
};

struct ArrayLiteral : public Expression {
	std::vector<std::unique_ptr<Expression>> values;

	ArrayLiteral(std::vector<std::unique_ptr<Expression>> values, const CodePos pos) : Expression{ExpressionTag::ArrayLiteral, pos}, values{std::move(values)} {}
};

struct Call : public Expression {
	std::unique_ptr<Expression> function;
	std::vector<std::unique_ptr<Expression>> values;

	Call(std::unique_ptr<Expression> function, std::vector<std::unique_ptr<Expression>> values, const CodePos pos) : Expression{ExpressionTag::Call, pos}, function{std::move(function)}, values{std::move(values)} {}
};

// --- STATEMENTS --------------------------------------------------------------

struct Statement {
	StatementTag tag;
	CodePos pos;

	Statement(const StatementTag tag, const CodePos pos) : tag{tag}, pos{pos} {}
	virtual ~Statement() = 0;
};

inline Statement::~Statement() {}

struct ConditionBlock {
	std::unique_ptr<Expression> condition;
	std::vector<std::unique_ptr<Statement>> statements;

	ConditionBlock() = default;
	ConditionBlock(std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Statement>> statements) : condition{std::move(condition)}, statements{std::move(statements)} {}
};

struct IfStatement : public Statement {
	std::vector<ConditionBlock> elifChain;
	std::vector<std::unique_ptr<Statement>> elseBlock;

	IfStatement(std::vector<ConditionBlock> elifChain, std::vector<std::unique_ptr<Statement>> elseBlock, const CodePos pos) : Statement{StatementTag::If, pos}, elifChain{std::move(elifChain)}, elseBlock{std::move(elseBlock)} {}
};

struct WhileStatement : public Statement {
	std::unique_ptr<Expression> condition;
	std::vector<std::unique_ptr<Statement>> statements;

	WhileStatement(std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Statement>> statements, const CodePos pos) : Statement{StatementTag::While, pos}, condition{std::move(condition)}, statements{std::move(statements)} {}
};

struct AssignmentStatement : public Statement {
	std::string name;
	std::unique_ptr<Expression> value;

	AssignmentStatement(std::string name, std::unique_ptr<Expression> value, const CodePos pos) : Statement{StatementTag::Assignment, pos}, name{std::move(name)}, value{std::move(value)} {}
};

struct ArrayWriteStatement : public Statement {
	std::string name;
	std::unique_ptr<Expression> index;
	std::unique_ptr<Expression> value;

	ArrayWriteStatement(std::string name, std::unique_ptr<Expression> index, std::unique_ptr<Expression> value, const CodePos pos) : Statement{StatementTag::ArrayWrite, pos}, name{std::move(name)}, index{std::move(index)}, value{std::move(value)} {}
};

struct ArrayPushStatement : public Statement {
	std::string name;
	std::unique_ptr<Expression> value;

	ArrayPushStatement(std::string name, std::unique_ptr<Expression> value, const CodePos pos) : Statement{StatementTag::ArrayPush, pos}, name{std::move(name)}, value{std::move(value)} {}
};

struct ArrayPopStatement : public Statement {
	std::string name;

	ArrayPopStatement(std::string name, const CodePos pos) : Statement{StatementTag::ArrayPop, pos}, name{std::move(name)} {}
};

struct ExpressionStatement : public Statement {
	std::unique_ptr<Expression> value;

	ExpressionStatement(const StatementTag tag, std::unique_ptr<Expression> value, const CodePos pos) : Statement{tag, pos}, value{std::move(value)} {}
};

// --- PARSER ------------------------------------------------------------------

struct Token;

Error Parse(std::vector<std::unique_ptr<Token>>& tokens, std::vector<std::unique_ptr<Statement>>& statements);
