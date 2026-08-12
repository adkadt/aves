#include <Arduino.h>

#include "system.hpp"
#include "buzzer.hpp"
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

    // update buzzer player
    buzzerPlayer.update();
}

// private implementation
namespace {
    void playStartup() {
        switch (System::getMode()) {
            case System::Mode::FLIGHT:
                tone(Pins::BUZZER, 523);   // C5
                delay(90);

                tone(Pins::BUZZER, 784);   // G5
                delay(90);

                tone(Pins::BUZZER, 1047);  // C6
                delay(90);

                tone(Pins::BUZZER, 1319);  // E6
                delay(150);

                tone(Pins::BUZZER, 784);   // G5
                delay(220);

                noTone(Pins::BUZZER);
                break;

            case System::Mode::GROUND:
                tone(Pins::BUZZER, 1047);  // C6
                delay(120);

                tone(Pins::BUZZER, 784);   // G5
                delay(120);

                tone(Pins::BUZZER, 1047);  // C6
                delay(200);

                noTone(Pins::BUZZER);
                break;

            case System::Mode::DEBUG:
                tone(Pins::BUZZER, 523, 200);   // C5
                delay(400);

                tone(Pins::BUZZER, 523, 200);   // C5
                delay(400);

                noTone(Pins::BUZZER);
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
}
