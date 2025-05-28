import requests

url = "http://127.0.0.1:5000/sensor/conf"
new_config = {
    "scale": 2.5,
    "unit": "Kelvin"
}

response = requests.put(url, json=new_config)

if response.status_code == 200:
    print("Updated:", response.json())
elif response.status_code == 400:
    print("Config file does not exist")
elif response.status_code == 406:
    print("Invalid JSON")
else:
    print("Error:", response.status_code, response.text)
