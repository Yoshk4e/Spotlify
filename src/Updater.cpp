#include <iostream>
#include <curl/curl.h>
#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <string>
#include <nlohmann/json.hpp>

size_t WriteCallBack(void* contents, size_t size, size_t nmemb, std::string* response) {
	// Calculate total size of received data
	size_t totalSize = size * nmemb;

	// Append data to the response string
	response->append((char*)contents, totalSize);

	// Return total size of data processed
	return totalSize;
}

plog::ColorConsoleAppender<plog::TxtFormatter> colorConsoleAppender; //set logging

std::string checkUpdate() {

	CURL* curl;
	CURLcode res;
	std::string Version;

	//Intiating WinSock
	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();

	if (curl) {

		//get latest version number
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/Yoshk4e/Spotlify/releases/latest");
		PLOGD << "Checking for Updates..";
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack);
		PLOGD << "Writing CallBack data..";
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &Version);
		PLOGD << "DONE!";

		res = curl_easy_perform(curl);

		//check for errors
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s \n",
				curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}


	try {
		nlohmann::json LatestVer = nlohmann::json::parse(Version);

		if (LatestVer.contains("tag_name"))
			PLOGD << "Found version!";
		return LatestVer["tag_name"];
	}
	else if (LatestVer.contains("message")) {
		PLOGF << "Error Communicating With github servers!: " << LatestVer["message"].get<std::string>() << '\n';
		(LatestVer.contains("status")) {
			PLOGF << "Cause: " << LatestVer[]
		}
		}
}