#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

enum class TokenTag {
	KeyVoid,
	KeyIf,
	KeyElif,
	KeyElse,
	KeyWhile,
	KeyEnd,
	KeyFn,
	KeyReturn,
	KeyPush,
	KeyPop,
	KeyNot,
	KeyAnd,
	KeyOr,
	KeyXor,
	KeyNeg,
	KeyFalse,
	KeyTrue,

	Newline,

	Comma,         // ,
	BracketOpen,   // [
	BracketClose,  // ]
	ParenOpen,     // (
	ParenClose,    // )

	Plus,          // +
	Minus,         // -
	Star,          // *
	Slash,         // /
	Percent,       // %

	Equals,        // =
	LessThan,      // <
	GreaterThan,   // >
	LessEquals,    // <=
	GreaterEquals, // >=
	EqualsEquals,  // ==
	NotEquals,     // !=

	At,            // @
	Hash,          // #

	Number,
	Identifier,
	Comment,
};

struct LexerError {
	bool error;
	std::string message;
	size_t line;
	size_t col;

	LexerError() : error{false}, message{}, line{}, col{} {}
	LexerError(std::string message, size_t line, size_t col) : error{true}, message{std::move(message)}, line{line}, col{col} {}

	operator bool() const { return error; }
};

struct Token {
	TokenTag tag;
	union {
		double value;
		std::string_view text;
	};
	size_t line;
	size_t col;

	Token(): tag{}, value{}, line{}, col{} {}
	Token(const TokenTag tag, const size_t line, const size_t col) : tag{tag}, line{line}, col{col} {}
};

LexerError Lex(const char* code, std::vector<Token>& tokens);
