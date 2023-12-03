import requests

url = 'http://172.30.75.165:5000/post_string'
data = 'Hello, this is a test string!'

response = requests.post(url, data=data)
print(response.text)
