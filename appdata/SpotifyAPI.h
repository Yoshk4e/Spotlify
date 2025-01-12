#pragma once
#include <string>


//forward decleration of WriteCallback Function in SpotifyAPI.cpp
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);
//forward decleration of getAccessToken Function in SpotifyAPI.cpp
std::string getAccessToken(const std::string& postData);
//forward decleration of getPlayList Function in SpotifyAPI.cpp
//std::string getPlayList(const std::string& Link, const std::string& accesstoken);