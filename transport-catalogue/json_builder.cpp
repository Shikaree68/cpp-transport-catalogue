#include "json_builder.h"
#include <memory>
using namespace std::string_literals;
namespace json {
	Builder::DictValueContext Builder::Key(std::string key) {
	if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && !key_set) {
		dict_key_ = std::move(key);
		key_set = true;
	}
	else {
		throw std::logic_error("previos is not a StartDict"s);
	}
	return *this;
}

Builder::BaseContext Builder::Value(Node::Value value) {
	//если стэк пуст то принимаем все содержимое?
	if (nodes_stack_.empty() && !in_use) {
		root_ = std::move(value);
		in_use = true;
	}else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
		const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(std::move(value));
	}
	else if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && key_set) {
		//добавляем в словарь пару ключ - значение
		const_cast<Dict&>(nodes_stack_.back()->AsDict()).emplace(std::move(dict_key_), std::move(value));
		key_set = false;
	}
	else {
		throw std::logic_error("previos is not correct"s);
	}
	return BaseContext(*this);
}

Builder::DictItemContext Builder::StartDict() {
	StartContainer(Dict());
	return  *this;
}

Builder::ArrayItemContext Builder::StartArray() {
	StartContainer(Array());
	return *this;
}
template <class T>
void Builder::StartContainer(T) {
	
	if (nodes_stack_.empty() && !in_use) {
		root_ = T();
		nodes_stack_.push_back(&root_);
		in_use = true;
	}
	else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
		const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(T());
		nodes_stack_.push_back(&const_cast<Node&>(nodes_stack_.back()->AsArray().back()));
	}
	else if (!nodes_stack_.empty() && key_set && nodes_stack_.back()->IsDict()) {
		//добавляем в словарь пару ключ - значение
		const_cast<Dict&>(nodes_stack_.back()->AsDict()).emplace(dict_key_, T());
		nodes_stack_.push_back(&const_cast<Node&>(nodes_stack_.back()->AsDict().at(dict_key_)));
		key_set = false;
	}
	else {
		throw std::logic_error("previos is not correct"s);
	}
}

Builder& Builder::EndDict() {
	if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
		throw std::logic_error("previos is not a StartDict"s);
	}
	nodes_stack_.pop_back();
	return *this;
}

Builder& Builder::EndArray() {
	if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
		throw std::logic_error("previos is not an StartArray"s);
	}
	nodes_stack_.pop_back();
	return *this;
}

Node Builder::Build() {
	if (!nodes_stack_.empty() || !in_use || key_set) {
		throw std::logic_error("not all Starts Ended"s);
	}
	return root_;
}
Builder::DictValueContext Builder::BaseContext::Key(std::string key) {
	return builder_.Key(key);
}
Builder::BaseContext Builder::BaseContext::Value(Node::Value value) {
	return builder_.Value(value);
}
Builder::DictItemContext Builder::BaseContext::StartDict() {
	return builder_.StartDict();
}
Builder::ArrayItemContext Builder::BaseContext::StartArray() {
	return builder_.StartArray();
}
Builder& Builder::BaseContext::EndDict() {
	return builder_.EndDict();
}
Builder& Builder::BaseContext::EndArray() {
	return builder_.EndArray();
}
Node Builder::BaseContext::Build() {
	return builder_.Build();
}
Builder::DictItemContext Builder::DictValueContext::Value(Node::Value value){
	BaseContext temp{ builder_.Value(value) };
	return static_cast<DictItemContext&>(temp);
}
Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
	BaseContext temp{ builder_.Value(value) };
	return static_cast<ArrayItemContext&>(temp);

}
}