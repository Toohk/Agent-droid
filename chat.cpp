
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "chat.hpp"
#include "voice.hpp"
#include "AudioQueue.hpp"
#include <thread>
#include <vector>

std::string accumulatedText;
extern AudioQueue audioQueue;


// Structure pour représenter un message
struct Message {
    std::string role;
    std::string content;

    Message(const std::string& r, const std::string& c) : role(r), content(c) {}
};

// Liste pour stocker l'historique des messages
std::vector<Message> history;

void addMessageToHistory(const std::string& role, const std::string& content) {
    history.push_back(Message(role, content));
}

std::string constructPayload() {
    std::string payload = R"({
        "model": "gpt-3.5-turbo",
        "messages": [)";

    for (const auto& msg : history) {
        payload += R"({"role": ")" + msg.role + R"(","content": ")" + msg.content + R"("},)";
    }

    // Enlever la dernière virgule et ajouter la fermeture de l'objet JSON et du tableau
    payload = payload.substr(0, payload.size() - 1) + R"(],
    "stream": true
    })";

    return payload;
}

void sendTextToTTS(const std::string& text) {
    std::string audio_file = convertDroidTextToSpeech(text);
    
    audioQueue.enqueue(audio_file);
    accumulatedText.clear();
}

std::size_t callback(const char* in, std::size_t size, std::size_t num, std::string* out) {
    const std::size_t totalBytes(size * num);
    out->append(in, totalBytes);

    const std::string delimiter = "data: ";
    std::string token;
    std::string::size_type position;
    
    while ((position = out->find(delimiter)) != std::string::npos) {
        token = out->substr(0, position);
        out->erase(0, position + delimiter.length());
        
        if (!token.empty()) {
            try {
                auto jsonResponse = nlohmann::json::parse(token);
                if (jsonResponse.contains("choices") &&
                    jsonResponse["choices"].is_array() &&
                    !jsonResponse["choices"].empty() &&
                    jsonResponse["choices"][0].contains("delta") &&
                    jsonResponse["choices"][0]["delta"].contains("content")) {
                   
                   std::cout << jsonResponse["choices"][0]["delta"]["content"].get<std::string>() << std::flush;
                    
                    std::string content = jsonResponse["choices"][0]["delta"]["content"].get<std::string>();
                    
                    accumulatedText += content;
                    if ((accumulatedText.length() >= 100) &&
                        (content.find('.') != std::string::npos || 
                        content.find('!') != std::string::npos || 
                        content.find('?') != std::string::npos)) {
                            
                             sendTextToTTS(accumulatedText);
                                               }                 }
            } catch (const nlohmann::json::exception& e) {
                // TODO: Handle parsing error, maybe log it
            }
        }
    }

    return totalBytes;
}

void sendMessageToAPI(const std::string& message) {
    const std::string url = "https://api.openai.com/v1/chat/completions";
    if (history.empty()) {
        addMessageToHistory("system", "Prompt");
    }
    addMessageToHistory("user", message);

    const std::string payload = constructPayload();

    const char* api_key = "";  // Attention : ne jamais afficher votre clé API en clair !
    if (!api_key) {
        std::cerr << "Veuillez définir l'environnement OPENAI_API_KEY" << std::endl;
        return;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Erreur lors de l'initialisation de curl" << std::endl;
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth_header = "Authorization: Bearer " + std::string(api_key);
    headers = curl_slist_append(headers, auth_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Erreur curl: " << curl_easy_strerror(res) << std::endl;
    } else {

        sendTextToTTS(accumulatedText);
        std::cout << std::endl;  // Nouvelle ligne à la fin
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}


