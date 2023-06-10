#include "json.h"
#include <vector>
#include <string>


namespace json {
class BaseContext;
class DictValueContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
	DictValueContext Key(std::string key);
	virtual BaseContext Value(Node::Value value);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
	Builder& EndDict();
	Builder& EndArray();
	Node Build();

private:
	Node root_;
	std::vector<Node*> nodes_stack_;
	std::string dict_key_{};
	bool in_use{ false };
	bool key_set{ false };
};

class BaseContext {
public:
	BaseContext(Builder& builder) : builder_(builder) {}
	DictValueContext Key(std::string key);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
	BaseContext Value(Node::Value value);
	Builder& EndDict();
	Builder& EndArray();
	Node Build();
protected:	
	Builder& builder_;
};

class DictValueContext : public BaseContext {
public:
	using BaseContext::StartDict;
	using BaseContext::StartArray;
	DictItemContext Value(Node::Value value);
private:
	using BaseContext::Value;
	using BaseContext::BaseContext;
	using BaseContext::Key;
	using BaseContext::EndDict;
	using BaseContext::EndArray;
	using BaseContext::Build;
};

class DictItemContext : public BaseContext {
public:
	using BaseContext::Key;
	using BaseContext::EndDict;
private:
	using BaseContext::BaseContext;
	using BaseContext::Value;
	using BaseContext::StartDict;
	using BaseContext::StartArray;
	using BaseContext::EndArray;
	using BaseContext::Build;
};

class ArrayItemContext : public BaseContext {
public:
	ArrayItemContext Value(Node::Value value);
	using BaseContext::StartDict;
	using BaseContext::StartArray;
	using BaseContext::EndArray;
private:
	using BaseContext::BaseContext;
	using BaseContext::Key;
	using BaseContext::EndDict;
	using BaseContext::Build;
};

}