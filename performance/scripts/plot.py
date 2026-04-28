#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import csv

# === Charger les données ===
filename = "measures/run_standard_2000.csv"
filename = filename.replace('\r', '')

samples = []
values = []

with open(filename, 'r') as f:
    reader = csv.reader(f)
    header = next(reader)

    # Les colonnes de samples commencent à l'indice 4
    samples = list(map(int, header[4:]))

    for row in reader:
        values.append(list(map(float, row[4:])))

values = np.array(values)  # shape = (n_runs, n_samples)

# === Calculs statistiques ===
means = np.mean(values, axis=0)
stds  = np.std(values, axis=0)

# === Plot ===
plt.figure()

plt.errorbar(samples, means, yerr=stds, marker='o', capsize=5)

plt.xlabel("Nombre d'échantillons")
plt.ylabel("Temps (s)")
plt.title("Runtime vs Samples (moyenne ± écart-type)")
plt.grid()

plt.show()
