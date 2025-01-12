#include <iostream>
#include <curl/curl.h>
#include <string>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
	// Calculate total size of received data
	size_t totalSize = size * nmemb;

	// Append data to the response string
	response->append((char*)contents, totalSize);

	// Return total size of data processed
	return totalSize;
}


std::string checkUpdate