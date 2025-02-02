#include <iostream>
#include <string>
#include <filesystem>
#include "SpotifyAPI.h"
#include "Updater.h"
#include "Downloader.h"
#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <windows.h>

void setConsoleEncoding() {
	SetConsoleOutputCP(CP_UTF8);
}


int main() {
	setConsoleEncoding();
	checkUpdate();
	std::string cID, cSECRET;
	std::cout << "Enter CLIENT_ID: ";

	std::getline(std::cin >> std::ws, cID);
	std::cout << "Enter CLIENT_SECRET: ";
	std::getline(std::cin >> std::ws, cSECRET);

	std::string postData = "grant_type=client_credentials&client_id=" + cID + "&client_secret=" + cSECRET;
	std::string token{ SpotifyAPI::getAccessToken(postData) };

	PLOGD << "Welcome to Spotlify!" << '\n';
	PLOGD << "if you got an error in the access token step please restart and try again";
	std::cout << "Enter the id of the playlist you want to download: ";
	std::string ID{};
	std::getline(std::cin >> std::ws, ID);
	SpotifyAPI::GetPlaylistTrackNames(ID, token);

	std::vector<std::string> track_names = { "Song Name 1", "Song Name 2" };

	for (const auto& track : track_names) {
		std::string video_id = SearchYouTube(track);
		if (!video_id.empty()) {
			std::string audio_url = GetAudioUrl(video_id);
			if (!audio_url.empty()) {
				std::string output = track + ".mp3";
				DownloadAndConvert(audio_url, output);
			}
		}
	}

}

	/*checkUpdate();*/
	