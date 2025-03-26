import serial
import time
import csv
import json

port = "/dev/ttyUSB0"
baudrate = 115200

ser = serial.Serial(port, baudrate, timeout=1)  # Use the correct port
time.sleep(2)                                   # Allow time for serial connection




try:
    with open("label_count.json", "r") as f:
        label_count = json.load(f)                     # Convert JSON to dictionary
except (FileNotFoundError, json.JSONDecodeError):
    label_count = {}                                   # Initialize if not found

label_count_copy = label_count.copy()

#while 1:

label = input("Enter label: ")
label_count_copy[label] = label_count_copy.get(label, -1) + 1


filename = "img/"+str(label)+"_"+str(label_count_copy[label])
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

with open("img_path.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow([str(label) + "_" + str(label_count_copy[label]), filename])

with open("label_count.json", "w+") as f:
    json.dump(label_count_copy, f, indent = 4)












ser.close()

