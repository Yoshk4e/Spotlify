#pragma once
#include <iostream>

size_t WriteCallBack(void* contents, size_t size, size_t nmemb, std::string* response);

std::string checkUpdate();
