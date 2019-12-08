/*
Name: Ismael Rodriguez
Class: CSCE 463-500
Semester: Fall 2019
*/
#include "stdafx.h"

URLParser::URLParser() : port_number(NULL), scheme(NULL), host(NULL), request(NULL), path(NULL), query(NULL), fragment(NULL) {};

URLParser::URLParser(char* url, int arg) : port_number(NULL), scheme(NULL), host(NULL), request(NULL), path(NULL), query(NULL), fragment(NULL)
{
	//printf("URL: %s\n", url);
	
	argc = arg;
	parse_url(url);
}

//destructor, free allocated memory
URLParser::~URLParser()
{
	if (host != NULL) free(host);
	if (port_number != NULL) free(port_number);
	if (request != NULL) free(request);
	if (path != NULL) free(path);
	if (query != NULL) free(query);
	if (fragment != NULL) free(fragment);
}

//parse url
void URLParser::parse_url(char* baseURL)
{
	//printf("\tParsing URL... ");
	find_scheme(baseURL);
	find_host(baseURL);
	find_port(baseURL);
	find_request(baseURL);
	find_path(baseURL);
	find_query(baseURL);
	find_fragment(baseURL);
	//if (argc == 2) printf("host %s, port %s, request %s\n", host, port_number, request);
	//else printf("host %s, port %s\n", host, port_number);
}

//identify shceme
void URLParser::find_scheme(char* url)
{
	char default_scheme[] = "http://";
	if (strstr(url, default_scheme) == url)
	{
		scheme = (char*)"http://";
	}
	else
	{
		printf("failed with invalid scheme\n");
		exit(-1);
	}
}

//identify host
void URLParser::find_host(char* url)
{
	char* str;
	if (strchr(url, ':'))
	{
		str = strchr(url, ':');
		//printf("Finding host...\n");
		int i = 0;
		while (true) {
			if ((str[i] == ':') || (str[i] == '/')) {
				str += 1;	//traverse string
			}
			else
			{
				break;
			}
		}
		host = parse_host(str);
	}
	else
	{
		printf("Invalid URL\n");
		exit(-1);
	}
}

//identify host
void URLParser::find_port(char* url)
{
	char* str;
	if (strchr(url, ':'))
	{
		str = strchr(url, ':');
		//printf("Finding host...\n");
		int i = 0;
		while (true) {
			if ((str[i] == ':') || (str[i] == '/')) {
				str += 1;	//traverse string
			}
			else
			{
				break;
			}
		}
		i = 0;
		int starting_index = 0;
		int ending_index = 0;
		while (true)
		{
			if (str[i] == '/') break;
			else if (str[i] == '?') break;
			else if (str[i] == '#') break;
			else if (str[i] == '\0') break;
			else
			{
				ending_index = i;
				i++;
			}
		}
		int size = i + 1;
		char* sub_str = NULL;
		sub_str = create_array(sub_str, str, size);
		sub_str = strchr(sub_str, ':');
		if (sub_str != NULL)
		{
			port_number = parse_port(sub_str + 1);
			if (*port_number == (int)'0')
			{
				printf("failed with invalid port\n");
				exit(-1);
			}
		}
		else
		{
			int size = 3;
			char* sub_str;
			if ((sub_str = (char*)malloc(size)) == NULL)
			{
				printf("Failed to allocate memory\n");
				exit(-1);
			}
			strncpy_s(sub_str, size, "80", _TRUNCATE);
			port_number = sub_str;
		}
	}
	else
	{
		//printf(": not present in URL\n");		//Check out later
		return;
	}
	return;
}

//identify requeset
void URLParser::find_request(char* url)
{
	char* str;
	if ((str = strchr(url, ':')) == NULL)
	{
		printf("Invalid URL");
		exit(-1);
	}
	else {
		int i = 0;
		while (true)
		{
			if (str[i] == '/' || str[i] == ':')
			{
				str += 1;
			}
			else
			{
				break;
			}
		}
		request = parse_request(str);
	}
}

//identify path
void URLParser::find_path(char* url)
{
	char* str;
	if ((str = strchr(url, ':')) == NULL)
	{
		printf("Invalid URL");
		exit(-1);
	}
	else {
		int i = 0;
		while (true)
		{
			if (str[i] == '/' || str[i] == ':')
			{
				str += 1;
			}
			else
			{
				break;
			}
		}
		path = parse_path(str);
	}
}

//identify query
void URLParser::find_query(char* url)
{
	char* str;
	if ((str = strchr(url, '?')) == NULL)
	{
		//printf("No query\n");
	}
	else
	{
		query = parse_query(str);
	}
}

//identify fragment
void URLParser::find_fragment(char* url)
{

	char* str;
	if ((str = strchr(url, '#')) == NULL)
	{
		//printf("No fragment\n");
	}
	else
	{
		fragment = parse_fragment(str);
	}
}

//parse host from url
char* URLParser::parse_host(char* url)
{
	int i = 0;
	int starting_index = 0;
	int ending_index = 0;
	while (true)
	{
		if (url[i] == '/') break;
		else if (url[i] == ':') break;
		else if (url[i] == '?') break;
		else if (url[i] == '#') break;
		else if (url[i] == '\0') break;
		else
		{
			ending_index = i;
			i++;
		}
	}
	int size = i + 1;
	char* sub_str = NULL;
	sub_str = create_array(sub_str, url, size);
	//printf("Host Name: %s\n", sub_str);
	return sub_str;
}

//parse port from url
char* URLParser::parse_port(char* url)
{
	int lower_bound = (int) '0';
	int upper_bound = (int) '9';
	int i = 0;	//iterator for while loop
	int end_of_number = 0; //last index of port

	while (true)
	{
		if ((url[i] >= lower_bound) && (url[i] <= upper_bound))
		{
			end_of_number = i;
			i++;
		}
		else
		{
			if (i == 0)
			{

				printf("failed with invalid port: %s\n", url);
				//exit(-1);
				
				char default_port[] = "80";
				return default_port;
				
			}
			break;
		}
	}
	int size = i + 1;
	char* sub_str = NULL;
	sub_str = create_array(sub_str, url, size);
	//printf("Port Number: %s\n", sub_str);
	return sub_str;
}

//parse request from url
char* URLParser::parse_request(char* url)
{
	char* str = url;
	if ((str = strchr(str, '/')) == NULL)
	{
		int size = 2;
		char* sub_str;
		if ((sub_str = (char*)malloc(size)) == NULL)
		{
			printf("Failed to allocate memory\n");
			exit(-1);
		}
		strncpy_s(sub_str, size, "/", _TRUNCATE);
		return sub_str;
	}
	else	//create char array to place request
	{
		int size; //size of characters plus size of null terminator
		if (strchr(str, '#') == NULL) size = strlen(str) + 1;
		else
		{
			char* temp = strchr(str, '#');
			size = strlen(str) - strlen(temp) + 1;
		}
		char* sub_str = NULL;
		sub_str = create_array(sub_str, str, size);
		return sub_str;
		//printf("Request: %s\n", sub_str);
	}
}

//parse path from url
char* URLParser::parse_path(char* url)
{
	char* str = url;
	if ((str = strchr(str, '/')) == NULL)
	{
		//printf("No path found\n");
		return nullptr;
	}
	else	//create char array to place request
	{
		str += 1;
		int i = 0;
		int size = 0;
		while (true)
		{
			if (str[i] == '?' || str[i] == '#' || str[i] == '\0') break;
			size++;
			i++;
		}
		size += 1; //size of characters plus size of null terminator
		char* sub_str = NULL;
		sub_str = create_array(sub_str, str, size);
		//printf("Path: %s\n", sub_str);
		return sub_str;
	}
}

//parse query from url
char* URLParser::parse_query(char* url)
{
	int i = 0;
	int size = 0;
	char* str = url;
	str += 1;
	while (true)
	{
		if (str[i] == '#' || str[i] == '\0') break;
		size++;
		i++;
	}
	size += 1; //size of characters plus size of null terminator
	char* sub_str = NULL;
	sub_str = create_array(sub_str, str, size);
	//printf("Query: %s\n", sub_str);
	return sub_str;
}

//parse fragment from url
char* URLParser::parse_fragment(char* url)
{
	int i = 0;
	int size = 0;
	char* str = url;
	str += 1;
	while (true)
	{
		if (str[i] == '\0') break;
		size++;
		i++;
	}
	size += 1; //size of characters plus size of null terminator
	char* sub_str = NULL;
	sub_str = create_array(sub_str, str, size);
	//printf("Fragment: %s\n", sub_str);
	return sub_str;
}

//retrun host
char* URLParser::get_host()
{
	return host;
}

//return port
char* URLParser::get_port()
{
	return port_number;
}

//return request
char* URLParser::get_request()
{
	return request;
}

//return path
char* URLParser::get_path()
{
	return path;
}

//return query
char* URLParser::get_query()
{
	return query;
}

//return fragment
char* URLParser::get_fragment()
{
	return fragment;
}

//print all member variables
void URLParser::debug()
{
	printf("Scheme: %s\nHost: %s\nPort: %s\nRequest: %s\nPath: %s\nQuery: %s\nFragment: %s\n", scheme, host, port_number, request, path, query, fragment);
}

char* URLParser::create_array(char* new_str, char* old_str, int size)
{
	if ((new_str = (char*)malloc(size)) == NULL)
	{
		printf("Failed to allocate memory\n");
		exit(-1);
	}
	strncpy_s(new_str, size, old_str, _TRUNCATE);
	return new_str;
}

void URLParser::add_to_memory(char* array)
{
	memory_list.push_back(array);
}