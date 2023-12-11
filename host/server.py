from flask import Flask, request, jsonify
import json
import scipy
import numpy as np
from scipy.optimize import fsolve
import datetime
from math import sqrt
import time

# receive JSON 
# {id}
# {event_ts}
# {sync_ts}

app = Flask(__name__)
COORD = [[0,0], [-0.39,0.26], [-0.74,0]]
V = 340
def calculate_delta_t(coord , v, sync_et_list,sync_ts_list):
    delta_t = []
    m0 = coord[0]  # coordinates of mic 0 (x,y)
    m1 = coord[1]  # coordinates of mic 1 (x,y)
    m2 = coord[2]  # coordinates of mic 2 (x,y)

    d01 = sqrt((m0[0]-m1[0])**2.0 + (m0[1]-m1[1])**2.0)/v
    delta_t01 = (sync_ts_list[0][0] - sync_et_list[0]) + (d01/v) - (sync_ts_list[1][0] - sync_et_list[1])
    delta_t.append(delta_t01)

    d02 = sqrt((m0[0]-m2[0])**2.0 + (m0[1]-m2[1])**2.0)/v
    delta_t02 = (sync_ts_list[0][0] - sync_et_list[0]) + (d02/v) - (sync_ts_list[2][0] - sync_et_list[2])
    delta_t.append(delta_t02)

    d12 = sqrt((m1[0]-m2[0])**2.0 + (m1[1]-m2[1])**2.0)/v
    delta_t12 = (sync_ts_list[1][1] - sync_et_list[1]) + (d12/v) - (sync_ts_list[2][1] - sync_et_list[2])
    delta_t.append(delta_t12)

    return delta_t


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
sync_et_list = [0]*3
print(sync_et_list)
sync_ts_list = [[],[],[]]
print(sync_ts_list)

real_receive_length=0
@app.route('/post_json', methods=['POST'])
def post_json():
    global real_receive_length
    if request.method == 'POST':
        posted_data = request.get_json()
        print("Received JSON data:", posted_data)
        id = posted_data.get('id')
        event_ts = posted_data.get('event_ts')
        sync_ts = posted_data.get('sync_ts')
        sync_et_list[id] = event_ts
        sync_ts_list[id] = sync_ts
        print("received:", sync_ts_list)
        real_receive_length=real_receive_length+1
        if real_receive_length == 3:
            print("3 val:", sync_ts_list)
            delta_t = calculate_delta_t(COORD, V, sync_et_list, sync_ts_list)
            result = tdoa(COORD,V,delta_t)
            print("Result:", result)
            return jsonify({"message": "Values extracted successfully!", "result": result[0]})
        else:
            return jsonify({"wait": "Waiting for more data"})

@app.route('/get_received_json', methods=['GET'])
def get_received_json():
    return json.dumps(sync_ts_list)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
