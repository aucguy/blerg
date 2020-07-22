#ifndef PARENAST_H_
#define PARENAST_H_

#include <string>
#include <vector>

enum ParenNodeType {
    NODE_VECTOR,
    NODE_STRING
};

class ParenNode {
private:
    ParenNode(std::vector<ParenNode*>* children, std::string* name);
public:
    std::vector<ParenNode*>* children;
    std::string* name;

    static ParenNode* createVector(std::vector<ParenNode*>* children);
    static ParenNode* createString(std::string* name);

    ParenNodeType type();
    std::string* asStr();
};

ParenNode* parseParenNode(const std::string* src);

#endif /* PARENAST_H_ */
