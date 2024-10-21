/*
 * ZXArtLoader.cpp
 * 
 * This file implements the ZXArtFile and ZXArtCollection classes for the ZX Spectrum SCREEN$ Loader screen saver.
 * It provides the functionality to load and manage ZX Spectrum art files from the zxart.ee website.
 *
 * The implementation includes methods for downloading files, parsing JSON data, and managing collections of art files.
 * It also handles the communication with the zxart.ee API to retrieve art files based on various sorting criteria.
 *
 * Author: Claude 3.5 Sonnet by Anthropic
 *
 * This component was designed and implemented by Claude, an AI assistant created by Anthropic,
 * demonstrating the capabilities of artificial intelligence in software development.
 * The code was generated based on the user's requirements and best practices for C++ development.
 */

#include "ZXArtLoader.h"

// ZXArtFile implementation

size_t ZXArtFile::WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    std::vector<uint8_t>* file = static_cast<std::vector<uint8_t>*>(userdata);
    size_t total_size = size * nmemb;
    size_t old_size = file->size();
    file->resize(old_size + total_size);
    std::memcpy(file->data() + old_size, ptr, total_size);
    return total_size;
}

template<typename T>
std::optional<T> ZXArtFile::getJsonValue(const nlohmann::json& j, const std::string& key) {
    if (j.contains(key) && !j[key].is_null()) {
        return j[key].get<T>();
    }
    return std::nullopt;
}

std::string ZXArtFile::transliterate(const std::string &text) {
    UErrorCode status = U_ZERO_ERROR;

    std::string transliteratorId = "Any-Latin";
    icu::Transliterator *transliterator = icu::Transliterator::createInstance(transliteratorId.c_str(), UTRANS_FORWARD, status);

    if (U_FAILURE(status)) {
        std::cerr << "Transliterate error: " << u_errorName(status) << std::endl;
        return text;
    }

    icu::UnicodeString unicodeText = icu::UnicodeString::fromUTF8(text);
    transliterator->transliterate(unicodeText);
    std::string result;
    unicodeText.toUTF8String(result);

    delete transliterator;

    return result;
}

ZXArtFile::ZXArtFile(const nlohmann::json& json) 
    : id(getJsonValue<int>(json, "id").value_or(0)),
      title(getJsonValue<std::string>(json, "title").value_or("")),
      url(getJsonValue<std::string>(json, "url").value_or("")),
      originalUrl(getJsonValue<std::string>(json, "originalUrl").value_or("")),
      type(getJsonValue<std::string>(json, "type").value_or("")),
      year(getJsonValue<std::string>(json, "year").value_or("")) {
    auto tagsOpt = getJsonValue<std::vector<std::string>>(json, "tags");
    if (tagsOpt) {
        tags = *tagsOpt;
    }
}

bool ZXArtFile::downloadFile() {
    if (originalUrl.empty()) {
        std::cerr << "Original URL is empty, cannot download file" << std::endl;
        return false;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, originalUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fileData);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Failed to download file: " << originalUrl << std::endl;
        return false;
    }

    return true;
}

int ZXArtFile::getId() const { return id; }
const std::string& ZXArtFile::getTitle() const { return title; }
const std::string& ZXArtFile::getUrl() const { return url; }
const std::string& ZXArtFile::getOriginalUrl() const { return originalUrl; }
const std::vector<std::string>& ZXArtFile::getTags() const { return tags; }
const std::string& ZXArtFile::getType() const { return type; }
const std::string& ZXArtFile::getYear() const { return year; }
const std::vector<uint8_t>& ZXArtFile::getFileData() const { return fileData; }
const std::string ZXArtFile::getTransliteratedTitle() const { return transliterate(title); }

// ZXArtCollection implementation

size_t ZXArtCollection::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string ZXArtCollection::getSortTypeString(SortType sortType) {
    switch (sortType) {
        case SortType::Votes: return "votes";
        case SortType::Views: return "views";
        case SortType::CommentsAmount: return "commentsAmount";
        case SortType::Year: return "year";
        case SortType::Date: return "date";
        case SortType::Title: return "title";
        default: return "votes";
    }
}

std::string ZXArtCollection::getSortDirectionString(SortDirection direction) {
    switch (direction) {
        case SortDirection::Ascending: return "asc";
        case SortDirection::Descending: return "desc";
        case SortDirection::Random: return "rand";
        default: return "desc";
    }
}

ZXArtCollection::ZXArtCollection() : curl(nullptr) {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

ZXArtCollection::~ZXArtCollection() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

std::vector<std::unique_ptr<ZXArtFile>> ZXArtCollection::getFiles(
    int start, 
    int limit, 
    SortType sortType, 
    SortDirection direction
) {
    buffer.clear();
    std::string url = "https://zxart.ee/api/types:zxPicture/export:zxPicture/language:rus/start:" + 
                      std::to_string(start) + "/limit:" + std::to_string(limit) + 
                      "/order:" + getSortTypeString(sortType) + "," + getSortDirectionString(direction) +
                      "/filter:zxPictureType=standard";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return {};
    }

    std::vector<std::unique_ptr<ZXArtFile>> files;
    auto json = nlohmann::json::parse(buffer);
    for (const auto& item : json["responseData"]["zxPicture"]) {
        files.push_back(std::make_unique<ZXArtFile>(item));
    }

    return files;
}