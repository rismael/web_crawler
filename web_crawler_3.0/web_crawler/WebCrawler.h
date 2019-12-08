/*
Name: Ismael Rodriguez
Class: CSCE 463-500
Semester: Fall 2019
*/
#pragma once
#include <vector>

class WebCrawler {
public:
	WebCrawler();

	~WebCrawler();

	int start_robots(sockaddr_in server, URLParser* url_parser);

	int start_page(sockaddr_in server, URLParser* url_parser, char* url, int arg);

	//Append arrays with one another by creating a new block of memory.
	//Then copying the passed arrays into the new block
	//The bool variable is used for memory management
	//Returns the new block containing the appanded arrays
	char* append_char_arrays(char* first_array, char* second_array, bool prev_used);

	bool valid_header(char* buffer);

	char* get_status_code(char* recv_buf, char* status_array);

	void print_http_header(char* recv_buf);

	unsigned long socket_receive(SOCKET sock, char** recv_buf, int recv_buf_length, char** start_of_buf, int buf_limit);

	void socket_send(SOCKET sock, char* request);

	int* get_links();

	int* get_http_codes();

	int get_bytes_downloaded();

	int get_tamu_links();

	int get_outside_tamu_links();

	void add_to_memory(char* array);

private:

	int links = 0;
	int bytes = 0;
	int tamu_links = 0;
	int outside_tamu = 0;
	char* robot_recv_buf, * recv_buf, * url;
	SOCKET sock, sock_robot;
	std::vector <char*> memory_list;
	//http_codes, 0th index is 2xx, 1st index is 3xx, 2nd index is 4xx, 3rd index is 5xx, and 4th index is other
	int http_codes[5] = { 0, 0, 0, 0, 0 };
};