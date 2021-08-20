#include "Lexer.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>

#define TRY(x) do { error = (x); if (error) return error; } while (false)

using namespace std::literals;

struct CodePtr {
	const char* ptr;
	size_t line{1};
	size_t col{1};

	CodePtr(const char* const ptr) : ptr{ptr} {}

	const char& operator[](const size_t index) const { return ptr[index]; }

	CodePtr& operator+=(const size_t count)
	{
		for (size_t i = 0; i < count; ++i, ++ptr)
		{
			const char c = *ptr;
			if (c == '\n')
			{
				line += 1;
				col = 1;
			}
			else
			{
				col += 1;
			}
		}
		return *this;
	}
};

constexpr size_t KEYWORD_COUNT = 17;
constexpr std::string_view KEYWORDS[KEYWORD_COUNT] = {
	"void"sv,
	"if"sv,
	"elif"sv,
	"else"sv,
	"while"sv,
	"end"sv,
	"fn"sv,
	"return"sv,
	"push"sv,
	"pop"sv,
	"not"sv,
	"and"sv,
	"or"sv,
	"xor"sv,
	"neg"sv,
	"false"sv,
	"true"sv,
};

static bool success;

static void SkipWhitespace(CodePtr& ptr);
static bool IsIdentifierStart(char c);
static bool IsIdentifierMiddle(char c);
static bool IsDigit(char c);
static LexerError LexComment(CodePtr& ptr, Token& out);
static LexerError LexIdentifierOrKeyword(CodePtr& ptr, Token& out);
static LexerError LexNumber(CodePtr& ptr, Token& out);

LexerError Lex(const char* const code, std::vector<Token>& tokens)
{
	LexerError error;
	Token token;
	CodePtr codePtr{code};

	while (true)
	{
		// whitespace

		SkipWhitespace(codePtr);

		if (codePtr[0] == '\0') return LexerError{};

		// newline

		if (codePtr[0] == '\n')
		{
			tokens.emplace_back(TokenTag::Newline, codePtr.line, codePtr.col);
			codePtr += 1;

			continue;
		}

		// comment

		TRY(LexComment(codePtr, token));
		if (success)
		{
			tokens.emplace_back(token);
			continue;
		}

		// identifier or keyword

		TRY(LexIdentifierOrKeyword(codePtr, token));
		if (success)
		{
			tokens.emplace_back(token);
			continue;
		}

		// number

		TRY(LexNumber(codePtr, token));
		if (success)
		{
			tokens.emplace_back(token);
			continue;
		}

		// simple tokens (2 chars)

		if (codePtr[0] != '\0')
		{
			const uint16_t val = (codePtr[0] << 8) | codePtr[1];
			switch (val)
			{
				case 0x3C3D: tokens.emplace_back(TokenTag::LessEquals, codePtr.line, codePtr.col);    codePtr += 2; continue;
				case 0x3E3D: tokens.emplace_back(TokenTag::GreaterEquals, codePtr.line, codePtr.col); codePtr += 2; continue;
				case 0x3D3D: tokens.emplace_back(TokenTag::EqualsEquals, codePtr.line, codePtr.col);  codePtr += 2; continue;
				case 0x213D: tokens.emplace_back(TokenTag::NotEquals, codePtr.line, codePtr.col);     codePtr += 2; continue;
			}
		}

		// simple tokens (1 char)

		switch (codePtr[0])
		{
			case ',': tokens.emplace_back(TokenTag::Comma, codePtr.line, codePtr.col);        codePtr += 1; continue;
			case '[': tokens.emplace_back(TokenTag::BracketOpen, codePtr.line, codePtr.col);  codePtr += 1; continue;
			case ']': tokens.emplace_back(TokenTag::BracketClose, codePtr.line, codePtr.col); codePtr += 1; continue;
			case '(': tokens.emplace_back(TokenTag::ParenOpen, codePtr.line, codePtr.col);    codePtr += 1; continue;
			case ')': tokens.emplace_back(TokenTag::ParenClose, codePtr.line, codePtr.col);   codePtr += 1; continue;
			case '+': tokens.emplace_back(TokenTag::Plus, codePtr.line, codePtr.col);         codePtr += 1; continue;
			case '-': tokens.emplace_back(TokenTag::Minus, codePtr.line, codePtr.col);        codePtr += 1; continue;
			case '*': tokens.emplace_back(TokenTag::Star, codePtr.line, codePtr.col);         codePtr += 1; continue;
			case '/': tokens.emplace_back(TokenTag::Slash, codePtr.line, codePtr.col);        codePtr += 1; continue;
			case '%': tokens.emplace_back(TokenTag::Percent, codePtr.line, codePtr.col);      codePtr += 1; continue;
			case '=': tokens.emplace_back(TokenTag::Equals, codePtr.line, codePtr.col);       codePtr += 1; continue;
			case '<': tokens.emplace_back(TokenTag::LessThan, codePtr.line, codePtr.col);     codePtr += 1; continue;
			case '>': tokens.emplace_back(TokenTag::GreaterThan, codePtr.line, codePtr.col);  codePtr += 1; continue;
			case '@': tokens.emplace_back(TokenTag::At, codePtr.line, codePtr.col);           codePtr += 1; continue;
			case '#': tokens.emplace_back(TokenTag::Hash, codePtr.line, codePtr.col);         codePtr += 1; continue;
			default: return LexerError{"Unrecognized token", codePtr.line, codePtr.col};
		}
	}
}

static void SkipWhitespace(CodePtr& ptr)
{
	while (true)
	{
		const char c = ptr[0];
		switch (c)
		{
		case '\t':
		case '\r':
		case ' ':
			ptr += 1;
			break;
		default:
			return;
		}
	}
}

static bool IsIdentifierStart(const char c)
{
	return (c >= 'A' && c <= 'Z')
		|| c == '_'
		|| (c >= 'a' && c <= 'z');
}

static bool IsIdentifierMiddle(const char c)
{
	return (c >= '0' && c <= '9')
		|| (c >= 'A' && c <= 'Z')
		|| c == '_'
		|| (c >= 'a' && c <= 'z');
}

static bool IsDigit(const char c)
{
	return c >= '0' && c <= '9';
}

static LexerError LexComment(CodePtr& ptr, Token& out)
{
	if (ptr[0] != '/' || ptr[1] != '/')
	{
		success = false;
		return LexerError{};
	}

	const size_t line = ptr.line;
	const size_t col = ptr.col;

	ptr += 2;
	SkipWhitespace(ptr);

	const char* const commentStart = &ptr[0];
	size_t commentLength = 0;

	while (true)
	{
		switch (ptr[0])
		{
		case '\0':
		case '\n':
			out.tag = TokenTag::Comment;
			out.text = std::string_view{commentStart, commentLength};
			out.line = line;
			out.col = col;
			success = true;
			return LexerError{};
		default:
			ptr += 1;
			commentLength += 1;
			continue;
		}
	}
}

static LexerError LexIdentifierOrKeyword(CodePtr& ptr, Token& out)
{
	if (!IsIdentifierStart(ptr[0]))
	{
		success = false;
		return LexerError{};
	}

	const size_t line = ptr.line;
	const size_t col = ptr.col;

	const char* const start = &ptr[0];

	size_t length = 0;
	do
	{
		length += 1;
		ptr += 1;
	} while (IsIdentifierMiddle(ptr[0]));

	const std::string_view text{start, length};

	// keyword

	for (size_t i = 0; i < KEYWORD_COUNT; ++i)
	{
		if (text != KEYWORDS[i]) continue;

		success = true;
		out = Token{static_cast<TokenTag>(i), line, col};
		return LexerError{};
	}

	// identifier

	out.tag = TokenTag::Identifier;
	out.text = text;
	out.line = line;
	out.col = col;
	success = true;
	return LexerError{};
}

static LexerError LexNumber(CodePtr& ptr, Token& out)
{
	if (!IsDigit(ptr[0]))
	{
		success = false;
		return LexerError{};
	}

	const size_t line = ptr.line;
	const size_t col = ptr.col;

	const char* const start = &ptr[0];

	bool hasDot = false;
	ptr += 1;

	while (true)
	{
		const char c = ptr[0];
		if (c == '.')
		{
			if (hasDot) break;

			ptr += 1;
			hasDot = true;
		}

		if (!IsDigit(ptr[0])) break;
		ptr += 1;
	}

	out.tag = TokenTag::Number;
	out.value = atof(start);
	out.line = line;
	out.col = col;
	success = true;
	return LexerError{};
}
