#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <vector>
#include <complex>
#include <functional>
#include <string>

using namespace std;

class AWGNAdder {
private:
    mt19937 gen;
    normal_distribution<double> dist;
public:
    AWGNAdder(const double noise_variance) : gen(random_device{}()), dist(0.0, sqrt(noise_variance / 2)) {}

    vector<complex<double>> add_awgn(const vector<complex<double>> clear_signal) {
        int signal_length = clear_signal.size();
        vector<complex<double>> noise_signal(signal_length);

        for (int i = 0; i < signal_length; i++) {
            noise_signal[i] = clear_signal[i] + complex<double>(dist(gen), dist(gen));
        }

        return noise_signal;
    }
};

class ModulatorGrey {
protected:
    int bits_per_symbol;
    int bits_per_components;
    int components_number;
    function<double(const vector<bool>&)> alpha;
public:
    ModulatorGrey(int modulation_size) {
        bits_per_symbol = static_cast<int>(log2(modulation_size));
        bits_per_components = static_cast<int>(bits_per_symbol / 2);

        components_number = 1 << bits_per_components;

        vector<int> components(components_number);

        for (int n = 0; n < components_number; n++) {
            components[n] = components_number - 1 - 2 * n;
        }

        double mean_power = 0;

        for (int I : components) {
            for (int Q : components) {
                mean_power += pow(I, 2) + pow(Q, 2);
            }
        }

        mean_power /= (double)modulation_size;

        switch (modulation_size) {
        case 4: alpha = [mean_power](const vector<bool>& bits) {return (-2 * bits[0] + 1) / sqrt(mean_power);};
              break;

        case 16: alpha = [mean_power](const vector<bool>& bits) {return (-2 * bits[0] + 2 * bits[1] - 4 * bits[0] * bits[1] + 1) / sqrt(mean_power);};
               break;

        case 64: alpha = [mean_power](const vector<bool>& bits) {return (-6 * bits[0] + 2 * bits[1] - 2 * bits[2] - 4 * bits[0] * bits[1] + 4 * bits[0] * bits[2] + 4 * bits[1] * bits[2] - 8 * bits[0] * bits[1] * bits[2] + 3) / sqrt(mean_power);};
        }
    }

    vector<complex<double>> modulate(const vector<bool>& tx_bits) {
        const int bits_number = tx_bits.size();
        const int symbol_number = bits_number / bits_per_symbol;

        vector<complex<double>> tx_signal(symbol_number);

        for (int symbol = 0; symbol < symbol_number; symbol++) {
            vector<bool> bits_I(bits_per_components);
            vector<bool> bits_Q(bits_per_components);
            for (int bit = 0; bit < bits_per_components; bit += 1) {
                bits_I[bit] = tx_bits[bits_per_symbol * symbol + 2 * bit];
                bits_Q[bit] = tx_bits[bits_per_symbol * symbol + 2 * bit + 1];
            }
            tx_signal[symbol] = complex<double>(alpha(bits_I), alpha(bits_Q));
        }

        return tx_signal;
    }
};

class DemodulatorGrey : protected ModulatorGrey {
private:
    vector<vector<bool>> bits_comb;
    vector<double> components_comb;
public:
    DemodulatorGrey(int modulation_size) : ModulatorGrey(modulation_size) {
        bits_comb = vector<vector<bool>>(components_number, vector<bool>(bits_per_components));
        components_comb = vector<double>(components_number);

        for (int number = 0; number < (components_number); number++) {
            for (int bit = 0; bit < bits_per_components; bit++) {
                bits_comb[number][bit] = (number >> bit) & 1;
            }
            components_comb[number] = alpha(bits_comb[number]);
        }
    }

    vector<bool> demodulate(const vector<complex<double>>& rx_signal) {
        const int symbol_number = rx_signal.size();
        const int bits_number = symbol_number * bits_per_symbol;

        vector<bool> rx_bits(bits_number);

        for (int symbol = 0; symbol < symbol_number; symbol++) {
            double I = real(rx_signal[symbol]);
            double Q = imag(rx_signal[symbol]);

            double min_d = 1e4;
            int min_index_I = 0;
            int min_index_Q = 0;


            for (int number_I = 0; number_I < components_number; number_I++) {
                for (int number_Q = 0; number_Q < components_number; number_Q++) {
                    double d = sqrt(pow((I - components_comb[number_I]), 2) + pow((Q - components_comb[number_Q]), 2));
                    if (d < min_d) {
                        min_d = d;
                        min_index_I = number_I;
                        min_index_Q = number_Q;
                    }
                }
            }

            for (int bit = 0; bit < bits_per_components; bit++) {
                rx_bits[symbol * bits_per_symbol + 2 * bit] = bits_comb[min_index_I][bit];
                rx_bits[symbol * bits_per_symbol + 2 * bit + 1] = bits_comb[min_index_Q][bit];
            }
        }

        return rx_bits;
    }
};


static vector<bool> generate_bits_vector(int bits_number) {
    vector<bool> bits(bits_number);

    mt19937 gen(random_device{}());

    uniform_int_distribution<> dist(0, 1);

    for (int i = 0; i < bits_number; i++) {
        bits[i] = dist(gen);
    }

    return bits;
}

static double compute_ber(const vector<bool>& tx_bits, const vector<bool>& rx_bits) {
    int bits_number = tx_bits.size();
    int error = 0;
    for (int i = 0; i < bits_number; i++) {
        error += tx_bits[i] ^ rx_bits[i];
    }

    return error / (double)bits_number;
}


int main(int argc, char* argv[]) {
    vector<string> modulation_types;

    for (int i = 1; i < argc; i++) {
        modulation_types.push_back(argv[i]);
    }

    double snr_arr[21];

    for (int snr = 0; snr < 21; snr++) {
        snr_arr[snr] = snr;
    }

    for (string modulation_type : modulation_types) {
        int modulation_size;

        if (modulation_type == "QPSK") {
            modulation_size = 4;
        }
        else if (modulation_type == "QAM16") {
            modulation_size = 16;
        }
        else if (modulation_type == "QAM64") {
            modulation_size = 64;
        }

        ofstream output_file("ber_vs_snr_" + modulation_type + ".csv");

        output_file << "snr,ber\n";

        const int bits_per_symbol = static_cast<int>(log2(modulation_size));
        const int bits_number = bits_per_symbol * 1000000;

        for (double snr : snr_arr) {
            double noise_variance = pow(10.0, -snr / 10.0);

            vector<bool> tx_bits = generate_bits_vector(bits_number);

            vector<complex<double>> signal_tx = ModulatorGrey(modulation_size).modulate(tx_bits);

            vector<complex<double>> noise_signal = AWGNAdder(noise_variance).add_awgn(signal_tx);

            vector<bool> rx_bits = DemodulatorGrey(modulation_size).demodulate(noise_signal);

            double ber = compute_ber(tx_bits, rx_bits);

            output_file << snr << "," << ber << "\n";
        }

        output_file.close();
    }

    return 0;
}
