import serial
import time
import csv
import json

# --- Config ---
PORT = '/dev/ttyACM0'
BAUD = 115200
FILE_PREFIX = int(time.time())


def run_logger():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        raw_filename = f"raw_data_{FILE_PREFIX}.csv"
        cal_filename = f"offsets_{FILE_PREFIX}.json"

        print(f"Conectado. Guardando datos en {raw_filename}")
        print("================================================================================")
        print("1. Calibración - Mantén el sategglite quieto por 5 segundos...")

        cal_samples = []
        start_cal = time.time()

        with open(raw_filename, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['time', 'ax', 'ay', 'az', 'gx', 'gy', 'gz'])

            while True:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if not line or "Time" in line:
                    continue

                try:
                    data = [float(x) for x in line.split(',')]
                    writer.writerow(data)

                    # Muestrea datos de calibración por 5 segundos
                    if time.time() - start_cal < 5:
                        cal_samples.append(data)
                    elif cal_samples:
                        # Calcula y guarda los offsets
                        avg = [sum(col)/len(col) for col in zip(*cal_samples)]
                        offsets = {
                            'ax': avg[1], 'ay': avg[2], 'az': avg[3] - 1.0,
                            'gx': avg[4], 'gy': avg[5], 'gz': avg[6]
                        }
                        with open(cal_filename, 'w') as jf:
                            json.dump(offsets, jf)
                        # Limpia para que no se repitan los datos
                        cal_samples = []
                        print(f"Calibración guradada en {cal_filename}")
                        print("================================================================================")
                        print("2. Listo para tirar. Presiona Ctrl+C para detener loggeo.")

                except (ValueError, IndexError):
                    continue

    except KeyboardInterrupt:
        print(f"\nLoggeo terminado. Archivo raw: {raw_filename}")


if __name__ == "__main__":
    run_logger()
