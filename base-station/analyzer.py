import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import json
import sys


def analyze_drop(csv_file, cal_file):
    df = pd.read_csv(csv_file)
    with open(cal_file, 'r') as f:
        offsets = json.load(f)

    # Aplica calibración
    for axis in ['ax', 'ay', 'az', 'gx', 'gy', 'gz']:
        df[axis] -= offsets[axis]

    # Calcula velocidad total de rotación
    df['rot_speed'] = np.sqrt(df['gx']**2 + df['gy']**2 + df['gz']**2)

    # Convirtiendo aceleración de G's a m/s^2
    df['accel_m_s2'] = (df['az'] - 1.0) * 9.81
    # === Velocidad vertical ===
    # Integración numérica: $v_{t+1} = v_t + a_t * dt$
    df['dt'] = df['time'].diff().fillna(0)
    df['velocity_z'] = (df['accel_m_s2'] * df['dt']).cumsum()

    # Gráfica
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

    ax1.plot(df['time'], df['velocity_z'], label='Velocidad vertical', color='blue')
    ax1.set_ylabel('Velocidad (m/s)')
    ax1.set_title('Velocidad calculada')
    ax1.grid(True)

    ax2.plot(df['time'], df['rot_speed'], label='Velocidad de rotación', color='red')
    ax2.set_ylabel('Rotación (deg/s)')
    ax2.set_xlabel('Tiempo (s)')
    ax2.set_title('Magnitud total de rotación')
    ax2.grid(True)

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    # Uso: python analyzer.py raw_data_123.csv offsets_123.json
    if len(sys.argv) > 2:
        analyze_drop(sys.argv[1], sys.argv[2])
    else:
        print("Indica los archivos .csv y .json")
