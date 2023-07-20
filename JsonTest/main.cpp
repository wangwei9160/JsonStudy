#include <iostream>
#include "MyJsonParse.h"
#include <format>
using namespace MyJson;

void test_parse_null(std::string_view s)
{
	std::cout << std::format("null\tParse Json : {}\t", s);
	auto x = ParseJson(s);
	if (x.has_value())
	{
		std::cout << std::format("parse ok \n");
	}
	else {
		std::cout << "parse error\n";
	}
}

void test_parse_true(std::string_view s)
{
	std::cout << std::format("true\tParse Json : {}\t", s);
	auto x = ParseJson(s);
	if (x.has_value() && std::get<MyBool>(x.value().value) )
	{
		std::cout << std::format("parse ok \n");
	}
	else {
		std::cout << "parse error\n";
	}
}

void test_parse_false(std::string_view s)
{
	std::cout << std::format("false\tParse Json : {}\t", s);
	auto x = ParseJson(s);
	if (x.has_value() && !std::get<MyBool>(x.value().value))
	{
		std::cout << std::format("parse ok \n");
	}
	else {
		std::cout << "parse error\n";
	}
}

void test_parse_string(std::string_view s)
{
	std::cout << std::format("string\tParse Json : {}\t", s);
	auto x = ParseJson(s);
	if (x.has_value())
	{
		std::cout << std::format("parse ok \n");
	}
	else {
		std::cout << "parse error\n";
	}
}

void test_parse_array(std::string_view s)
{
	std::cout << std::format("string\tParse Json : {}\t", s);
	auto x = ParseJson(s);
	if (x.has_value())
	{
		std::cout << std::format("parse ok \n");
	}
	else {
		std::cout << "parse error\n";
	}
}

void test_parse_object(std::string_view s)
{
	std::cout << std::format("string\tParse Json : {}\t", s);
	auto x = ParseJson(s);
	if (x.has_value())
	{
		std::cout << std::format("parse ok \n");
	}
	else {
		std::cout << "parse error\n";
	}
}

void test_string()
{
	test_parse_string("\"Hello\"");
	test_parse_string("hello");
	test_parse_string("\"\\\"");
	test_parse_string("\"\\\"");
	test_parse_string("\\\"");
}

void test_array()
{
	test_parse_array("[1,2,3]");
	test_parse_array("[1.1,2.2,3.3]");
	test_parse_array("[1,2.2,\"3\"]");
}

void test_object()
{
	test_parse_array("{\"a\":1,\"b\":[1,2,3]}");
	test_parse_array("{,,}");
	test_parse_array("{:1}");
	test_parse_array("{\"a\":}");
}	

void test_null()
{
	test_parse_null("null");
	test_parse_null("nul");
}

void test_true()
{
	test_parse_true("true");
	test_parse_true("false");
}

void test_false()
{
	test_parse_false("true");
	test_parse_false("false");
}

void test_parser()
{
	test_null();
	test_true();
	test_false();
	test_string();
	test_array();
	test_object();
}

#define GENERATE(a) do {\
	std::string ss{ a };\
	std::cout << std::format("Before toJson : {} \t" , ss);\
	auto x = ParseJson(ss).value();\
	std::string sss{toJson(x)};\
	std::cout << toJson(x) << "\t";\
	auto y = ParseJson(sss).value();\
	std::cout << toJson(y) << '\n';\
}while(0)

void test_generate()
{
	GENERATE("  null  ");
	GENERATE("true  ");
	GENERATE("false ");
	GENERATE("123 ");
	GENERATE("\"123456\"");
	GENERATE("123.456");
	GENERATE("[1  ,   2   ,  3]");
	GENERATE("[   1.1       ,  2.2  ,  3.3]");
}

int main()
{
	test_parser();
	test_generate();
	return 0;
}