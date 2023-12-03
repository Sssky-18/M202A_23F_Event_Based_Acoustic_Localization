import requests
import json
from math import sqrt

url = 'http://192.168.137.249:5000/post_json'
'''
def calculate_t():

    m0 = [-3, 0]  # coordinates of mic 0 (x,y)
    m1 = [3, 1]  # coordinates of mic 1 (x,y)
    m2 = [0, 4]  # coordinates of mic 2 (x,y)

    v = 1503.84  # velocity of sound in saltwater at 12 degrees C

    # arrival times (pythagorean theorem for model with set point source as prompt)
    xs = float(input('Source x: '))
    ys = float(input('Source y: '))
    a0 = sqrt((-3.0-xs)**2.0 + (0.0-ys)**2.0)/v  # arrival time of mic 0
    a1 = sqrt((3.0-xs)**2.0 + (1.0-ys)**2.0)/v  # arrival time of mic 1
    a2 = sqrt((0.0-xs)**2.0 + (4.0-ys)**2.0)/v  # arrival time of mic 2

    # tdoa
    t1 = a0 - a1  # tdoa between mic 0 and 1
    t2 = a0 - a2  # tdoa between mic 0 and 2
    t3 = a1 - a2  # tdoa between mic 1 and 2

    return [t1,t2,t3]
'''
#t = calculate_t()

json_data = [
    {
    "id": 1,
    "event_ts": 0,
    "sync_ts": [1.5, 1.6, 1.7]
  },
  {
    "id": 2,
    "event_ts": 0,
    "sync_ts": [1.8, 1.9, 2.0]
  },
  {
    "id": 3,
    "event_ts": 0,
    "sync_ts": [2.5, 2.6, 2.7]
  }
]

for data in json_data:
    response = requests.post(url, json=data)
    print(response.text)
