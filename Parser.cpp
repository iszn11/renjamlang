#include "Common.h"
#include "Parser.h"

#include "Lexer.h"

using Statements = std::vector<std::unique_ptr<Statement>>;

static bool success;
static std::unique_ptr<Token>* tokenPtr;
static std::unique_ptr<Token>* tokenEnd;

static Error ParseExpression(std::unique_ptr<Expression>& out);

static Error ParseStatement(Statements& statements);
static Error ParseIf(Statements& statements);
static Error ParseWhile(Statements& statements);
static Error ParseAssignment(Statements& statements);
static Error ParseArrayWrite(Statements& statements);
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

static Error ParseStatement(Statements& statements)
{
	return Error{"TODO", (*tokenPtr)->pos};
}
