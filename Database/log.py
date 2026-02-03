import os
import serial
import time
import csv
import json
import shutil

class Logger:
    def __init__(self,
                 temp_folder = "temp",
                 img_folder = "img",
                 img_path_file = "img_path.csv",
                 label_count_file="label_count.json"
                 ):
        self.temp_folder = temp_folder
        self.img_folder = img_folder
        self.img_path_file = img_path_file
        self.label_count_file = label_count_file

    def confirm(self, temp_filename_path, filename_path):
        """Move data from temp to permanent location"""
        shutil.copy(temp_filename_path, filename_path)
        shutil.copy(os.path.join(self.temp_folder, os.path.basename(self.img_path_file)), self.img_path_file)
        shutil.copy(os.path.join(self.temp_folder, os.path.basename(self.label_count_file)), self.label_count_file)

    def get_label_count_json(self):
        """Read label count from JSON or return empty dict if unavailable"""
        try:
            with open(self.label_count_file, "r") as f:
                return json.load(f)
        except (FileNotFoundError, json.JSONDecodeError):
            return {}

    def store_img(self, esp32, path, read_time=1):
        """Read MPU6050 data via serial and write to CSV"""
        with open(path, "w", newline="") as file:
            writer = csv.writer(file)
            start_time = time.time()
            while time.time() - start_time < read_time:
                if esp32.in_waiting:
                    line = esp32.readline().decode("utf-8").strip()
                    data = line.split(",")
                    if len(data) == 6:
                        writer.writerow(map(str, data))

    def update_img_path_csv(self, filename, final_path):
        """Append new file path to img_path.csv (via temp)"""
        shutil.copy(self.img_path_file, os.path.join(self.temp_folder, os.path.basename(self.img_path_file)))
        with open(os.path.join(self.temp_folder, os.path.basename(self.img_path_file)), "a", newline="") as f:
            writer = csv.writer(f)
            writer.writerow([filename, final_path])

    def update_label_count_json(self, label_count):
        """Write updated label counts to temp"""
        with open(os.path.join(self.temp_folder, os.path.basename(self.label_count_file)), "w+") as f:
            json.dump(label_count, f, indent=4)

    def clear_temp(self):
        """Clear temp directory contents"""
        shutil.rmtree(self.temp_folder)
        os.mkdir(self.temp_folder)


if __name__ == "__main__":
    logger = Logger()

    # Serial port setup (for Windows use 'COM7')
    port = "/dev/rfcomm0"
    baudrate = 115200
    esp32 = serial.Serial(port, baudrate, timeout=2)
    time.sleep(2)



    cont = True
    while cont:
        label_count = logger.get_label_count_json()

        # Ask user for label
        label = input("Enter label: ")
        label_count[label] = label_count.get(label, -1) + 1

        filename = f"{label}_{label_count[label]}.csv"
        final_path = os.path.join(logger.img_folder, filename)
        temp_path = os.path.join(logger.temp_folder, filename)

        # Record data
        logger.store_img(esp32, temp_path, read_time=2)
        logger.update_img_path_csv(filename, final_path)
        logger.update_label_count_json(label_count)

        # Confirm save or discard
        if input("Do you wanna confirm? ").lower() in {"yes", "y", ""}:
            logger.confirm(temp_path, final_path)
        else:
            print("abort")

        # Clean temp directory
        logger.clear_temp()

        cont = input("Do you wanna continue? (y/n): ").lower() in {"y", "yes", ""}

    esp32.close()
