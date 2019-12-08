/*
Name: Ismael Rodriguez
Class: CSCE 463-500
Semester: Fall 2019
*/
#pragma once
#include <vector>

class URLParser
{
//member variables
private:
	char* port_number, *scheme, * host, * request, * path, * query, * fragment;
	std::vector<char*> memory_list;
	int argc;
//functions for finding the respective, parsing, and returning the respective info
public:
	URLParser();

	URLParser(char* url, int arg);

	~URLParser();

	void parse_url(char* url);

	void find_scheme(char* url);

	void find_host(char* url);

	void find_port(char* url);

	void find_request(char* url);

	void find_path(char* url);

	void find_query(char* url);

	void find_fragment(char* url);

	char* parse_host(char* parsed_url);

	char* parse_port(char* parsed_url);

	char* parse_request(char* parsed_url);

	char* parse_path(char* parsed_url);

	char* parse_query(char* parsed_url);

	char* parse_fragment(char* parsed_url);

	char* get_host();

	char* get_port();

	char* get_request();

	char* get_path();

	char* get_query();

	char* get_fragment();

	void debug();

	char* create_array(char* new_str, char* old_str, int size);

	void add_to_memory(char* array);
};

