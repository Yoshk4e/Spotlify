// Common.cpp
#include <string>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    if (!s) return 0;  // Prevent null pointer access
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}