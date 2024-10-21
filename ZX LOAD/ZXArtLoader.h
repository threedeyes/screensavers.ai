/*
 * ZXArtLoader.h
 * 
 * This file defines the ZXArtFile and ZXArtCollection classes for the ZX Spectrum SCREEN$ Loader screen saver.
 * These classes handle the loading and management of ZX Spectrum art files from the zxart.ee website.
 *
 * The ZXArtFile class represents a single ZX Spectrum art file, including its metadata and content.
 * The ZXArtCollection class manages the retrieval of multiple ZX Spectrum art files from the zxart.ee API.
 *
 * Author: Claude 3.5 Sonnet by Anthropic
 *
 * This component was designed and implemented by Claude, an AI assistant created by Anthropic,
 * demonstrating the capabilities of artificial intelligence in software development.
 * The code was generated based on the user's requirements and best practices for C++ development.
 */

#ifndef _H_ZX_ART_LOADER
#define _H_ZX_ART_LOADER

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <memory>
#include <optional>
#include <curl/curl.h>
#include <unicode/unistr.h>
#include <unicode/translit.h>
#include <nlohmann/json.hpp>

class ZXArtFile {
private:
    int id;
    std::string title;
    std::string url;
    std::string originalUrl;
    std::vector<std::string> tags;
    std::string type;
    std::string year;
    std::vector<uint8_t> fileData;

    static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
    template<typename T>
    static std::optional<T> getJsonValue(const nlohmann::json& j, const std::string& key);
    static std::string transliterate(const std::string &text);

public:
    ZXArtFile(const nlohmann::json& json);
    bool downloadFile();
    int getId() const;
    const std::string& getTitle() const;
    const std::string& getUrl() const;
    const std::string& getOriginalUrl() const;
    const std::vector<std::string>& getTags() const;
    const std::string& getType() const;
    const std::string& getYear() const;
    const std::vector<uint8_t>& getFileData() const;
    const std::string getTransliteratedTitle() const;
};

enum class SortType {
    Votes,
    Views,
    CommentsAmount,
    Year,
    Date,
    Title
};

enum class SortDirection {
    Ascending,
    Descending,
    Random
};

class ZXArtCollection {
private:
    CURL* curl;
    std::string buffer;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string getSortTypeString(SortType sortType);
    std::string getSortDirectionString(SortDirection direction);

public:
    ZXArtCollection();
    ~ZXArtCollection();
    std::vector<std::unique_ptr<ZXArtFile>> getFiles(
        int start = 0, 
        int limit = 60, 
        SortType sortType = SortType::Votes, 
        SortDirection direction = SortDirection::Descending
    );
};

#endif // _H_ZX_ART_LOADER