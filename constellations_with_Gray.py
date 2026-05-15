import numpy as np
import matplotlib.pyplot as plt


modulations_size = [4, 16, 64]

fig = plt.figure(figsize=(20, 30))
for m, M in enumerate(modulations_size):
    match M:
        case 4: alpha = lambda b: -2*b[0] + 1
        case 16: alpha =  lambda b: -2*b[0] + 2*b[1] - 4*b[0]*b[1] + 1
        case 64: alpha = lambda b: -6*b[0] + 2*b[1] - 2*b[2] - 4*b[0]*b[1] + 4*b[1]*b[2] + 4*b[0]*b[2] - 8*b[0]*b[1]*b[2] + 3
        case _: break
    bits_per_symbol = int(np.log2(M))
    bits_per_components = bits_per_symbol // 2

    components_number = 1 << bits_per_components

    bits_comb = np.zeros((components_number, bits_per_components))
    components_comb = np.zeros(components_number)

    for number in range(components_number):
        for bit in range(bits_per_components):
            bits_comb[number][bit] = (number >> (bits_per_components - 1 - bit)) & 1
        components_comb[number] = alpha(bits_comb[number])

    ax = fig.add_subplot(3, 2, bits_per_components + m)
    if M == 4:
        ax.set_title("QPSK", fontsize=20)
    else:
        ax.set_title(f'QAM{M}', fontsize=20)
    for I in range(components_number):
        for Q in range(components_number):
            bits = ''
            for b0, b1 in zip(bits_comb[I], bits_comb[Q]):
                bits += f'{int(b0)}{int(b1)}'
            ax.text(components_comb[I] + 0.1, components_comb[Q] + 0.1, bits)
            ax.scatter(components_comb[I], components_comb[Q], color="red")

    ax.set_xlabel("I", fontsize=15)
    ax.set_ylabel("Q", fontsize=15)
    ax.set_xlim(-9, 9)
    ax.set_ylim(-9, 9)

    odd_ticks = np.arange(-9, 11, 2)
    plt.xticks(odd_ticks)
    plt.yticks(odd_ticks)

    ax.axhline(y=0, color='black', linewidth=0.5)
    ax.axvline(x=0, color='black', linewidth=0.5)

    ax.grid(True)

    mean_power = np.mean(2 * components_comb**2)

    ax = fig.add_subplot(3, 2, bits_per_components + 1 + m)
    if M == 4:
        ax.set_title("QPSK", fontsize=20)
    else:
        ax.set_title(f'QAM{M}', fontsize=20)
    for I in range(components_number):
        for Q in range(components_number):
            bits = ''
            for b0, b1 in zip(bits_comb[I], bits_comb[Q]):
                bits += f'{int(b0)}{int(b1)}'
            ax.text(components_comb[I] / np.sqrt(mean_power) - 0.1, components_comb[Q] / np.sqrt(mean_power) + 0.1, bits)
            ax.scatter(components_comb[I] / np.sqrt(mean_power), components_comb[Q] / np.sqrt(mean_power), color="red")

    ax.set_xlabel("I", fontsize=15)
    ax.set_ylabel("Q", fontsize=15)
    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.5, 1.5)

    odd_ticks = np.arange(-1, 2, 2)
    plt.xticks(odd_ticks)
    plt.yticks(odd_ticks)

    ax.axhline(y=0, color='black', linewidth=0.5)
    ax.axvline(x=0, color='black', linewidth=0.5)

    ax.grid(True)

fig.savefig("constellations.png")

