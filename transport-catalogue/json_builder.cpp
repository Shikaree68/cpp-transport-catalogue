#include "json_builder.h"
#include <memory>
using namespace std::string_literals;
namespace json {
DictValueContext Builder::Key(std::string key) {
	if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && !key_set) {
		dict_key_ = std::move(key);
		key_set = true;
	}
	else {
		throw std::logic_error("previos is not a StartDict"s);
	}
	return *this;
}

BaseContext Builder::Value(Node::Value value) {
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

DictItemContext Builder::StartDict() {
	if (nodes_stack_.empty() && !in_use) {
		root_ = Dict();
		nodes_stack_.push_back(&root_);
		in_use = true;
	}
	else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
		const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(Dict());
		nodes_stack_.push_back(&const_cast<Node&>(nodes_stack_.back()->AsArray().back()));
	}
	else if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && key_set) {
		//добавляем в словарь пару ключ - значение
		const_cast<Dict&>(nodes_stack_.back()->AsDict()).emplace(dict_key_, Dict());
		nodes_stack_.push_back(&const_cast<Node&>(nodes_stack_.back()->AsDict().at(std::move(dict_key_))));
		key_set = false;
	}
	else {
		throw std::logic_error("previos is not correct"s);
	}
	return  *this;
}

ArrayItemContext Builder::StartArray() {
	if (nodes_stack_.empty() && !in_use) {
		root_ = Array();
		nodes_stack_.push_back(&root_);
		in_use = true;
	}
	else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
		const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(Array());
		nodes_stack_.push_back(&const_cast<Node&>(nodes_stack_.back()->AsArray().back()));
	} else if (!nodes_stack_.empty() && key_set && nodes_stack_.back()->IsDict()) {
		//добавляем в словарь пару ключ - значение
		const_cast<Dict&>(nodes_stack_.back()->AsDict()).emplace(dict_key_, Array());
		nodes_stack_.push_back(&const_cast<Node&>(nodes_stack_.back()->AsDict().at(dict_key_)));
		//dict_key_ = ""s;
		key_set = false;
	} else {
		throw std::logic_error("previos is not correct"s);
	}
	return *this;
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
DictValueContext BaseContext::Key(std::string key) {
	return builder_.Key(key);
}
BaseContext BaseContext::Value(Node::Value value) {
	return builder_.Value(value);
}
DictItemContext BaseContext::StartDict() {
	return builder_.StartDict();
}
ArrayItemContext BaseContext::StartArray() {
	return builder_.StartArray();
}
Builder& BaseContext::EndDict() {
	return builder_.EndDict();
}
Builder& BaseContext::EndArray() {
	return builder_.EndArray();
}
Node BaseContext::Build() {
	return builder_.Build();
}
DictItemContext DictValueContext::Value(Node::Value value){
	BaseContext temp{ builder_.Value(value) };
	return static_cast<DictItemContext&>(temp);
}
ArrayItemContext ArrayItemContext::Value(Node::Value value) {
	BaseContext temp{ builder_.Value(value) };
	return static_cast<ArrayItemContext&>(temp);
}

}