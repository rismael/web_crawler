/*
Name: Ismael Rodriguez
Class: CSCE 463-500
Semester: Fall 2019
*/
#include "stdafx.h"
#define SD_RECV 0
#define SD_SEND 1
#define SD_BOTH 2


char* get_url(char** file_buf)
{
	if ((*file_buf = strstr(*file_buf, "http://")) == NULL)
	{
		//printf("Error parsing file_buf\n");
		return NULL;
	}
	char* temp_url = strchr(*file_buf, '\r\n');
	if (temp_url == NULL) {
		printf("Error parsing file_buf \\r\\n");
		return NULL;
	}
	int size = temp_url - *file_buf + 1;
	char* sub_str;
	if ((sub_str = (char*)malloc(size)) == NULL)
	{
		printf("Failed to allocate memory\n");
		exit(-1);
	}
	strncpy_s(sub_str, size, *file_buf, _TRUNCATE);
	*file_buf += size;
	return sub_str;
}

struct stat_arg
{
	HANDLE event_t;
	vector<char*> *queue;
	unordered_set<string> *host;
	int* dns, * robots, * urls, * links, * threads, * extracted_urls, * http_codes, * page_bytes, * tamu_links, * outside_tamu_links;
	unordered_set<DWORD> *ip;

	stat_arg(HANDLE event_handle, vector<char*>* url_queue, unordered_set<string>* hosts, int* dns_, unordered_set<DWORD>* ips, 
		int* robots_, int* urls_, int* links_, int* threads_, int* extracted_, int* http_codes_, int* page_bytes_, int* tamu_links_, int* outside_tamu_)
		: event_t(event_handle), queue(url_queue), host(hosts), dns(dns_), ip(ips), robots(robots_), urls(urls_), links(links_), threads(threads_), 
		extracted_urls(extracted_), http_codes(http_codes_), page_bytes(page_bytes_), tamu_links(tamu_links_), outside_tamu_links(outside_tamu_) {};
};

struct consumer_arg
{
	CRITICAL_SECTION* section_1, section_2, section_3, section_4;
	vector<char*>* queue;
	unordered_set<string>* host;
	int* dns, * robots, * urls, * links, * threads, * extracted_urls, * http_codes, * page_bytes, * tamu_links, * outside_tamu_links;
	unordered_set<DWORD>* ip;
	//http_codes, 0th index is 2xx, 1st index is 3xx, 2nd index is 4xx, 3rd index is 5xx, and 4th index is other

	consumer_arg(CRITICAL_SECTION* sec_, vector<char*>* url_queue, unordered_set<string>* hosts, int* dns_, unordered_set<DWORD>* ips, 
		int* robots_, int* urls_, int* links_, int* threads_, int* extracted_, int* http_codes_, int* page_bytes_, int* tamu_links_, int* outside_tamu_)
		: section_1(sec_), queue(url_queue), host(hosts), dns(dns_), ip(ips), robots(robots_), urls(urls_), links(links_), 
		threads(threads_), extracted_urls(extracted_), http_codes(http_codes_), page_bytes(page_bytes_), tamu_links(tamu_links_), outside_tamu_links(outside_tamu_)
	{
		InitializeCriticalSectionAndSpinCount(&section_2, 0x00000400);
		InitializeCriticalSectionAndSpinCount(&section_3, 0x00000400);
		InitializeCriticalSectionAndSpinCount(&section_4, 0x00000400);
	};
};

struct producer_arg
{
	CRITICAL_SECTION* section_1;
	vector<char*>* queue;
	char** file;
};

UINT stat_thread_function(void* arg)
{
	stat_arg* args = (stat_arg*)arg;
	clock_t start = clock();
	clock_t prev_time = 0;
	int prev_page_amount = 0;
	int prev_page_bytes = 0;
	while(TRUE)
	{
		
		DWORD wait_err = WaitForSingleObject(args->event_t, 2000);
		if (wait_err == WAIT_TIMEOUT)
		{
			float total_pages = *args->extracted_urls - prev_page_amount;
			float pages_per_sec = total_pages / 2;
			float total_bytes = *args->page_bytes - prev_page_bytes;
			float megabytes_per_second = (total_bytes / 2) / (float)125000;
			clock_t total_time = (clock() - start) / CLOCKS_PER_SEC;
			printf("[%3d] %4d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4d\n",
				total_time, *args->threads, args->queue->size(), *args->extracted_urls, args->host->size(), *args->dns, args->ip->size(), *args->robots, *args->urls, *args->links);
			printf("      *** crawling %.1f pps @ %.1f Mbps\n", pages_per_sec, megabytes_per_second);
			prev_page_amount = *args->extracted_urls;
		}
		else if (wait_err == WAIT_OBJECT_0)
		{
			clock_t total_time = (clock() - start) / CLOCKS_PER_SEC;
			printf("\nExtracted %d URLs @ %d/s\n", *args->extracted_urls, (*args->extracted_urls / total_time));
			printf("Looked up %d DNS names @ %d/s\n", *args->dns, (*args->dns / total_time));
			printf("Downloaded %d robots @ %d/s\n", *args->robots, (*args->robots / total_time));
			printf("Crawled %d pages @ %d/s (%.2f MB)\n", *args->urls, (*args->urls / total_time), ((float)*args->page_bytes) / ((float)1000000));
			printf("Parsed %d links @ %d/s\n", *args->links, (*args->links / total_time));
			printf("HTTP codes: 2xx = %d, 3xx = %d, 4xx = %d, 5xx = %d, other = %d\n", args->http_codes[0], args->http_codes[1], args->http_codes[2], args->http_codes[3], args->http_codes[4]);
			printf("2xx tamu.edu links: %d, Outside tamu: %d\n", *args->tamu_links, *args->outside_tamu_links);

			return 1;
		}
		else if (wait_err == WAIT_ABANDONED)
		{
			printf("WaitForSingleObject: WAIT_ABANDONED\n");
			exit(-1);
		}
		else if (wait_err == WAIT_FAILED)
		{
			printf("WaitForSingleObject: WAIT_FAILED, %d\n", GetLastError());
			exit(-1);
		}
	}

}

UINT consumer_thread_function(void* arg)
{
	consumer_arg* args = (consumer_arg*)arg;
	//multiple url mode
	int argc = 3; 
	while (TRUE)
	{
		EnterCriticalSection(args->section_1);
		if (args->queue->size() == 0)
		{
			//decrement thread count
			(*args->threads)--;
			LeaveCriticalSection(args->section_1);
			break;
		}
		char* url = args->queue->front();
		(*args->extracted_urls)++;
		args->queue->front() = NULL;
		//memory_list.push_back(url);
		args->queue->erase(args->queue->begin());
		LeaveCriticalSection(args->section_1);

		//create url parser to parse importatn url information
		//printf("........: %s", url);
		URLParser* url_parser = new URLParser(url, argc);
		char* host = url_parser->get_host();
		//check host uniqueness
		//printf("\tChecking host uniqueness... ");
		int prev_host_size = args->host->size();

		EnterCriticalSection(&args->section_2);
		args->host->insert(host);
		LeaveCriticalSection(&args->section_2);

		if (args->host->size() == prev_host_size)
		{
			//printf("failed\n");
			continue;
		}
		//printf("passed\n");
		//start clock for DNS timing
		//clock_t start = clock();
		//printf("\tDoing DNS... ");
		//in_addr used to contain address information
		struct in_addr addr;
		//inet_addr used for storing ip-address
		DWORD address = inet_addr(host);
		addr.s_addr = address;
		//if inet_addr() fails use gethostbyname()
		if (address == INADDR_NONE)
		{
			struct hostent* remote = gethostbyname(host);
			//API hanlder
			if (remote == NULL) {
				int error = WSAGetLastError();
				if (remote != NULL) {
					if (error == WSAHOST_NOT_FOUND) {
						printf("host not found\n");
						continue;
					}
					else if (error == WSANO_DATA) {
						printf("no data record found\n");
						continue;
					}
					else {
						printf("function failed with error: %ld\n", error);
						continue;
					}
				}
				//printf("failed with %d\n", error);
				continue;
			}
			//add ip-addresses to addr
			int i = 0;
			while (remote->h_addr_list[i] != 0) {
				address = (DWORD)remote->h_addr_list[i];
				addr.s_addr = *(DWORD*)remote->h_addr_list[i];
				//printf("\tIP Address: %s\n", inet_ntoa(addr));
				i++;
			}
		}

		EnterCriticalSection(&args->section_2);
		//increment DNS success													<----Remember to mutex
		(*args->dns)++;
		LeaveCriticalSection(&args->section_2);

		//stop clock and print timing information for DNS
		//clock_t stop = clock();
		//clock_t time_elapsed = (stop - start) / CLOCKS_PER_MS;
		//printf("done in %d ms, found %s\n", time_elapsed, inet_ntoa(addr));
		//check IP uniqueness
		//printf("\tChecking IP uniqueness... ");
		int prev_ip_size = args->ip->size();

		EnterCriticalSection(&args->section_2);
		args->ip->insert(address);
		LeaveCriticalSection(&args->section_2);
		
		if (args->ip->size() == prev_ip_size)
		{
			//printf("failed\n");
			continue;
		}
		//printf("passed\n");
		//sockaddr_in used to store url information
		struct sockaddr_in server;
		int port = atoi(url_parser->get_port());
		server.sin_family = AF_INET; // IPv4
		server.sin_addr = addr; // from inet_addr or gethostbyname
		server.sin_port = htons(port); // port #
		WebCrawler crawl = WebCrawler();
		int robots_return = crawl.start_robots(server, url_parser);
		EnterCriticalSection(&args->section_3);
		for (int i = 0; i < 5; i++)
		{
			args->http_codes[i] += crawl.get_http_codes()[i];
		}
		LeaveCriticalSection(&args->section_3);
		if (robots_return == -1) continue;
		else
		{
			EnterCriticalSection(&args->section_4);
			(*args->robots)++;
			LeaveCriticalSection(&args->section_4);
		}
		int page_return = crawl.start_page(server, url_parser, url, argc);
		EnterCriticalSection(&args->section_3);
		for (int i = 0; i < 5; i++)
		{
			args->http_codes[i] += crawl.get_http_codes()[i];
		}
		LeaveCriticalSection(&args->section_3);
		if ( page_return == -1) continue;
		else
		{
			EnterCriticalSection(&args->section_4);
			*args->links += *(crawl.get_links());
			*args->page_bytes += (crawl.get_bytes_downloaded());
			(*args->urls)++;
			*args->tamu_links += crawl.get_tamu_links();
			*args->outside_tamu_links += crawl.get_outside_tamu_links();
			LeaveCriticalSection(&args->section_4);
		}
	}
	return 0;
}

UINT producer_thread_function(void* arg)
{
	producer_arg* args = (producer_arg*)arg;
	char* a = NULL;
	char* start = *args->file;
	//parse through url file and populate url_queue 
	while (a = get_url(args->file))
	{
		EnterCriticalSection(args->section_1);
		args->queue->push_back(a);
		LeaveCriticalSection(args->section_1);
	}
	a = NULL;
	delete start;
	return 0;
}

//------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	//handle command line arguments.
	//Only one argument is valid else exit program.
	char* url = NULL;
	int thread_count = 0;
	ofstream file;
	char* url_file = NULL;
	char* file_buf = NULL;
	int file_size;
	vector<char*> memory_list;
	CRITICAL_SECTION section;
	InitializeCriticalSectionAndSpinCount(&section, 0x00000400);
	if (argc == 2)
	{
		url = argv[1];
		//printf("Read Argument: %s\n", url);
	}
	else if (argc == 3)
	{
		thread_count = atoi(argv[1]);
		url_file = argv[2];
		if (thread_count < 1)
		{
			printf("Invalid thread count\n");
			exit(-1);
		}
		HANDLE handle_file = CreateFile(url_file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		// process errors
		if (handle_file == INVALID_HANDLE_VALUE)
		{
			printf("CreateFile failed with %d\n", GetLastError());
			exit(-1);
		}
		LARGE_INTEGER large_integer;
		BOOL bool_return = GetFileSizeEx(handle_file, &large_integer);
		// process errors
		if (bool_return == 0)
		{
			printf("GetFileSizeEx error %d\n", GetLastError());
			exit(-1);
		}
		file_size = (DWORD)large_integer.QuadPart;			// assumes file size is below 2GB; otherwise, an __int64 is needed
		DWORD bytes_read;
		// allocate buffer
		file_buf = new char[file_size];
		// read into the buffer
		bool_return = ReadFile(handle_file, file_buf, file_size, &bytes_read, NULL);
		// process errors
		if (bool_return == 0 || bytes_read != file_size)
		{
			printf("ReadFile failed with %d\n", GetLastError());
			exit(-1);
		}
		// done with the file
		CloseHandle(handle_file);
		printf("Opened: %s with size %d\n", url_file, file_size);
	}
	else
	{
		printf("Input error: Too little or too many arguments.\n");
		exit(-1);
	}
	//initialization
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	//API handler
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		printf("WSAStartup error %d\n", WSAGetLastError());
		exit(-1);
	}
	unordered_set<string> seen_host;
	unordered_set<DWORD> seen_ip;
	int dns_success = 0;
	int robot_success = 0;
	int url_success = 0;
	int total_links = 0;
	int extracted_urls = 0;
	int page_bytes = 0;
	int tamu_link = 0;
	int outside_tamu_link = 0;
	//http_codes, 0th index is 2xx, 1st index is 3xx, 2nd index is 4xx, 3rd index is 5xx, and 4th index is other
	int http_codes[5] = { 0, 0, 0, 0, 0 };
	HANDLE stat_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!stat_event)
	{
		printf("CreateEvent failed: %d\n", GetLastError());
		exit(-1);
	}
	vector<char*> url_queue;
	producer_arg prod_arg;
	prod_arg.file = &file_buf;
	prod_arg.queue = &url_queue;
	prod_arg.section_1 = &section;
	HANDLE prod_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)producer_thread_function, &prod_arg, 0, NULL);
	//pass file buf into prod thread and make it work
	vector<HANDLE> handler;
	stat_arg arg(stat_event, &url_queue, &seen_host, &dns_success, &seen_ip, &robot_success, &url_success, &total_links,
		&thread_count, &extracted_urls, http_codes, &page_bytes, &tamu_link, &outside_tamu_link);
	consumer_arg cons_arg(&section, &url_queue, &seen_host, &dns_success, &seen_ip, &robot_success, &url_success,
		&total_links, &thread_count, &extracted_urls, http_codes, &page_bytes, &tamu_link, &outside_tamu_link);
	HANDLE stat_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)stat_thread_function, &arg, 0, NULL);
	Sleep(2000);

	for (int i = 0; i < thread_count; i++)
	{
		HANDLE cons_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)consumer_thread_function, &cons_arg, 0, NULL);
		handler.push_back(cons_thread);
	}
	WaitForSingleObject(prod_thread, INFINITE);
	CloseHandle(prod_thread);
	for (int i = 0; i < handler.size(); i++)
	{
		WaitForSingleObject(handler[i], INFINITE);
		CloseHandle(handler[i]);
	}
	SetEvent(stat_event);
	CloseHandle(stat_thread);
	for (int i = 0; i < memory_list.size(); i++) free(memory_list[i]);
	CloseHandle(stat_event);
	WSACleanup();
	return 0;
}