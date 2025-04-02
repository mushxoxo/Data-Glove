import os
import serial
import time
import csv
import json
import shutil

port = "/dev/rfcomm0"  # 'COM7' for windows
baudrate = 115200

esp32 = serial.Serial(port, baudrate, timeout=2)  # Use the correct port
time.sleep(2)  # Allow time for serial connection


def confirm():
    shutil.copy(temp_filename_path, filename_path)
    shutil.copy("temp/img_path.csv", "img_path.csv")
    shutil.copy("temp/label_count.json", "label_count.json")


def get_label_count_json():
    """Read label_count.json and return a dictionary"""
    try:
        with open("label_count.json", "r") as f:
            label_count = json.load(f)  # Convert JSON to dictionary
    except (FileNotFoundError, json.JSONDecodeError):
        label_count = {}  # Initialize if not found

    return label_count


def store_img(filename, filename_path, read_time=1):  # Store in temp
    """Store MPU6050 readings to /temp/filename
    Parameters: filename, filename_path, read_time=1"""
    with open(filename_path, "w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["A_0", "A_1", "A_2", "G_0", "G_1", "G_2"])  # Column headers

        start_time = time.time()
        while time.time() - start_time < read_time:  # Collect data for n seconds
            if esp32.in_waiting:
                line = esp32.readline().decode("utf-8").strip()
                data = line.split(",")
                if len(data) == 6:
                    writer.writerow(map(str, data))


def update_img_path_csv(filename, filename_path): # Store in temp
    shutil.copy("img_path.csv", "temp/img_path.csv")
    with open("temp/img_path.csv", "a", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([filename, filename_path])



def update_label_count_json(label_count):
    with open("temp/label_count.json", "w+") as f:
        json.dump(label_count, f, indent=4)

def clear_temp():
    folder = "temp"
    shutil.rmtree(folder)  # Deletes everything inside, including subdirectories
    os.mkdir(folder)  # Recreates the empty folder


cont = 1
while cont:

    label_count = get_label_count_json()

    label = input("Enter label: ")


    label_count[label] = label_count.get(label, -1) + 1
    filename = str(label) + "_" + str(label_count[label]) + ".csv"
    filename_path = "img/" + filename
    temp_filename_path = "temp/" + filename

    store_img(filename, temp_filename_path, read_time = 2)
    update_img_path_csv(filename, filename_path)
    update_label_count_json(label_count)


    if input("Do you wanna confirm? ").lower() in {'yes', 'y', ''}:
        confirm()
    else:
        print("abort")

    clear_temp()  # Clear temp folder

    cont = input("Do you wanna continue? (y/n): ").lower() in {'y', 'yes', ''}


esp32.close()
