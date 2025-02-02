#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <curl/curl.h>
#include <string>
#include <regex>
#include <stdexcept>  // Add exception handling

// Add error codes
enum class DownloadError {
    OK,
    CURL_INIT_FAILED,
    EMPTY_RESPONSE,
    JSON_PARSE_ERROR,
    INVALID_VIDEO_DATA
};


// Callback declaration
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);

// YouTube search and download functions
std::string SearchYouTube(const std::string& query);
std::string ExtractPlayerResponse(const std::string& html);
std::string GetAudioUrl(const std::string& video_id);
void DownloadAndConvert(const std::string& url, const std::string& output);

#endif // DOWNLOADER_H