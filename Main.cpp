#include "Lexer.h"

#include <cstdio>
#include <iostream>

static bool ReadFile(const char* filepath, std::string& out)
{
	out.clear();

	FILE* file = fopen(filepath, "rb");
	if (!file) return false;

	fseek(file, 0, SEEK_END);
	const long size = ftell(file);
	rewind(file);

	out.resize(size);
	fread(out.data(), 1, size, file);
	fclose(file);

	return true;
}

static void PrintLexResults(const std::string_view filePrefix, const std::vector<Token>& tokens)
{
	for (const auto& token : tokens)
	{
		std::cout << filePrefix << ':' << token.line << ':' << token.col << ':';

		switch (token.tag)
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
			case TokenTag::Newline: std::cout << "Newline"; break;
			case TokenTag::Comma: std::cout << "Comma"; break;
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
			case TokenTag::Number: std::cout << "Number " << token.value; break;
			case TokenTag::Identifier: std::cout << "Identifier " << token.text; break;
			case TokenTag::Comment: std::cout << "Comment " << token.text; break;
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

	std::string code;
	if (!ReadFile(argv[1], code))
	{
		std::cerr << "Couldn't read file " << argv[1] << '\n';
		return 1;
	}

	std::vector<Token> tokens;
	LexerError error = Lex(code.c_str(), tokens);
	if (error.error)
	{
		std::cerr << "Lexer error: " << error.message << " at line " << error.line << ", column " << error.col << '\n';
		return 1;
	}

	PrintLexResults(argv[1], tokens);
}
