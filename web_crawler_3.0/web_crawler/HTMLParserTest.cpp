/*
Name: Ismael Rodriguez
Class: CSCE 463-500
Semester: Fall 2019
*/
// HTMLParserTest.cpp
// Example program showing how to use the parser
// CSCE 463 sample code
//
#include "stdafx.h"

HTMLParserBase::HTMLParserBase(char* baseURL, char* recv_buffer, int size)
{
	HTMLParserBase* parser = new HTMLParserBase;
	//printf("      + Parsing page... ");
	clock_t start = clock();
	int nLinks;
	char* linkBuffer = parser->Parse(recv_buffer, size, baseURL, (int)strlen(baseURL), &nLinks);
	for (int i = 0; i < nLinks; i++)
	{
		if (strstr(linkBuffer, "tamu.edu") == NULL)
		{
			linkBuffer += strlen(linkBuffer) + 1;
			continue;
		}
		//printf("%s\n", linkBuffer);
		char* str = strchr(linkBuffer, ':');
		//printf("Finding host...\n");
		int j = 0;
		while (true) {
			if ((str[j] == ':') || (str[j] == '/')) {
				str += 1;	//traverse string
			}
			else
			{
				break;
			}
		}
		j = 0;
		int starting_index = 0;
		int ending_index = 0;
		while (true)
		{
			if (str[j] == '/') break;
			else if (str[j] == ':') break;
			else if (str[j] == '?') break;
			else if (str[j] == '#') break;
			else if (str[j] == '\0') break;
			else
			{
				ending_index = j;
				j++;
			}
		}
		int size = j + 1;
		char* sub_str = NULL;
		if ((sub_str = (char*)malloc(size)) == NULL)
		{
			printf("Failed to allocate memory\n");
			exit(-1);
		}
		strncpy_s(sub_str, size, str, _TRUNCATE);
		string temp = (string)sub_str;
		int n;
		if (temp.find("tamu.edu") == -1)
		{
			linkBuffer += strlen(linkBuffer) + 1;
			continue;
		}
		n = temp.find("tamu.edu");
		sub_str += (n + 8);
		if (sub_str[0] != '.')
		{
			//found a match
			string url_ = (string)baseURL;
			if (url_.find("tamu.edu") == -1)
			{
				outside_tamu++;
			}
			tamu_links++;
		}
		linkBuffer += strlen(linkBuffer) + 1;
	}
	clock_t stop = clock();
	clock_t time_elapsed = (stop - start) / CLOCKS_PER_MS;
	//printf("done in %d ms with %d links\n", time_elapsed, nLinks);
	// check for errors indicated by negative values
	if (nLinks < 0)
		nLinks = 0;
	links = &nLinks;
	delete parser;		// this internally deletes linkBuffer
	//----------------------------------------------------------------------------------------------------------------------------------------
}

int* HTMLParserBase::get_links()
{
	return links;
}

int HTMLParserBase::get_tamu_links()
{
	return tamu_links;
}

int HTMLParserBase::get_outside_tamu_links()
{
	return outside_tamu;
}
