import subprocess

from plot_ber import plot_ber

if __name__ == "__main__":
    modulations_string = input("Введите желаемые типы модуляций через запятую (QPSK, QAM16, QAM64): ").replace(" ", "")

    if not modulations_string:
        raise ValueError("Список модуляций не может быть пустым")

    modulations = modulations_string.split(",")

    for modulation in modulations:
        match modulation:
            case "QPSK" | "QAM16" | "QAM64":
              pass
            case _:
              raise ValueError(f"Неизвестный тип модуляции: {modulation}")


    subprocess.run(["./qam_modulator.exe"] + modulations, check=True)

    fig = plot_ber(modulations=modulations)

    fig.savefig("ber_vs_snr_and_variance.png")
