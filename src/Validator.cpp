#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <WbemIdl.h>
#include <curl/curl.h>
#include <ctime>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <comutil.h>

// Link libraries
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "WbemUuid.lib")



// Hashing function (unchanged)
std::string sha256(const std::string& data) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create EVP_MD_CTX.");

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, data.c_str(), data.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, hash, &lengthOfHash) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Hashing failed.");
    }

    EVP_MD_CTX_free(ctx);
    std::ostringstream oss;
    for (unsigned int i = 0; i < lengthOfHash; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();
}

// WMI query utility
std::string queryWMIProperty(const std::wstring& wqlQuery, const std::wstring& property) {
    HRESULT hres;
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    std::string result;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) throw std::runtime_error("COM library initialization failed.");

    hres = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
    if (FAILED(hres)) {
        CoUninitialize();
        throw std::runtime_error("Security initialization failed.");
    }

    hres = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, reinterpret_cast<void**>(&pLoc));
    if (FAILED(hres)) {
        CoUninitialize();
        throw std::runtime_error("Failed to create WbemLocator.");
    }

    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr,
        0, nullptr, nullptr, &pSvc);
    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        throw std::runtime_error("Failed to connect to WMI namespace.");
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), bstr_t(wqlQuery.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr, &pEnumerator);
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        throw std::runtime_error("WMI query failed.");
    }

    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;
    if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        VARIANT vtProp;
        VariantInit(&vtProp);
        if (pclsObj->Get(property.c_str(), 0, &vtProp, nullptr, nullptr) == S_OK) {
            result = _bstr_t(vtProp.bstrVal);
        }
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
    return result;
}

// Collect HWID components and hash them
std::string getHWID() {
    std::ostringstream hwidComponents;
    try {
        hwidComponents << queryWMIProperty(L"SELECT SerialNumber FROM Win32_BaseBoard", L"SerialNumber");
        hwidComponents << queryWMIProperty(L"SELECT ProcessorId FROM Win32_Processor", L"ProcessorId");
        hwidComponents << queryWMIProperty(L"SELECT SerialNumber FROM Win32_DiskDrive", L"SerialNumber");
        hwidComponents << queryWMIProperty(L"SELECT MACAddress FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled = TRUE", L"MACAddress");
    }
    catch (const std::exception& ex) {
        std::cerr << "Error collecting HWID: " << ex.what() << std::endl;
        return "";
    }
    return sha256(hwidComponents.str());
}

bool validateLicense(const std::string& licensePath) {
    // Open and read the license file
    std::ifstream file(licensePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "License file not found at path: " << licensePath << std::endl;
        return false;
    }

    // Read encrypted license data
    std::string encryptedData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (encryptedData.empty()) {
        std::cerr << "License file is empty." << std::endl;
        return false;
    }

    // Initialize cURL for the POST request
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize cURL." << std::endl;
        return false;
    }

    std::string serverUrl = "https://spotlify-api.vercel.app/";
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Create the JSON body
    nlohmann::json requestBody;
    requestBody["action"] = "decrypt";
    requestBody["encrypted"] = encryptedData;

    // Set up cURL options
    curl_easy_setopt(curl, CURLOPT_URL, serverUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.dump().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append(ptr, size * nmemb);
        return size * nmemb;
        });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "cURL request failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    // Parse and handle the response
    try {
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        if (jsonResponse.contains("success") && jsonResponse["success"].get<bool>()) {
            std::cout << "Decrypted successfully: " << jsonResponse["licenseData"] << std::endl;
            return true;
        }
        else {
            std::cerr << "Decryption failed: " << jsonResponse["message"] << std::endl;
            return false;
        }
    }
    catch (const nlohmann::json::parse_error& ex) {
        std::cerr << "JSON parsing error: " << ex.what() << std::endl;
        return false;
    }
}
