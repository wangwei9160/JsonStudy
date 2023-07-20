#pragma once
#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <assert.h>
#include <string_view>
#include <optional>
#include <type_traits>
namespace MyJson {

	struct JsonBase;
	using MyNull = std::monostate; // 非可默认构造的 variant 可以列 std::monostate 为其首个可选项：这使得 variant 自身可默认构造。
	using MyBool = bool;
	using MyInt = long long;
	using MyDouble = double;
	using MyString = std::string;
	using MyArray = std::vector<JsonBase>;
	using MyObject = std::unordered_map<std::string, JsonBase >;
	using MyValue = std::variant<MyNull, MyBool, MyInt, MyDouble, MyString, MyArray, MyObject>;

	struct JsonBase
	{
		MyValue value{};

		JsonBase() :value{ NULL } {}
		JsonBase(MyValue tmp) : value(tmp) {}

		// 如果是MyArray 可以使用[](index)
		auto operator[](int idx)
		{
			assert(std::holds_alternative<MyArray>(value));
			auto arr = std::get_if<MyArray>(&value);
			return arr->at(idx);
		}

		// 如果是MyObject 可以使用[](std::string)
		auto operator[](std::string key)
		{
			assert(std::holds_alternative<MyObject>(value));
			auto obj = std::get_if<MyObject>(&value);
			return obj->at(key);
		}

	};


	struct MyJsonParse
	{
		std::string_view str;
		size_t len{ 0 };
		size_t pos{ 0 };

		inline void parse_whitespace()
		{
			while (pos < len && std::isspace(str[pos]))
			{
				pos++;
			}
		}

		auto parse_literal(std::string_view strView) -> std::optional<MyValue>
		{
			size_t strLen = strView.size();
			if (pos + strLen > len) return {};
			for (size_t i{ 0 }; i < strLen; i++)
			{
				if (str[pos + i] != strView[i])
				{
					return {};
				}
			}
			pos = pos + strLen - 1;
			if (strView.starts_with("null")) return MyNull{};
			else if (strView.starts_with("true")) return true;
			else if (strView.starts_with("false")) return false;
			return {};
		}

		auto parse_string() -> std::optional<MyValue>
		{
			pos++; // "
			std::string ret = "";
			while (pos < len && str[pos] != '\"')
			{
				ret += str[pos];
				pos++;
			}
			if (str[pos++] != '\"') return {};
			return ret;
		}

		auto parse_array() -> std::optional<MyValue>
		{
			pos++; // [
			MyArray arr;
			while (pos < len && str[pos] != ']')
			{
				parse_whitespace();
				auto val = parse_value();
				if (val.has_value())
					arr.push_back(val.value());
				else return {};
				parse_whitespace();
				if (pos < len && str[pos] == ',')
					pos++;
			}
			if (str[pos] != ']') return {};
			pos++; // ]
			return arr;
		}

		auto parse_object() -> std::optional<MyValue>
		{
			pos++;// {
			MyObject obj;
			while (pos < len && str[pos] != '}')
			{
				parse_whitespace();
				auto key = parse_value();
				if (!key.has_value() || !std::holds_alternative<MyString>(key.value()))
				{
					return {};
				}
				parse_whitespace();
				if (str[pos] == ':')
				{
					pos++;
				}
				else return {};
				auto val = parse_value();
				if (!val.has_value()) return {};
				obj[std::get<MyString>(key.value())] = val.value();
				parse_whitespace();
				if (pos < len && str[pos] == ',')
					pos++;
			}
			pos++; // }
			return obj;
		}

		auto parse_number() -> std::optional<MyValue>
		{
			size_t endPos = pos;
			while (endPos < len && (std::isdigit(str[endPos]) || str[endPos] == 'e' || str[endPos] == '.'))
			{
				endPos++;
			}
			std::string tmpStr{ str.substr(pos , endPos - pos) };
			pos = endPos;
			auto can2Double = [&]()
			{
				return tmpStr.find('.') != tmpStr.npos || tmpStr.find('e') != tmpStr.npos;
			};

			if (can2Double())
			{
				try
				{
					MyDouble myDouble = std::stod(tmpStr);
					return myDouble;
				}
				catch (const std::exception&)
				{
					return {};
				}
			}
			else {
				try
				{
					MyInt myInt = std::stoi(tmpStr);
					return myInt;
				}
				catch (const std::exception&)
				{
					return {};
				}
			}
			return {};
		}

		auto parse_value() -> std::optional<MyValue>
		{
			parse_whitespace();
			switch (str[pos])
			{
			case 'n':
				return parse_literal("null");
			case 't':
				return parse_literal("true");
			case 'f':
				return parse_literal("false");
			case '"':
				return parse_string();
			case '[':
				return parse_array();
			case '{':
				return parse_object();
			default:
				return parse_number();
			}
		}

		auto parse() -> std::optional<JsonBase>
		{
			parse_whitespace();
			auto ret = parse_value();
			if (!ret.has_value())
			{
				return {};
			}
			return JsonBase{ ret.value() };
		}

	};

	auto ParseJson(std::string_view strView) -> std::optional<JsonBase>
	{
		// 解析时不会修改字符串的内容 std::string_view 
		MyJsonParse mjp{ strView , strView.size() };
		return mjp.parse();
	}

	struct JsonGenerator
	{

		static auto generate(const JsonBase& obj) -> std::string
		{
			return std::visit([](auto&& arg) -> std::string {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, MyNull>)
				{
					return "null";
				}
				else if constexpr (std::is_same_v<T, MyBool>)
				{
					return arg ? "true" : "false";
				}
				else if constexpr (std::is_same_v<T, MyInt>)
				{
					return std::to_string(arg);
				}
				else if constexpr (std::is_same_v<T, MyDouble>)
				{
					return std::to_string(arg);
				}
				else if constexpr (std::is_same_v<T, MyString>)
				{
					return generate_string(arg);
				}
				else if constexpr (std::is_same_v < T, MyArray >)
				{
					return generate_array(arg);
				}
				return "";
				}, obj.value);

		}

		static auto generate_string(const MyString& str) -> std::string
		{
			std::string ret = "\"";
			ret += str;
			ret += "\"";
			return ret;
		}
		static auto generate_array(const MyArray& arr) -> std::string
		{
			std::string ret = "[";
			for (bool f = false; const auto x : arr)
			{
				if (f) ret += ",";
				ret += generate(x);
				f = true;
			}
			ret += "]";
			return ret;
		}
		static auto generate_array(const MyObject& obj) -> std::string
		{
			std::string ret = "{";

			for (bool f = false; const auto & [key, val] : obj)
			{
				if (f) ret += ",";
				ret += generate_string(key);
				ret += ":";
				ret += generate(val);
				f = true;
			}
			ret += "}";
			return ret;
		}

	};
	auto toJson(JsonBase obj) -> std::string
	{
		return JsonGenerator::generate(obj);
	}
}