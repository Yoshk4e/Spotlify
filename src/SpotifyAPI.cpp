/*
 * SpotifyAPI.cpp - Implementation of Spotify Web API interactions
 * Handles authentication and playlist data retrieval
 */
#include "SpotifyAPI.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <plog/Log.h>
#include <vector>
#include <algorithm>
#include "common.h"


namespace SpotifyAPI {

    // Authenticates with Spotify API and retrieves access token
    std::string getAccessToken(const std::string& postData) {
        CURL* curl;
        CURLcode res;
        std::string apiResponse;

        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &apiResponse);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                PLOGF << "CURL error: " << curl_easy_strerror(res);
            }
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();

        try {
            auto JsonResponse = nlohmann::json::parse(apiResponse);
            if (JsonResponse.contains("access_token")) {
                return JsonResponse["access_token"];
            }
            if (JsonResponse.contains("error")) {
                PLOGF << "API Error: " << JsonResponse["error"].get<std::string>();
                if (JsonResponse.contains("error_description")) {
                    PLOGF << "Details: " << JsonResponse["error_description"].get<std::string>();
                }
                return "";
            }
            PLOGF << "Malformed response: " << JsonResponse.dump();
        }
        catch (const nlohmann::json::exception& e) {
            PLOGF << "JSON Error: " << e.what();
            return "1";
        }
        return "";
    }

    // Internal JSON parser
    static std::vector<std::string> ExtractTrackNames(const std::string& playlistJson) {
        std::vector<std::string> trackNames;
        try {
            auto jsonData = nlohmann::json::parse(playlistJson);
            if (jsonData.contains("tracks") && jsonData["tracks"].contains("items")) {
                for (const auto& item : jsonData["tracks"]["items"]) {
                    if (item.contains("track") && item["track"].contains("name")) {
                        std::string name = item["track"]["name"];
                        name.erase(std::remove(name.begin(), name.end(), '/'), name.end());
                        trackNames.push_back(name);
                    }
                }
            }
        }
        catch (const nlohmann::json::exception& e) {
            PLOGF << "JSON Parse Error: " << e.what();
        }
        return trackNames;
    }

    // Retrieves track names from specified playlist
    std::vector<std::string> GetPlaylistTrackNames(const std::string& playlistId, const std::string& accessToken) {
        CURL* curl;
        CURLcode res;
        std::string apiResponse;
        const std::string endpoint = "https://api.spotify.com/v1/playlists/" + playlistId;

        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();

        if (curl) {
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &apiResponse);

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);

            if (res != CURLE_OK) {
                PLOGF << "CURL Error: " << curl_easy_strerror(res);
                return {};
            }
        }
        curl_global_cleanup();
        return ExtractTrackNames(apiResponse);
    }
}