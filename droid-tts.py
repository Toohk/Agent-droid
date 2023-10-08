
import numpy as np
from scipy.io.wavfile import write
import sys

sample_rate = 44100

def apply_envelope(signal, sample_rate=44100):
    """Apply an amplitude envelope to the signal."""
    envelope_size = int(0.02 * sample_rate)  # 20ms
    envelope = np.linspace(0, 1, envelope_size)
    return np.concatenate([envelope, np.ones(len(signal) - 2*envelope_size), envelope[::-1]]) * signal

def sweeping_sine_wave(start_freq, end_freq, duration, sample_rate=44100, amplitude=1):
    """Generate a sine wave that sweeps from start_freq to end_freq."""
    t = np.linspace(0, duration, int(sample_rate * duration), endpoint=False)
    freqs = np.linspace(start_freq, end_freq, int(sample_rate * duration))
    signal = amplitude * np.sin(2 * np.pi * freqs * t)
    return signal

def properties_from_word(word):
    """Generate sound properties based on a word."""
    word = word.lower().strip('!,.?')
    
    # Intonation (start and end frequency)
    start_freq = 600 + (ord(word[0]) - ord('a')) * 15  # Range from 600Hz to 990Hz
    end_freq = 600 + (ord(word[-1]) - ord('a')) * 15  # Range from 600Hz to 990Hz
    
    # Length of the "bip"
    duration = 0.2 + len(word) * 0.01  # Range from 0.2s to 0.5s
    
    # Duration of the pause
    silence_duration = 0.02 + len(word) * 0.001  # Range from 20ms to 50ms
    
    return start_freq, end_freq, duration, silence_duration

def text_to_droid_bips(text, sample_rate=44100):
    words = text.split()
    filtered_words = [word for word in words if word.strip('!,.?')]
    
    unique_bips = []
    i = 0
    while i < len(filtered_words):
        # Group words by 3 or less if near the end
        group = filtered_words[i:i+3]
        word_for_bip = ' '.join(group)

        start_freq, end_freq, duration, silence_duration = properties_from_word(word_for_bip)
        bip = apply_envelope(sweeping_sine_wave(start_freq, end_freq, duration))
        unique_bips.append(bip)
        
        # Add longer pause for end of sentences
        if word_for_bip[-1] in ['.', '!', '?'] and i != len(filtered_words) - 1:
            unique_bips.append(np.zeros(int(sample_rate * 0.5)))
        else:
            unique_bips.append(np.zeros(int(sample_rate * silence_duration)))
        
        i += 3  # Move to the next group of words
    
    return np.concatenate(unique_bips)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python droid-tts.py 'Text for the tts' 'output_filename.wav'")
        sys.exit(1)

    text = sys.argv[1]
    output_filename = sys.argv[2]
    sample_rate = 44100
    sound = text_to_droid_bips(text)
    normalized_sound = (sound / np.max(np.abs(sound)) * 32767)
    amplitude = 0.1  # Réduisez l'amplitude du son après normalisation
    reduced_sound = (normalized_sound * amplitude).astype(np.int16)
    write(output_filename, sample_rate, reduced_sound)
    print(f"Audio saved to {output_filename}")
