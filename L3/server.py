import io
from flask import Flask, send_file
import os.path
import re

pattern = '#define BUILD_NUMBER\s+"([^"]+)"'
app = Flask(__name__)

@app.route('/firmware.bin')
def firm():
    with open(".pio\\build\\esp-wrover-kit\\firmware.bin", 'rb') as bites:
        print(bites)
        return send_file(
                     io.BytesIO(bites.read()),
                     mimetype='application/octet-stream'
               )

@app.route("/")
def hello():
    return "Hello World!"

@app.route("/version")
def get_version():
    with open("include\\version.h", 'r') as version:
        match = re.search(pattern, version.read())
        if match:
            version_short = match.group(1)
            print(f"Version short = {version_short}\n")
        else:
            version_short = "Unknown"
            print(f"Version short not found in the version file")
        return version_short#, 200, {'Content-Type': 'text/plain'}
        

if __name__ == '__main__':
    app.run(host='0.0.0.0', ssl_context=('ca_cert.pem', 'ca_key.pem'), debug=True)