/*
Name: Ismael Rodriguez
Class: CSCE 463-500
Semester: Fall 2019
*/
#include "stdafx.h"

WebCrawler::WebCrawler() {}

WebCrawler::~WebCrawler() 
{
	free(robot_recv_buf);
	free(recv_buf);
	closesocket(sock_robot);
	closesocket(sock);
}

int WebCrawler::start_robots(sockaddr_in server, URLParser* url_parser)	//review
{
	//create socket
	SOCKET sock_robot = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//API handler
	if (sock_robot == INVALID_SOCKET)
	{
		printf("socket() error %d\n", WSAGetLastError());
		exit(-1);
	}
	//start clock for connecting to page
	clock_t start = clock();
	//printf("\tConnecting on robots... ");
	//connect to page
	//API handler
	if (connect(sock_robot, (struct sockaddr*) & server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		//printf("failed with %d\n", WSAGetLastError());
		return -1;
	}
	//stop clock and print timing info for connecting to page
	clock_t stop = clock();
	clock_t time_elapsed = (stop - start) / CLOCKS_PER_MS;
	//printf("done in %d ms\n", time_elapsed);
	//create a head request for robots
	char head_request[] = "HEAD /robots.txt HTTP/1.0\r\nUser-agent: myUniversityCrawler/1.0\r\nHost: ";
	char head_request_1[] = "\r\nConnection: close\r\n\r\n";
	char* temp_robot = append_char_arrays(head_request, url_parser->get_host(), false);
	char* head_request_final = append_char_arrays(temp_robot, head_request_1, true);
	//send head request
	socket_send(sock_robot, head_request_final);
	free(head_request_final);
	//define robot buffer info
	unsigned long robot_buf_max = 16384;
	unsigned long robot_buf_length = 8192;
	char* robot_recv_buf = (char*)malloc(robot_buf_length);
	char* start_of_robot = robot_recv_buf;
	//space used by robot buffer
	unsigned long space_used_robot;
	//API handler
	if (robot_recv_buf == NULL)
	{
		printf("unable to malloc. Exiting...\n");
		exit(-1);
	}
	//robot receive ----------------
	start = clock();
	//printf("\tLoading... ");


	space_used_robot = socket_receive(sock_robot, &robot_recv_buf, robot_buf_length, &start_of_robot, robot_buf_max);
	if (space_used_robot == -1)
	{
		return -1;
	}
	if (!valid_header(start_of_robot))
	{
		//printf("failed with non-HTTP header\n");
		return -1;
	}
	//printf("Start: \n%s\nEnd", start_of_robot);
	stop = clock();
	time_elapsed = (stop - start) / CLOCKS_PER_MS;
	//printf("done in %d ms with %d bytes\n", time_elapsed, space_used_robot);
	robot_recv_buf = start_of_robot;
	start_of_robot = NULL;
	//printf("\tVerying header... ");
	//3 spaces for status code plus null terminator
	char status[4];
	char* robot_status_code;
	//get status code
	robot_status_code = get_status_code(robot_recv_buf, (char*)status);
	if (robot_status_code[0] == '2')
	{
		http_codes[0]++;
	}
	else if (robot_status_code[0] == '3')
	{
		http_codes[1]++;
	}
	else if (robot_status_code[0] == '4')
	{
		http_codes[2]++;
	}
	else if (robot_status_code[0] == '5')
	{
		http_codes[3]++;
	}
	else
	{
		http_codes[4]++;
	}
	//printf("status code %s\n", robot_status_code);
	if ((atoi(robot_status_code) < 400) || (atoi(robot_status_code) > 499))
	{
		return -1;
	}
	//end of robot
}

int WebCrawler::start_page(sockaddr_in server, URLParser* url_parser, char* url, int arg)	//review
{
	//start of page socket
	//create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//API handler
	if (sock == INVALID_SOCKET)
	{
		printf("socket() error %d\n", WSAGetLastError());
		exit(-1);
	}
	clock_t start = clock();
	//printf("      * Connecting to page...");
	//connect to page
	//API handler
	if (connect(sock, (struct sockaddr*) & server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		//printf("failed with %d\n", WSAGetLastError());
		return -1;
	}
	clock_t stop = clock();
	clock_t time_elapsed = (stop - start) / CLOCKS_PER_MS;
	//printf("done in %d ms\n", time_elapsed);
	//create a get request and append necessary info to it
	char get_request[] = "GET ";
	char get_request_1[] = " HTTP/1.0\r\nUser-agent: myUniversityCrawler/1.0\r\nHost: ";
	char get_request_2[] = "\r\nConnection: close\r\n\r\n";
	char* temps = append_char_arrays(get_request, url_parser->get_request(), false);
	temps = append_char_arrays(temps, get_request_1, true);
	temps = append_char_arrays(temps, url_parser->get_host(), true);
	char* get_request_final = append_char_arrays(temps, get_request_2, true);
	socket_send(sock, get_request_final);
	//free/delete memory that is no longer needed
	free(get_request_final);
	delete url_parser;

	//prepare a receive buffer of 8KB that dynamically resizes
	unsigned long recv_buf_length = 8192;
	char* recv_buf = (char*)malloc(recv_buf_length);

	int recv_buf_max;
	if (arg == 3) recv_buf_max = 2097152;
	else recv_buf_max = 300000000;
	unsigned long space_used;
	//to keep track of the start of buffer when reallocated
	char* start_of_buf = recv_buf;
	//API hanlder
	if (recv_buf == NULL)
	{
		printf("unable to malloc. Exiting...\n");
		exit(-1);
	}
	//printf("\tLoading... ");

	start = clock();
	space_used = socket_receive(sock, &recv_buf, recv_buf_length, &start_of_buf, recv_buf_max);
	bytes = space_used;
	if (space_used == -1)
	{
		return -1;
	}
	if (!valid_header(start_of_buf))
	{
		//printf("failed with non-HTTP header\n");
		return -1;
	}
	//printf("Start: \n%s\n", start_of_buf);
	recv_buf = start_of_buf;
	start_of_buf = NULL;
	//stop timer and display timer info of loading page
	stop = clock();
	time_elapsed = (stop - start) / CLOCKS_PER_MS;
	//printf("done in %d ms, with %d bytes\n", time_elapsed, space_used);
	//printf("\tVerying header... ");
	//3 spaces for status code plus null terminator
	char* status_code;
	char status[4];
	//get status code
	status_code = get_status_code(recv_buf, (char*)status);
	//printf("status code %s\n", status_code);
	if (status_code[0] == '2')
	{
		http_codes[0]++;
	}
	else if (status_code[0] == '3')
	{
		http_codes[1]++;
	}
	else if (status_code[0] == '4')
	{
		http_codes[2]++;
	}
	else if (status_code[0] == '5')
	{
		http_codes[3]++;
	}
	else
	{
		http_codes[4]++;
	}
	//find end of HTTP header
	char* temp = recv_buf;
	temp = strstr(temp, "\r\n\r\n");
	int header_size = temp - recv_buf;
	int remaining_size = space_used - (temp - recv_buf);
	temp += header_size;
	bool header_OK = false;
	//if 200 OK status then parse HTML
	if (atoi(status_code) == 200)
	{
		HTMLParserBase* html_parser = new HTMLParserBase(url, temp, remaining_size);
		links = *(html_parser->get_links());
		tamu_links = html_parser->get_tamu_links();
		outside_tamu = html_parser->get_outside_tamu_links();
		//prevent dangling pointers
		temp = NULL;
		header_OK = true;
		delete html_parser;
	}
	if (arg == 2)
	{
		print_http_header(recv_buf);
		//printf("\n");
	}
	if (header_OK) return 1;
}

char* WebCrawler::append_char_arrays(char* first_array, char* second_array, bool prev_used)
{
	//length used to create memory block of the appropriate size
	int length_1 = strlen(first_array);
	int length_2 = strlen(second_array);
	int null_character = 1;
	int total_length = length_1 + length_2 + null_character;
	//address for start of buffer
	void* start_address;
	//if already a previously created memory block then reallocate it
	if (prev_used)
	{
		start_address = realloc(first_array, total_length);
	}
	//else create new memory block
	else
	{
		if ((start_address = malloc(total_length)) == NULL)
		{
			printf("Failed to allocate memory\n");
			exit(-1);
		}
	}
	//copy passed contents into new memory block
	char* send_buf = (char*)start_address;
	start_address = NULL;
	for (int i = 0; i < length_1; i++) {
		send_buf[i] = first_array[i];
	}
	for (int i = length_1; i < total_length; i++) {
		send_buf[i] = second_array[i - length_1];
	}
	//return appended/merged arrays
	return send_buf;
}

bool WebCrawler::valid_header(char* buffer)
{
	char header[6] = { 'H', 'T', 'T', 'P','/', '\0' };
	int count = 0;
	int i = 0;
	while (true)
	{
		if (i > strlen(header))
		{
			return false;
		}
		else if (buffer[i] == header[i])
		{
			count++;
			if (count == strlen(header))
			{
				return true;
			}
		}
		i++;
	}
}

char* WebCrawler::get_status_code(char* recv_buf, char* status_array)
{
	int count = 0;
	//bound used to identify numbers from recv_buf
	int lower_bound = '0';
	int upper_bound = '9';
	//iterator
	int i = 0;
	//find status code
	while (true)
	{
		if ((recv_buf[i] >= lower_bound) && (recv_buf[i] <= upper_bound))
		{
			status_array[count] = recv_buf[i];
			count++;
			if (count == 3)
			{
				status_array[count] = '\0';
				break;
			}
		}
		else
		{
			count = 0;
		}
		i++;
	}
	return status_array;
}

void WebCrawler::print_http_header(char* recv_buf)
{
	//printf("\n---------------------------------------- \n");
	//iterator
	int i = 0;
	int count = 0;
	//print HTTP header
	char* temp = strstr(recv_buf, "\r\n\r\n");
	char* copied_buffer = recv_buf;
	while (copied_buffer != temp)
	{
		//printf("%c", copied_buffer[0]);
		copied_buffer++;
	}
	copied_buffer = NULL;
}

unsigned long WebCrawler::socket_receive(SOCKET sock, char** recv_buf, int recv_buf_length, char** start_of_buf, int buf_limit)
{
	//variables to keep track of buffer info
	unsigned long result = 0;
	unsigned long space_available = recv_buf_length;
	unsigned long space_used_up = 0;
	//bool checked_header = false;
	//timeout at 10 seconds
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	clock_t start = clock();
	clock_t stop;
	//start clock for page loading
	int value;
	while (true)
	{
		fd_set input_fd;
		FD_ZERO(&input_fd);
		FD_SET(sock, &input_fd);
		//receive result info
		if ((value = select(0, &input_fd, NULL, NULL, &timeout)) > 0)
		{
			result = recv(sock, *recv_buf, space_available, 0);
			//printf("Timeval: %d\n", timeout.tv_sec);
			//check if it is a valid header
			if (result > 0)
			{
				//keep track of space used
				space_used_up += result;
				//printf("Space Used: %d\n", space_used);
				//if buffer is full resize it and double the size
				if (space_used_up >= recv_buf_length)
				{
					if (recv_buf_length >= buf_limit || space_used_up >= buf_limit)
					{
						//printf("failed with exceeding max\n");
						return -1;
					}
					recv_buf_length *= 2;
					*start_of_buf = (char*)realloc(*start_of_buf, recv_buf_length);
					*recv_buf = *start_of_buf + space_used_up;
					space_available = recv_buf_length - space_used_up;
				}
				//adjust recv_buf pointer
				//adjust space available
				else
				{
					*recv_buf += result;
					space_available -= result;
				}
			}
			else if (result == 0)
			{
				//connection closed
				break;
			}
			else
			{
				//recv failed and return error
				printf("failed with %d on recv\n", WSAGetLastError());
				return -1;
			}
		}
		else if (value == 0)
		{
			//printf("slow download\n");
			return -1;
		}
		else
		{
			//printf("failed with %d\n", WSAGetLastError());
			return -1;
		}
		stop = clock();
		clock_t time_elapsed = (stop - start) / CLOCKS_PER_MS;
		if (time_elapsed >= 10000)
		{
			//printf("slow download\n");
			return -1;
		}
	}
	return space_used_up;
}

void WebCrawler::socket_send(SOCKET sock, char* request)
{
	//send request
	//API hanlder
	if (send(sock, request, strlen(request), 0) == SOCKET_ERROR)
	{
		printf("Send error: %d\n", WSAGetLastError());
		exit(-1);
	}
}

int* WebCrawler::get_links()
{
	return &links;
}

int* WebCrawler::get_http_codes()
{
	return http_codes;
}

int WebCrawler::get_bytes_downloaded()
{
	return bytes;
}

int WebCrawler::get_tamu_links()
{
	return tamu_links;
}

int WebCrawler::get_outside_tamu_links()
{
	return outside_tamu;
}

void WebCrawler::add_to_memory(char* array)
{
	memory_list.push_back(array);
}