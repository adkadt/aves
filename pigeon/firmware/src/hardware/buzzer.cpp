#include <Arduino.h>

#include "system/system.hpp"
#include "hardware/buzzer.hpp"
#include "config.hpp"

namespace {
    void playStartup();

    struct BuzzerNote {
        uint16_t frequency;
        uint16_t duration;
    };

    class BuzzerPlayer {
        public:
            template <size_t N>
            void play(const BuzzerNote (&notes)[N]);
            void update();
            void stop();
            bool isPlaying() const;

        private:
            const BuzzerNote* notes = nullptr;
            uint16_t length = 0;
            uint8_t index = 0;
            uint32_t lastUpdate = 0;
            bool playing = false;
    };

    BuzzerPlayer buzzerPlayer;
}

namespace {
    const BuzzerNote gpsLockNotes[] = {
        { 880, 120 },
        {   0,  60 },
        { 1175, 220 },
    };

    const BuzzerNote gpsLostNotes[] = {
        { 1175, 220 },
        { 0,     80 },
        { 700,  300 },
    };

    const BuzzerNote startupFlightNotes[] = {
        { 523,  90 }, // C5
        { 784,  90 }, // G5
        { 1047, 90 }, // C6
        { 1319, 150}, // E6
        { 784,  220}, // G5
    };

    const BuzzerNote startupGroundNotes[] = {
        { 1047, 120 }, // C6
        { 784,  120 }, // G5
        { 1047, 200 }, // C6
    };

    const BuzzerNote startupDebugNotes[] = {
        { 523, 200 }, // C5
        { 0,   200 },
        { 523, 200 }, // C5
        { 0,   200 },
    };

    const BuzzerNote recoveryNotes[] = {
        { 2200, 300 },
        {    0, 1700 },
    };
}


void Buzzer::begin() {
    pinMode(Pins::BUZZER, OUTPUT);
    noTone(Pins::BUZZER);

    buzzerPlayer.stop();
    playStartup();
}


void Buzzer::update() {
    // play mode start up after mode change
    if (System::modeChanged()) {
        buzzerPlayer.stop();
        playStartup();
        return;
    }
    
    // start buzzer sequence
    if (System::stateChanged()) {
        System::State previousState = System::getPreviousState();

        // startSequence
        switch (System::getState()) {
            case System::State::GPS_SEARCHING:
                if (previousState == System::State::GPS_LOCK)
                    buzzerPlayer.play(gpsLostNotes);
                break;
            case System::State::GPS_LOCK:
                buzzerPlayer.play(gpsLockNotes);
                break;
            default:
                break;
        }
    }

    // Recovery buzzer
    if (System::getState() == System::State::RECOVERY) {
        if (!buzzerPlayer.isPlaying())
            buzzerPlayer.play(recoveryNotes);

        buzzerPlayer.update();
        return;
    }

    // update buzzer player
    buzzerPlayer.update();
}

// private implementation
namespace {
    void playStartup() {
        switch (System::getMode()) {
            case System::Mode::FLIGHT:
                buzzerPlayer.play(startupFlightNotes);
                break;

            case System::Mode::GROUND:
                buzzerPlayer.play(startupGroundNotes);
                break;

            case System::Mode::DEBUG:
                buzzerPlayer.play(startupDebugNotes);
                break;
            
            default:
                break;
        }
    }

    template <size_t N>
    void BuzzerPlayer::play(const BuzzerNote (&notes)[N]) {
        if (notes == nullptr || N == 0) {
            stop();
            return;
        }

        this->notes = notes;
        this->length = N;
        index = 0;
        lastUpdate = millis();
        playing = true;

        tone(Pins::BUZZER, notes[index].frequency, notes[index].duration);
    }

    void BuzzerPlayer::update() {
        if (!playing)
            return;

        uint32_t now = millis();
        if (now - lastUpdate < notes[index].duration)
            return;

        lastUpdate = now;
        index++;

        if (index >= length) {
            stop();
            return;
        }

        tone(Pins::BUZZER, notes[index].frequency, notes[index].duration);
    }

    void BuzzerPlayer::stop() {
        noTone(Pins::BUZZER);
        playing = false;
        notes = nullptr;
        index = 0;
        lastUpdate = 0;
    }   

    bool BuzzerPlayer::isPlaying() const {
        return playing;
    }
}
