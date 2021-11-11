#pragma once

#include <cctype>
#include <ostream>

#include "ast/nodes.h"
#include "lexer/buffer.h"
#include "lexer/token.h"
#include "utilities/trie.h"

#include "dtypes.h"

#include <iostream>

/*
 *  Lexer is a stream of tokens
 *
 *      TODO:   DocString support
 */

namespace lython {

struct OpConfig {
    int            precedence       = -1;
    bool           left_associative = true;
    TokenType      type;
    BinaryOperator binarykind = BinaryOperator::None;
    UnaryOperator  unarykind  = UnaryOperator::None;
    BoolOperator   boolkind   = BoolOperator::None;
    CmpOperator    cmpkind    = CmpOperator::None;

    String __str__() const {
        StringStream ss;
        ss << to_string(type) << "(pred: " << precedence << ") "
           << "(binary: " << int(binarykind) << ") "
           << "(unary: " << int(unarykind) << ") "
           << "(bool: " << int(boolkind) << ") "
           << "(cmp: " << int(cmpkind) << ") ";

        return ss.str();
    }
};

Dict<String, OpConfig> const &default_precedence();

class LexerOperators {
    public:
    LexerOperators() {
        for (auto &c: _precedence_table) {
            _operators.insert(c.first);
        }
    }

    Trie<128> const *match(int c) const { return _operators.trie().matching(c); }

    Dict<String, OpConfig> const &precedence_table() const { return _precedence_table; }

    TokenType token_type(String const &str) const { return _precedence_table.at(str).type; }

    private:
    CoWTrie<128>           _operators;
    Dict<String, OpConfig> _precedence_table = default_precedence();
};

class AbstractLexer {
    public:
    virtual ~AbstractLexer() {}

    virtual Token const &next_token() = 0;

    virtual Token const &peek_token() = 0;

    virtual Token const &token() = 0;

    // print tokens with their info
    std::ostream &debug_print(std::ostream &out);

    // print out tokens as they were inputed
    std::ostream &print(std::ostream &out);

    // extract a token stream into a token vector
    Array<Token> extract_token() {
        Array<Token> v;

        Token t = next_token();
        do {
            v.push_back(t);
        } while ((t = next_token()));

        v.push_back(t); // push eof token
        return v;
    }
};

class ReplayLexer: public AbstractLexer {
    public:
    ReplayLexer(Array<Token> &tokens): tokens(tokens) {}

    Token const &next_token() override final {
        if (i + 1 < tokens.size())
            i += 1;

        return tokens[i];
    }

    Token const &peek_token() override final {
        auto n = i + 1;

        if (n >= tokens.size())
            n = i;

        return tokens[n];
    }

    Token const &token() override final { return tokens[i]; }

    ~ReplayLexer() {}

    private:
    std::size_t   i = 0;
    Array<Token> &tokens;
};

class Lexer: public AbstractLexer {
    public:
    Lexer(AbstractBuffer &reader): _reader(reader), _cindent(indent()), _oindent(indent()) {}

    ~Lexer() {}

    Token const &token() override final { return _token; }
    Token const &next_token() override final;
    Token const &peek_token() override final {
        // we can only peek ahead once
        if (_buffered_token)
            return _buffer;

        // Save current token a get next
        Token current_token = _token;
        _buffer             = next_token();
        _token              = current_token;
        _buffered_token     = true;
        return _buffer;
    }

    Token const &make_token(int8 t) {
        _token = Token(t, line(), col());
        return _token;
    }

    Token const &make_token(int8 t, const String &identifier) {
        _token              = Token(t, line(), col());
        _token.identifier() = identifier;
        return _token;
    }

    private:
    AbstractBuffer &_reader;
    Token           _token{dummy()};
    int32           _cindent;
    int32           _oindent;
    bool            _buffered_token = false;
    Token           _buffer{dummy()};
    LexerOperators  _operators;

    // shortcuts
    const String &file_name() { return _reader.file_name(); }
    int32         line() { return _reader.line(); }
    int32         col() { return _reader.col(); }
    int32         indent() { return _reader.indent(); }
    void          consume() { return _reader.consume(); }
    char          peek() { return _reader.peek(); }
    bool          empty_line() { return _reader.empty_line(); }

    char nextc() {
        _reader.consume();
        return _reader.peek();
    }

    // what characters are allowed in identifiers
    bool is_identifier(char c) {
        if (std::isalnum(c) || c == '_' || c == '?' || c == '!' || c == '-')
            return true;
        return false;
    }
};

} // namespace lython
