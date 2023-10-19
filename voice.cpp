
#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <random>
#include "voice.hpp"

constexpr size_t CHUNK_SIZE = 1024;
const std::string XI_API_KEY = ""; 
const std::string VOICE_ID = ""; 

std::size_t write_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata) {
    std::ofstream* out = static_cast<std::ofstream*>(userdata);
    out->write(ptr, size * nmemb);
    return size * nmemb;
}

std::string generateRandomFilename(const std::string& prefix, const std::string& extension, int length = 10) {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, sizeof(alphanum) - 2);
    
    std::string result;
    for (int i = 0; i < length; ++i) {
        result += alphanum[dist(gen)];
    }
    return prefix + result + extension;
}

std::string convertTextToSpeech(const std::string& text) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Erreur lors de l'initialisation de curl" << std::endl;
        return "";
    }

    std::string tts_url = "https://api.elevenlabs.io/v1/text-to-speech/" + VOICE_ID + "/stream";

    nlohmann::json data = {
        {"text", text},
        {"model_id", "eleven_multilingual_v2"}
        // Ajoutez ici d'autres paramètres si nécessaire
    };

    std::string payload = data.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("xi-api-key: " + XI_API_KEY).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, tts_url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string outputPath = "./temp/elevenlabs/" + generateRandomFilename("output_", ".mp3");
    std::ofstream outputFile(outputPath, std::ios::binary);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputFile);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Erreur curl: " << curl_easy_strerror(res) << std::endl;
        outputPath = "";
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    outputFile.close();

    return outputPath;
}

std::string remove_quotes(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (c != '\'' && c != '\"') {
            result += c;
        }
    }
    return result;
}

std::string convertDroidTextToSpeech(const std::string& originalText) {
    std::string text = originalText;
    text = remove_quotes(text);
    std::string outputPath = "./temp/voice/" + generateRandomFilename("output_", ".wav");
    std::string command = "python droid-tts.py '" + text + "' '" + outputPath + "' > /dev/null 2>&1";
    system(command.c_str());
    return outputPath;
}
