#include "Common.h"
#include "Parser.h"

#include "Lexer.h"

using Statements = std::vector<std::unique_ptr<Statement>>;

static bool success;
static std::unique_ptr<Token>* tokenPtr;
static std::unique_ptr<Token>* tokenEnd;

static bool EatToken(const TokenTag tag);
static bool IsToken(const TokenTag tag);
static TokenTag GetTag();
static CodePos GetPos();

static Error ParseExpression(std::unique_ptr<Expression>& out);
static Error ParseExpressionInternal(std::unique_ptr<Expression>& out);

static Error ParseStatement(Statements& statements);
static Error ParseIf(Statements& statements);
static Error ParseWhile(Statements& statements);
static Error ParseAssignment(Statements& statements);
static Error ParseArrayPush(Statements& statements);
static Error ParseArrayPop(Statements& statements);
static Error ParseReturn(Statements& statements);
static Error ParseExpressionStatement(Statements& statements);

Error Parse(std::vector<std::unique_ptr<Token>>& tokens, Statements& statements)
{
	tokenPtr = &tokens.front();
	tokenEnd = &tokens.back();

	while (tokenPtr != tokenEnd)
	{
		TRY(ParseStatement(statements));
		if (!success)
		{
			return Error{"Unrecognized statement", (*tokenPtr)->pos};
		}
	}

	return Error::None;
}

static bool EatToken(const TokenTag tag)
{
	if ((*tokenPtr)->tag == tag)
	{
		tokenPtr += 1;
		return true;
	}

	return false;
}

static bool IsToken(const TokenTag tag)
{
	return (*tokenPtr)->tag == tag;
}

template <typename T>
static T* GetToken()
{
	return static_cast<T*>(tokenPtr->get());
}

static TokenTag GetTag()
{
	return (*tokenPtr)->tag;
}

static CodePos GetPos()
{
	return (*tokenPtr)->pos;
}

static Error ParseExpression(std::unique_ptr<Expression>& out)
{
	const CodePos pos = GetPos();

	TRY(ParseExpressionInternal(out));

	// function call

	while (EatToken(TokenTag::ParenOpen))
	{
		std::vector<std::unique_ptr<Expression>> values;

		while (!EatToken(TokenTag::ParenClose))
		{
			std::unique_ptr<Expression> value;
			TRY(ParseExpression(value));
			values.emplace_back(std::move(value));
		}

		std::unique_ptr<Expression> call = std::make_unique<Call>(std::move(out), std::move(values), pos);
		call.swap(out);
	}

	return Error::None;
}

static Error ParseExpressionInternal(std::unique_ptr<Expression>& out)
{
	const CodePos pos = GetPos();
	const TokenTag tag = GetTag();

	switch (tag)
	{
		// literals

		case TokenTag::KeyFalse: out = std::make_unique<Expression>(ExpressionTag::False, pos); tokenPtr += 1; return Error::None;
		case TokenTag::KeyTrue: out = std::make_unique<Expression>(ExpressionTag::True, pos); tokenPtr += 1; return Error::None;
		case TokenTag::Number: out = std::make_unique<NumberLiteral>(GetToken<NumberToken>()->value, pos); tokenPtr += 1; return Error::None;
		case TokenTag::BracketOpen:
		{
			tokenPtr += 1;

			std::vector<std::unique_ptr<Expression>> values;
			while (!EatToken(TokenTag::BracketClose))
			{
				std::unique_ptr<Expression> value;
				TRY(ParseExpression(value));
				values.emplace_back(std::move(value));
			}

			out = std::make_unique<ArrayLiteral>(std::move(values), pos);
			return Error::None;
		}
		case TokenTag::KeyFn:
		{
			tokenPtr += 1;

			if (!EatToken(TokenTag::ParenOpen))
			{
				return Error{"Expected \"(\" to start function argument list", GetPos()};
			}

			std::vector<std::string> args;
			while (!EatToken(TokenTag::ParenClose))
			{
				if (!IsToken(TokenTag::Identifier))
				{
					return Error{"Expected indentifier in function argument list", GetPos()};
				}
				const std::string& name = GetToken<IdentifierToken>()->name;
				tokenPtr += 1;

				args.emplace_back(name);
			}

			Statements statements;
			while (!EatToken(TokenTag::KeyEnd))
			{
				TRY(ParseStatement(statements));
			}

			out = std::make_unique<FunctionLiteral>(std::move(args), std::move(statements), pos);
			return Error::None;
		}

		// identifier

		case TokenTag::Identifier:
		{
			const std::string& name = GetToken<IdentifierToken>()->name;
			tokenPtr += 1;

			out = std::make_unique<Identifier>(name, pos);
			return Error::None;
		}

		// unary

		case TokenTag::KeyNot:
		case TokenTag::KeyNeg:
		case TokenTag::KeyVoid:
		case TokenTag::Hash:
		{
			tokenPtr += 1;

			std::unique_ptr<Expression> a;
			TRY(ParseExpression(a));
			out = std::make_unique<UnaryOperation>(tag, std::move(a), pos);
			return Error::None;
		}

		// binary

		case TokenTag::Plus:
		case TokenTag::Minus:
		case TokenTag::Star:
		case TokenTag::Slash:
		case TokenTag::Percent:
		case TokenTag::KeyAnd:
		case TokenTag::KeyOr:
		case TokenTag::KeyXor:
		case TokenTag::LessThan:
		case TokenTag::GreaterThan:
		case TokenTag::LessEquals:
		case TokenTag::GreaterEquals:
		case TokenTag::EqualsEquals:
		case TokenTag::NotEquals:
		case TokenTag::At:
		{
			tokenPtr += 1;

			std::unique_ptr<Expression> a;
			std::unique_ptr<Expression> b;
			TRY(ParseExpression(a));
			TRY(ParseExpression(b));
			out = std::make_unique<BinaryOperation>(tag, std::move(a), std::move(b), pos);
			return Error::None;
		}

		default:
			return Error{"Unrecognized expression", pos};
	}
}

static Error ParseStatement(Statements& statements)
{
	Error error = ParseIf(statements);
	if (error || success) return error;

	error = ParseWhile(statements);
	if (error || success) return error;

	error = ParseAssignment(statements);
	if (error || success) return error;

	error = ParseArrayPush(statements);
	if (error || success) return error;

	error = ParseArrayPop(statements);
	if (error || success) return error;

	error = ParseReturn(statements);
	if (error || success) return error;

	error = ParseExpressionStatement(statements);
	if (error || success) return error;

	success = false;
	return Error::None;
}

static Error ParseIf(Statements& statements)
{
	const CodePos pos = GetPos();

	if (!EatToken(TokenTag::KeyIf))
	{
		success = false;
		return Error::None;
	}

	std::vector<ConditionBlock> elifChain;
	std::vector<std::unique_ptr<Statement>> elseBlock;

	while (true)
	{
		std::unique_ptr<Expression> condition;
		TRY(ParseExpression(condition));

		Statements innerStatements;
		while (!IsToken(TokenTag::KeyElif) && !IsToken(TokenTag::KeyElse) && !IsToken(TokenTag::KeyEnd))
		{
			TRY(ParseStatement(innerStatements));
		}

		elifChain.emplace_back(std::move(condition), std::move(innerStatements));

		if (IsToken(TokenTag::KeyElif)) { tokenPtr += 1; continue; }
		if (IsToken(TokenTag::KeyEnd)) { tokenPtr += 1; break; }

		tokenPtr += 1;
		while (!EatToken(TokenTag::KeyEnd))
		{
			TRY(ParseStatement(elseBlock));
		}
	}

	success = true;
	statements.emplace_back(std::make_unique<IfStatement>(std::move(elifChain), std::move(elseBlock), pos));
	return Error::None;
}

static Error ParseWhile(Statements& statements)
{
	const CodePos pos = GetPos();

	if (!EatToken(TokenTag::KeyWhile))
	{
		success = false;
		return Error::None;
	}

	std::unique_ptr<Expression> condition;
	TRY(ParseExpression(condition));

	Statements innerStatements;
	while (!EatToken(TokenTag::KeyEnd))
	{
		TRY(ParseStatement(innerStatements));
	}

	success = true;
	statements.emplace_back(std::make_unique<WhileStatement>(std::move(condition), std::move(innerStatements), pos));
	return Error::None;
}

static Error ParseAssignment(Statements& statements)
{
	const CodePos pos = GetPos();

	if (!EatToken(TokenTag::Equals))
	{
		success = false;
		return Error::None;
	}

	// assignment
	if (IsToken(TokenTag::Identifier))
	{
		const std::string& name = GetToken<IdentifierToken>()->name;
		tokenPtr += 1;

		std::unique_ptr<Expression> value;
		TRY(ParseExpression(value));

		success = true;
		statements.emplace_back(std::make_unique<AssignmentStatement>(name, std::move(value), pos));
		return Error::None;
	}
	// array write
	else if (IsToken(TokenTag::At))
	{
		tokenPtr += 1;

		if (!IsToken(TokenTag::Identifier))
		{
			Error{"Expected identifier in array write. NOTE: Array write to expression is not supported.", GetPos()};
		}
		const std::string& name = GetToken<IdentifierToken>()->name;
		tokenPtr += 1;

		std::unique_ptr<Expression> index;
		TRY(ParseExpression(index));

		std::unique_ptr<Expression> value;
		TRY(ParseExpression(value));

		success = true;
		statements.emplace_back(std::make_unique<ArrayWriteStatement>(name, std::move(index), std::move(value), pos));
		return Error::None;
	}
	else
	{
		return Error{"Expected \"@\" (array write) or identifier (assignment) after \"=\".", GetPos()};
	}
}

static Error ParseArrayPush(Statements& statements)
{
	const CodePos pos = GetPos();

	if (!EatToken(TokenTag::KeyPush))
	{
		success = false;
		return Error::None;
	}

	if (!IsToken(TokenTag::Identifier))
	{
		Error{"Expected identifier in array push. NOTE: Array push to expression is not supported.", GetPos()};
	}
	const std::string& name = GetToken<IdentifierToken>()->name;
	tokenPtr += 1;

	std::unique_ptr<Expression> value;
	TRY(ParseExpression(value));

	success = true;
	statements.emplace_back(std::make_unique<ArrayPushStatement>(name, std::move(value), pos));
	return Error::None;
}

static Error ParseArrayPop(Statements& statements)
{
	const CodePos pos = GetPos();

	if (!EatToken(TokenTag::KeyPop))
	{
		success = false;
		return Error::None;
	}

	if (!IsToken(TokenTag::Identifier))
	{
		Error{"Expected identifier in array pop. NOTE: Array pop of expression is not supported.", GetPos()};
	}
	const std::string& name = GetToken<IdentifierToken>()->name;
	tokenPtr += 1;

	success = true;
	statements.emplace_back(std::make_unique<ArrayPopStatement>(name, pos));
	return Error::None;
}

static Error ParseReturn(Statements& statements)
{
	const CodePos pos = GetPos();

	if (!EatToken(TokenTag::KeyReturn))
	{
		success = false;
		return Error::None;
	}

	std::unique_ptr<Expression> value;
	TRY(ParseExpression(value));

	success = true;
	statements.emplace_back(std::make_unique<ExpressionStatement>(StatementTag::Return, std::move(value), pos));
	return Error::None;
}

static Error ParseExpressionStatement(Statements& statements)
{
	const CodePos pos = GetPos();

	std::unique_ptr<Expression> value;
	TRY(ParseExpression(value));

	success = true;
	statements.emplace_back(std::make_unique<ExpressionStatement>(StatementTag::Expression, std::move(value), pos));
	return Error::None;
}
