import csv

import matplotlib.pyplot as plt

def plot_ber(modulations):
    snr_to_variance = lambda snr: 10**(-snr/10)

    fig = plt.figure(figsize=(30, 16))
    ax_snr = fig.add_subplot(121)
    ax_var = fig.add_subplot(122)

    colors = ["green", "blue", "red"]

    for modulation, color in zip(modulations, colors):
        df = {"snr" : [],
              "ber" : [],}

        with open(f"ber_vs_snr_{modulation}.csv", 'r') as file:
            reader = csv.DictReader(file)
            for row in reader:
                snr = float(row["snr"])
                ber = float(row["ber"])
                if ber >= 1e-6:
                    df["snr"].append(snr)
                    df["ber"].append(ber)
                else:
                    break

        df["variance"] = list(map(snr_to_variance, df["snr"]))

        

        ax_snr.semilogy(df["snr"], df["ber"], color=color, label=modulation)

        ax_var.semilogy(df["variance"], df["ber"], color=color, label=modulation)
    
    ax_snr.tick_params(axis='both', labelsize=12)
    ax_var.tick_params(axis='both', labelsize=12)

    ax_snr.set_ylim(bottom=1e-6)
    ax_var.set_ylim(bottom=1e-6)

    ax_snr.set_title("BER vs SNR", fontsize=20)
    ax_snr.set_xlabel("SNR, dB", fontsize=18)
    ax_snr.set_ylabel("BER", fontsize=18)
    ax_snr.legend(fontsize=15)
    ax_snr.grid()
    ax_var.set_title("BER vs noise variance", fontsize=20)
    ax_var.set_xlabel("noise variance", fontsize=18)
    ax_var.set_ylabel("BER", fontsize=18)
    ax_var.legend(fontsize=15)
    ax_var.grid()

    return fig
