#include <string>
#include <vector>

#include "main/tokens.h"
#include "test/parenAst.h"

ParenNode::ParenNode(std::vector<ParenNode*>* children, std::string* name) :
    children(children), name(name) {};

ParenNode* ParenNode::createVector(std::vector<ParenNode*>* children) {
    return new ParenNode(children, nullptr);
}

ParenNode* ParenNode::createString(std::string* name) {
    return new ParenNode(nullptr, name);
}

ParenNodeType ParenNode::type() {
    if(this->children != nullptr) {
        return NODE_VECTOR;
    } else {
        return NODE_STRING;
    }
}

std::string* ParenNode::asStr() {
    if(this->type() == NODE_VECTOR) {
        std::string* concat = new std::string();
        concat->append("( ");
        auto items = this->children;
        for(auto iter = items->begin(); iter != items->end(); iter++) {
            concat->append(*(*iter)->asStr());
            concat->append(" ");
        }
        concat->append(")");
        return concat;
    } else {
        std::string* concat = new std::string();
        concat->append("'");
        concat->append(*this->name);
        concat->append("'");
        return concat;
    }
}

class ParenParseState {
private:
    const std::string* src;
    uint32_t index;
public:
    ParenParseState(const std::string* src, uint32_t index) :
        src(src), index(index) {}

    char getChar() {
        return this->src->at(this->index);
    }

    void nextChar() {
        this->index++;
    }
};

class ParenParseRet {
public:
    ParenParseState state;
    ParenNode* node;

    ParenParseRet(ParenParseState state, ParenNode* node) :
        state(state), node(node) {}
};

ParenParseState skipWhitespace(ParenParseState state) {
    char c = state.getChar();
    while(c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        state.nextChar();
        c = state.getChar();
    }
    return state;
}

ParenParseRet parseParenNode(ParenParseState state);

ParenParseRet parseParenLiteral(ParenParseState state) {
    if(state.getChar() == '\'') {
        state.nextChar();
    }

    std::string* str = new std::string();
    while(state.getChar() != '\'') {
        if(state.getChar() == '\\') {
            state.nextChar();
            if(state.getChar() == 'n') {
                str->push_back('\n');
            } else if(state.getChar() == 'r') {
                str->push_back('\r');
            } else if(state.getChar() == 't') {
                str->push_back('\t');
            } else if(state.getChar() == '\'') {
                str->push_back('\'');
            }
        } else {
            str->push_back(state.getChar());
        }
        state.nextChar();
    }

    state.nextChar();

    return ParenParseRet(state, ParenNode::createString(str));
}

ParenParseRet parseParenVector(ParenParseState state) {
    if(state.getChar() == '(') {
        state.nextChar();
    }

    std::vector<ParenNode*>* children = new std::vector<ParenNode*>();

    while(state.getChar() != ')') {
        state = skipWhitespace(state);
        ParenParseRet ret = parseParenNode(state);
        state = ret.state;
        children->push_back(ret.node);
        state = skipWhitespace(state);
    }

    return ParenParseRet(state, ParenNode::createVector(children));
}

ParenParseRet parseParenSymbol(ParenParseState state) {
    std::string* str = new std::string();

    char c = state.getChar();
    while(c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '(' &&
            c != ')') {
        str->push_back(c);
        state.nextChar();
        c = state.getChar();
    }

    return ParenParseRet(state, ParenNode::createString(str));
}

ParenParseRet parseParenNode(ParenParseState state) {
    state = skipWhitespace(state);

    if(state.getChar() == '\'') {
        return parseParenLiteral(state);
    } else if (state.getChar() == '(') {
        return parseParenVector(state);
    } else {
        return parseParenSymbol(state);
    }
}

ParenNode* parseParenNode(const std::string* src) {
    return parseParenNode(ParenParseState(src, 0)).node;
}
