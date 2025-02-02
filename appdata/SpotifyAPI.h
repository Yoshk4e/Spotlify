#pragma once
#include <vector>
#include <string>

namespace SpotifyAPI {
    std::string getAccessToken(const std::string& postData);
    std::vector<std::string> GetPlaylistTrackNames(const std::string& playlistId, const std::string& accessToken);
}