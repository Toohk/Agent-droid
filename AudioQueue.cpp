
#include "AudioQueue.hpp"
// Inclure d'autres bibliothèques nécessaires pour lire les fichiers audio

AudioQueue::AudioQueue() : isPlaying(false) {}

void AudioQueue::enqueue(const std::string& audioFile) {
    std::unique_lock<std::mutex> lock(mtx);
    audioFiles.push(audioFile);
    cv.notify_one();
}

void AudioQueue::stopPlayback() {
    isPlaying = false;
}

void AudioQueue::startPlayback() {
    if (isPlaying) return;

    isPlaying = true;
    while (isPlaying) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return !audioFiles.empty(); });

        std::string currentAudio = audioFiles.front();
        audioFiles.pop();
        lock.unlock();

        playAudio(currentAudio);
//        std::remove(currentAudio.c_str());

 // Arrêter la lecture de la file d'attente
    void stopPlayback();       // Supprimer le fichier audio après la lecture si nécessaire
        // std::remove(currentAudio.c_str());
    }
}

void AudioQueue::playAudio(const std::string& audioFile) {
    // Utilisez la bibliothèque ou l'outil de votre choix pour lire le fichier audio
    std::string command = "aplay " + audioFile + " > /dev/null 2>&1";
    system(command.c_str());
}
