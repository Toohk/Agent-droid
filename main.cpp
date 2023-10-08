
#include <iostream>
#include <cstdlib>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <cstring>
#include <algorithm>
#include "chat.hpp"
#include "AudioQueue.hpp"
#include <thread>

AudioQueue audioQueue;

int main() {
    Display *display;
    Window root;
    XEvent ev;
    int keycode;
    
    std::thread audioThread(&AudioQueue::startPlayback, &audioQueue);
    
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        std::cerr << "Erreur: Impossible d'ouvrir l'affichage." << std::endl;
        return 1;
    }

    root = DefaultRootWindow(display);

    // Convertir la clé en keycode
    keycode = XKeysymToKeycode(display, XK_backslash);

    // Saisir la touche pour toutes les fenêtres
    XGrabKey(display, keycode, AnyModifier, root, true, GrabModeAsync, GrabModeAsync);

    bool isRecording = false;
    pid_t recordingProcess = -1;

    while (1) {
        XNextEvent(display, &ev);

        if (ev.type == KeyPress && ev.xkey.keycode == keycode) {
            if (isRecording) {
                // Arrêter l'enregistrement
                system("killall arecord > /dev/null 2>&1");
                system("aplay stop-alert.wav > /dev/null 2>&1");
                system("./whisper.cpp/main -m ./whisper.cpp/models/ggml-base-q4.bin -f ./output.wav -otxt > /dev/null 2>&1");
                std::ifstream file("output.wav.txt");
                std::string textToSend, line;
                std::cout << std::endl;
                while (std::getline(file, line)) {
                    if (line.find("[") == std::string::npos || line.find("]") == std::string::npos) {
                        // Si la ligne ne contient pas de crochets, imprimez-la.

                        std::cout << "\033[37m" << line << std::endl << "\033[0m" << std::flush;
                        textToSend += line + " ";
                    }
                }
                std::cout << std::endl;
                file.close();
                isRecording = false;
                sendMessageToAPI(textToSend);
           } else {
                // Démarrer l'enregistrement
                system("aplay start-alert.wav > /dev/null 2>&1");
                if ((recordingProcess = fork()) == 0) {
                    system("arecord -r 16000 -f S16_LE -c 1 output.wav > /dev/null 2>&1");
                    exit(0);
                }
                isRecording = true;
            }
        }
    }
    audioQueue.stopPlayback(); // pour terminer la boucle de lecture dans le thread
    audioThread.join();  
    XCloseDisplay(display);
    return 0;
}

