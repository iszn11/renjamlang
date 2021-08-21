#include "Common.h"
#include "Lexer.h"

#include <cstdio>
#include <iostream>

static void PrintLexResults(const std::string_view filePrefix, const std::vector<std::unique_ptr<Token>>& tokens)
{
	for (const auto& token : tokens)
	{
		std::cout << filePrefix << ':' << token->pos.line << ':' << token->pos.col << ':';

		switch (token->tag)
		{
			case TokenTag::KeyVoid: std::cout << "KeyVoid"; break;
			case TokenTag::KeyIf: std::cout << "KeyIf"; break;
			case TokenTag::KeyElif: std::cout << "KeyElif"; break;
			case TokenTag::KeyElse: std::cout << "KeyElse"; break;
			case TokenTag::KeyWhile: std::cout << "KeyWhile"; break;
			case TokenTag::KeyEnd: std::cout << "KeyEnd"; break;
			case TokenTag::KeyFn: std::cout << "KeyFn"; break;
			case TokenTag::KeyReturn: std::cout << "KeyReturn"; break;
			case TokenTag::KeyPush: std::cout << "KeyPush"; break;
			case TokenTag::KeyPop: std::cout << "KeyPop"; break;
			case TokenTag::KeyNot: std::cout << "KeyNot"; break;
			case TokenTag::KeyAnd: std::cout << "KeyAnd"; break;
			case TokenTag::KeyOr: std::cout << "KeyOr"; break;
			case TokenTag::KeyXor: std::cout << "KeyXor"; break;
			case TokenTag::KeyNeg: std::cout << "KeyNeg"; break;
			case TokenTag::KeyFalse: std::cout << "KeyFalse"; break;
			case TokenTag::KeyTrue: std::cout << "KeyTrue"; break;
			case TokenTag::BracketOpen: std::cout << "BracketOpen"; break;
			case TokenTag::BracketClose: std::cout << "BracketClose"; break;
			case TokenTag::ParenOpen: std::cout << "ParenOpen"; break;
			case TokenTag::ParenClose: std::cout << "ParenClose"; break;
			case TokenTag::Plus: std::cout << "Plus"; break;
			case TokenTag::Minus: std::cout << "Minus"; break;
			case TokenTag::Star: std::cout << "Star"; break;
			case TokenTag::Slash: std::cout << "Slash"; break;
			case TokenTag::Percent: std::cout << "Percent"; break;
			case TokenTag::Equals: std::cout << "Equals"; break;
			case TokenTag::LessThan: std::cout << "LessThan"; break;
			case TokenTag::GreaterThan: std::cout << "GreaterThan"; break;
			case TokenTag::LessEquals: std::cout << "LessEquals"; break;
			case TokenTag::GreaterEquals: std::cout << "GreaterEquals"; break;
			case TokenTag::EqualsEquals: std::cout << "EqualsEquals"; break;
			case TokenTag::NotEquals: std::cout << "NotEquals"; break;
			case TokenTag::At: std::cout << "At"; break;
			case TokenTag::Hash: std::cout << "Hash"; break;
			case TokenTag::Number: std::cout << "Number " << static_cast<NumberToken*>(token.get())->value; break;
			case TokenTag::Identifier: std::cout << "Identifier " << static_cast<IdentifierToken*>(token.get())->name; break;
			case TokenTag::Comment:
			{
				std::cout << "Comment\n";

				for (const auto& node : static_cast<CommentToken*>(token.get())->nodes)
				{
					switch (node->tag)
					{
						case CommentNodeTag::Text: std::cout << "\tText " << static_cast<CommentTextNode*>(node.get())->text; break;
						case CommentNodeTag::Expression: std::cout << "\tExpression"; break; // TODO Print tokens
					}
				}
				break;
			}
		}

		std::cout << '\n';
	}
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " FILE\n";
		std::cerr << "Expected 1 argument, got " << (argc - 1) << '\n';
		return 1;
	}

	const char* const filepath = argv[1];

	std::string code;
	if (!ReadFile(filepath, code))
	{
		std::cerr << "Couldn't read file " << filepath << '\n';
		return 1;
	}

	std::vector<std::unique_ptr<Token>> tokens;
	Error error = Lex(code.c_str(), tokens);
	if (error)
	{
		std::cerr << filepath << ":" << error.pos.line << ":" << error.pos.col << ": Lexer error: " << error.message << '\n';
		return 1;
	}

	PrintLexResults(filepath, tokens);
}
