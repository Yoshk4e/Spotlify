#pragma once

std::string sha256(const std::string& data);
std::string queryWMIProperty(const std::wstring& wqlQuery, const std::wstring& property);
std::string getHWID();
bool validateLicense(const std::string& licensePath);