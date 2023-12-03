import requests
import json

url = 'http://172.30.75.165:5000/post_json'

json_data = [
    {"key1": "value1"},
    {"key2": "value2"},
    {"key3": "value3"},
    {"key4": "value4"}
]

for data in json_data:
    response = requests.post(url, json=data)
    print(response.text)
