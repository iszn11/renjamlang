#pragma once

#include "CodePos.h"
#include "Error.h"

#include <cstddef>
#include <memory>
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

	Eof,
};

enum class CommentNodeTag {
	Text,
	Expression,
};

struct Token {
	TokenTag tag;
	CodePos pos;

	Token(const TokenTag tag, const CodePos pos) : tag{tag}, pos{pos} {}
	virtual ~Token() = default;
};

struct NumberToken : public Token {
	double value;

	NumberToken(const double value, const CodePos pos) : Token{TokenTag::Number, pos}, value{value} {}
};

struct IdentifierToken : public Token {
	std::string name;

	IdentifierToken(std::string name, const CodePos pos) : Token{TokenTag::Identifier, pos}, name{std::move(name)} {}
};

struct CommentNode;

struct CommentToken : public Token {
	std::vector<std::unique_ptr<CommentNode>> nodes;

	CommentToken(std::vector<std::unique_ptr<CommentNode>> nodes, const CodePos pos) : Token{TokenTag::Comment, pos}, nodes{std::move(nodes)} {}
};

struct CommentNode {
	CommentNodeTag tag;

	CommentNode(const CommentNodeTag tag) : tag{tag} {}
	virtual ~CommentNode() = 0;
};

inline CommentNode::~CommentNode() {}

struct CommentTextNode : public CommentNode {
	std::string text;

	CommentTextNode(std::string text) : CommentNode{CommentNodeTag::Text}, text{text} {}
};

struct CommentExpressionNode : public CommentNode {
	std::vector<std::unique_ptr<Token>> tokens;

	CommentExpressionNode(std::vector<std::unique_ptr<Token>> tokens) : CommentNode{CommentNodeTag::Expression}, tokens{std::move(tokens)} {}
};

[[nodiscard]] Error Lex(const char* code, std::vector<std::unique_ptr<Token>>& tokens);
