
#ifndef VOICE_HPP
#define VOICE_HPP

#include <string>

// Fonction pour convertir du texte en discours et retourner le chemin du fichier audio généré.
std::string convertTextToSpeech(const std::string& text);

std::string convertDroidTextToSpeech(const std::string& text);
#endif // TEXT_TO_SPEECH_HPP
