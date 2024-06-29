#include <array>
#include <cmath>
#include <iostream>

struct Antenna {
    double amplitude;
    double phase;
};

struct AntennaArray {
    std::array<Antenna, 12> elements;
    double center_freq;
    double beam_width;
    double pulse_duration;
    double pulse_repetition_rate;

    void set_phase(int element, double angle) {
        elements[element].phase = -angle * (2 * M_PI) / (beam_width / 2);
    }

    void update_amplitudes() {
        for (int i = 0; i < elements.size(); i++) {
            elements[i].amplitude = std::sin(elements[i].phase] / 2 + 1;
        }
    }

    void send_pulse() {
        update_amplitudes();

        // Send the pulse to all antenna elements
        for (int i = 0; i < elements.size(); i++) {
            // Simulate sending the pulse
            std::cout << "Sending pulse from element " << i << " with amplitude " << elements[i].amplitude << std::endl;
        }

        // Wait for the next pulse
        std::this_thread::sleep_for(std::chrono::microseconds(pulse_repetition_rate * 1000));
    }
};

int main() {
    AntennaArray array;
    array.center_freq = 2e9;
    array.beam_width = 10;
    array.pulse_duration = 1e-6;
    array.pulse_repetition_rate = 1e6;

    // Set the phase for the first element to intercept the target at +/- 5 degrees from the center of the beam
    array.set_phase(0, 5);

    // Send the pulse
    array.send_pulse();

    return 0;
}
