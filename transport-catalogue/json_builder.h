#include "json.h"
#include <vector>
#include <string>


namespace json {

class Builder {
public:
	class BaseContext;
	class DictValueContext;
	class DictItemContext;
	class ArrayItemContext;
	DictValueContext Key(std::string key);
	virtual BaseContext Value(Node::Value value);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
	Builder& EndDict();
	Builder& EndArray();
	Node Build();
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
		DictItemContext Value(Node::Value value);
		DictValueContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	private:
		using BaseContext::BaseContext;
	};

	class DictItemContext : public BaseContext {
	public:
		BaseContext Value(Node::Value value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	private:
		using BaseContext::BaseContext;
	};

	class ArrayItemContext : public BaseContext {
	public:
		ArrayItemContext Value(Node::Value value);
		DictValueContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Node Build() = delete;
	private:
		using BaseContext::BaseContext;
	};

private:
	Node root_;
	std::vector<Node*> nodes_stack_;
	std::string dict_key_{};
	bool in_use{ false };
	bool key_set{ false };
	template <class T>
	void StartContainer(T type);
};


}