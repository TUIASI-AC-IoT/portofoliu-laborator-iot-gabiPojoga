from flask import Flask, jsonify, request
import serial
import struct
import os
import json
import time

app = Flask(__name__)

CONFIG_PATH = "sensor_config.json"

try:
    ser = serial.Serial('COM7', 115200, timeout=1)
except serial.SerialException as e:
    print(f"Failed to open serial port: {e}")
    ser = None


@app.route("/sensor", methods=["GET"])
def get_sensor_value():
    if not ser or not ser.is_open:
        return jsonify({"error": "Serial port not available"}), 500

    ser.reset_input_buffer()
    ser.write(b"get_temp\n")

    try:
        # Wait until all 8 bytes are received or timeout
        data = bytearray()
        timeout = 4  # seconds
        start_time = time.time()

        while len(data) < 8 and (time.time() - start_time) < timeout:
            chunk = ser.read(8 - len(data))
            if chunk:
                data.extend(chunk)

        if len(data) != 8:
            raise ValueError("Timeout or incomplete UART response")

        temp_value = struct.unpack("<Q", data)[0]
        return jsonify({"value": temp_value}), 200

    except Exception as e:
        return jsonify({"error": f"UART error: {str(e)}"}), 500


@app.route("/sensor", methods=["POST"])
def create_config():
    if os.path.exists(CONFIG_PATH):
        return jsonify({"error": "Config already exists"}), 409

    default_config = {"scale": 1.0, "unit": "Celsius"}
    with open(CONFIG_PATH, "w") as f:
        json.dump(default_config, f)

    return jsonify({"message": "Config created", "config": default_config}), 201

@app.route("/sensor/config", methods=["PUT"])
def update_config():
    if not os.path.exists(CONFIG_PATH):
        return jsonify({"error": "Config does not exist"}), 400

    try:
        new_config = request.get_json()
        with open(CONFIG_PATH, "w") as f:
            json.dump(new_config, f)
        return jsonify({"message": "Config updated", "config": new_config}), 200
    except Exception as e:
        return jsonify({"error": f"Invalid JSON: {str(e)}"}), 406

if __name__ == "__main__":
    app.run(debug=False)
