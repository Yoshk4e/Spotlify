#include <string>
#include <string_view>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <iostream>


size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
	// Calculate total size of received data
	size_t totalSize = size * nmemb;

	// Append data to the response string
	response->append((char*)contents, totalSize);

	// Return total size of data processed
	return totalSize;
}

static plog::ColorConsoleAppender<plog::TxtFormatter> colorConsoleAppender; // Logs to console


//getToken Function is used to get an acess token from spotify servers

std::string getAccessToken(const std::string& postData){

	plog::init(plog::debug, &colorConsoleAppender);

	CURL* curl;
	CURLcode res;
	std::string apiResponse; // Declare the variable to store the API response


	//Intiating WinSock stuff
	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if (curl) {
		//the url used to send a POST request for spotify servers
		curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
		//POST request required fields
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
		//output recieved data to the WriteCallback function
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		//Write the data to apiResponse
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &apiResponse);

		//perform request and then res gets the return code 
		res = curl_easy_perform(curl);

		//check for errors
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s \n",
				curl_easy_strerror(res));
		//cleanup

		curl_easy_cleanup(curl);
	}

	try {

		nlohmann::json JsonResponse = nlohmann::json::parse(apiResponse);
		
		if (JsonResponse.contains("access_token")) {
			PLOGD << "Access Token is Valid extracting.....";
			return JsonResponse["access_token"];
		}
		else if (JsonResponse.contains("error")) {
			PLOGF << "Error From API: " << JsonResponse["error"].get<std::string>() << '\n';
			if (JsonResponse.contains("error_description")) {
				PLOGF << "Description: " << JsonResponse["error_description"].get<std::string>() << '\n';
			}
		}
		else {
			PLOGF << "Unexpected JSON Format: " << JsonResponse.dump(4) << '\n';
		}
		
	}
	catch (nlohmann::json::parse_error& e) {

		PLOGF << "JSON Parse Error: " << e.what() << '\n';
		return "1";
	}
	catch (nlohmann::json::type_error& e) {
		PLOGF << "JSON Type Error: " << e.what() << '\n';
		return "1";
	}
	catch (std::exception& e) {
		PLOGF << "General Error: " << e.what() << '\n';
		return "1";
	}


	
	curl_global_cleanup();

	return apiResponse;
}

//
//std::string getPlayList(const std::string& Link, const std::string& accesstoken) {
//	
//	CURL* curl;
//	CURLcode res;
//	std::string PlayList; //Decleration of PlayList variable that will store the response
//
//	return "";
//}
