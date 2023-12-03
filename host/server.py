from flask import Flask, request
import json

app = Flask(__name__)
received_strings = []

@app.route('/post_json', methods=['POST'])
def post_json():
    if request.method == 'POST':
        posted_data = request.get_json()
        print("Received JSON data:", posted_data)
        received_strings.append(posted_data)
        return "JSON data received and saved successfully!"

@app.route('/get_received_json', methods=['GET'])
def get_received_json():
    return json.dumps(received_strings)

if __name__ == '__main__':
    app.run(host='172.30.75.165', port=5000)
