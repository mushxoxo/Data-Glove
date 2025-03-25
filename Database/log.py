import serial
import time
import csv

ser = serial.Serial('COM7', 115200, timeout=1)  # Use the correct port
time.sleep(2)  # Allow time for serial connection

filename = "mpu_data.csv"
with open(filename, "w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["A_0", "A_1", "A_2", "G_0", "G_1", "G_2"])  # Column headers

    start_time = time.time()
    while time.time() - start_time < 1:  # Collect data for 1 second
        if ser.in_waiting:
            line = ser.readline().decode('utf-8').strip()
            data = line.split(",")
            if len(data) == 6:
                writer.writerow(map(int, data))

print(f"Data saved in {filename}")
ser.close()

