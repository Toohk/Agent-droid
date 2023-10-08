
#ifndef AUDIO_QUEUE_HPP
#define AUDIO_QUEUE_HPP

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

class AudioQueue {
public:
    AudioQueue();
    
    // Ajouter un fichier audio à la file d'attente
    void enqueue(const std::string& audioFile);
    
    // Commencer la lecture de la file d'attente
    void startPlayback();

    // Arrêter la lectu:wre de la file d'attente
    void stopPlayback();

private:
    std::queue<std::string> audioFiles;
    std::mutex mtx;
    std::condition_variable cv;
    bool isPlaying;

    // Lire un fichier audio
    void playAudio(const std::string& audioFile);
};

#endif
