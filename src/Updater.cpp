#include <iostream>
#include <curl/curl.h>
#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <string>
#include <nlohmann/json.hpp>

std::string currentVersion{ "0.0.0" };

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

    plog::init(plog::debug, &colorConsoleAppender);

    CURL* curl;
    CURLcode res;
    std::string Version;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/Yoshk4e/Spotlify/releases/latest");
        PLOGD << "Checking for Updates..";

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &Version);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "User-Agent: Spotlify-Updater/1.0"); 
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);


        if (res != CURLE_OK) {
            PLOGF << "cURL error: " << curl_easy_strerror(res);
        }

        curl_easy_cleanup(curl);
    }

    if (Version.empty()) {
        PLOGF << "Empty response from server!";
        curl_global_cleanup();
        return "1";
    }

    try {
        nlohmann::json LatestVer = nlohmann::json::parse(Version);

        if (LatestVer.contains("tag_name")) {
            std::string latestVersion = LatestVer["tag_name"];
            PLOGD << "Found Version: " << latestVersion;
            if (latestVersion != currentVersion) {
                PLOGD << "A new Update has been released! " << currentVersion << " --> " << static_cast<std::string>(LatestVer["tag_name"]);
                PLOGD << "The Update Can be found here: https://ouo.io/jL9UXU";
                curl_global_cleanup();
                return "";
            }
            else if (latestVersion == currentVersion) {
                PLOGD << "It seems like you're on the latest version!";
                curl_global_cleanup();
                return "";
            }
            curl_global_cleanup();
            return latestVersion;
        }
        if (LatestVer.contains("message")) {
            PLOGF << "GitHub error: " << LatestVer["message"].get<std::string>();
            if (LatestVer.contains("status")) {
                PLOGF << "Cause: " << LatestVer["status"].get<std::string>();
            }
        }
        else {
            PLOGF << "Unhandled response: " << LatestVer.dump(4);
        }
    }
    catch (const nlohmann::json::exception& e) {
        PLOGF << "JSON Error: " << e.what();
        curl_global_cleanup();
        return "1";
    }

    curl_global_cleanup();
    return Version;  // Return raw response in case of unexpected conditions
}
