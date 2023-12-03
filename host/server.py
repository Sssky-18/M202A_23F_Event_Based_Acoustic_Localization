from flask import Flask, request, jsonify
import json
import scipy
import numpy as np
from scipy.optimize import fsolve
import datetime
from math import sqrt
import time

app = Flask(__name__)
COORD = [[-3,0], [3,1], [0,4]]
V = 1503.84

def tdoa(coord, v, delta_t):
    result = []
    # microphones
    m0 = coord[0]  # coordinates of mic 0 (x,y)
    m1 = coord[1]  # coordinates of mic 1 (x,y)
    m2 = coord[2]  # coordinates of mic 2 (x,y)

    v = v  # velocity of sound in saltwater at 12 degrees C

    # tdoa
    t1 = delta_t[0]  # tdoa between mic 0 and 1
    t2 = delta_t[1]  # tdoa between mic 0 and 2
    t3 = delta_t[2]  # tdoa between mic 1 and 2

    start = time.time()
    # solving for x and y using all 3 pair combinations

    def system0(z):
        x = z[0]
        y = z[1]
        f = np.zeros(2)
        f[0] = sqrt((x-m0[0])**2 + (y-m0[1])**2) - sqrt((x-m1[0])**2 + (y-m1[1])**2) - (t1*v)
        f[1] = sqrt((x-m0[0])**2 + (y-m0[1])**2) - sqrt((x-m2[0])**2 + (y-m2[1])**2) - (t2*v)
        return f


    a0 = fsolve(system0, [1, 1])
    print(a0)
    result.append(list(a0))


    def system1(z):
        x = z[0]
        y = z[1]
        f = np.zeros(2)
        f[0] = sqrt((x-m0[0])**2.0 + (y-m0[1])**2.0) - sqrt((x-m1[0])**2.0 + (y-m1[1])**2.0) - (t1*v)
        f[1] = sqrt((x-m1[0])**2.0 + (y-m1[1])**2.0) - sqrt((x-m2[0])**2.0 + (y-m2[1])**2.0) - (t3*v)
        return f


    a = fsolve(system1, [1, 1])
    print(a)
    result.append(list(a))

    def system2(z):
        x = z[0]
        y = z[1]
        f = np.zeros(2)
        f[0] = sqrt((x-m0[0])**2.0 + (y-m0[1])**2.0) - sqrt((x-m2[0])**2.0 + (y-m2[1])**2.0) - (t2*v)
        f[1] = sqrt((x-m1[0])**2.0 + (y-m1[1])**2.0) - sqrt((x-m2[0])**2.0 + (y-m2[1])**2.0) - (t3*v)
        return f


    a = fsolve(system2, [1, 1])
    print(a)
    result.append(list(a))

    end = time.time()
    print(end-start)
    return result

received_int = []

@app.route('/post_json', methods=['POST'])
def post_json():
    if request.method == 'POST':
        posted_data = request.get_json()
        print("Received JSON data:", posted_data)
        for value in posted_data.values():
            received_int.append(value)
        print("received:", received_int)

        if len(received_int) == 3:
            print("3 val:", received_int)
            result = tdoa(COORD,V,received_int)
            print(result)
            return jsonify({"message": "Values extracted successfully!", "result": result[0]})
        else:
            return jsonify({"wait": "Waiting for more data"})

@app.route('/get_received_json', methods=['GET'])
def get_received_json():
    return json.dumps(received_int)

if __name__ == '__main__':
    app.run(host='192.168.137.249', port=5000)
