import os
import serial
import time
import csv
import json
import shutil
import pandas as pd
import torch
import torch.nn as nn
import numpy as np

from Database.log import Logger  # Import Logger class from log.py


#LSTM Model
class LSTMClassifier(nn.Module):
    def __init__(self, input_dim, hidden_dim, output_dim):
        super().__init__()
        self.lstm = nn.LSTM(input_dim, hidden_dim, batch_first=True)
        self.fc = nn.Linear(hidden_dim, output_dim)
    def forward(self, x):
        _, (h_n, _) = self.lstm(x)
        return self.fc(h_n.squeeze(0))


def predict(path): # Load CSV file (shape: [timesteps, 6])
    csv_path = path
    data = pd.read_csv(csv_path, header=None).values.astype(np.float32)


    # Prepare tensor
    input_tensor = torch.tensor(data).unsqueeze(0).to(device)  # [1, 300, 6]

    # Predict
    with torch.no_grad():
        output = model(input_tensor)
        predicted = torch.argmax(output, dim=1)

        label = idx_to_label[predicted.item()]
        return label  # <-- RETURN the predicted label





if __name__ == "__main__":

    # Setup
    logger = Logger(
                 temp_folder = "Database/temp",
                 img_folder = "Database/img",
                 img_path_file = "Database/img_path.csv",
                 label_count_file="Database/label_count.json"
    )


    port = "/dev/rfcomm0"
    baudrate = 115200
    esp32 = serial.Serial(port, baudrate, timeout=2)
    time.sleep(2)

    # Labels & model
    idx_to_label = {
        0: "A",
        1: "B",
        2: "C",
        3: "D",
        4: "E"
    }

    model = LSTMClassifier(6, 128, len(idx_to_label))
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model.load_state_dict(torch.load("data_glove_lstm.pth", map_location=device))  # load weights
    model.to(device)
    model.eval()

    temp_filename = "test_input.csv"
    temp_filename_path = os.path.join(logger.temp_folder, temp_filename)

    cont = True
    while cont:
        print("Ready to record gesture...")
        logger.store_img(esp32, temp_filename_path, read_time=2)  # Reuse store_img from Logger

        predicted_label = predict(temp_filename_path)
        print(f"Predicted Label: {predicted_label}")

        true_label = input("True Label: ")

        if true_label == predicted_label or true_label.lower() == "":
            label = predicted_label
        else:
            label = true_label

        if input("Do you wanna store it? ").lower() in {"y", "yes", ""}:
            label_count = logger.get_label_count_json()
            label_count[label] = label_count.get(label, -1) + 1


            filename = f"{label}_{label_count[label]}.csv"
            final_path = os.path.join("img/", filename)

            logger.update_img_path_csv(filename, final_path)
            logger.update_label_count_json(label_count)

            # Confirm save or discard
            if input("Do you wanna confirm? ").lower() in {"yes", "y", ""}:
                logger.confirm(temp_filename_path, final_path)
            else:
                print("abort")

        # Clean temp directory
        logger.clear_temp()




        cont = input("Continue? (y/n): ").lower() in {"y", "yes", ""}

    esp32.close()
