from flask import Flask, request, jsonify
import json
import scipy
import numpy as np
from scipy.optimize import fsolve
import datetime
from math import sqrt
import time
import scipy.optimize as opt

# receive JSON 
# {id}
# {event_ts}
# {sync_ts}

app = Flask(__name__)
COORD = [[0,0], [-0.39,0.26], [-0.74,0]]
V = 340
F = 44100

# calculate the delta t between node 0&1, 0&2, 1&2
def calculate_delta_t(coord , v, sync_et_list,sync_ts_list):
    delta_t = []
    m0 = coord[0]  # coordinates of mic 0 (x,y)
    m1 = coord[1]  # coordinates of mic 1 (x,y)
    m2 = coord[2]  # coordinates of mic 2 (x,y)

    d01 = sqrt((m0[0]-m1[0])**2.0 + (m0[1]-m1[1])**2.0)/v
    delta_t01 = (sync_ts_list[0][0] - sync_et_list[0])/F + (d01) - (sync_ts_list[1][0] - sync_et_list[1])/F
    delta_t.append(delta_t01)

    d02 = sqrt((m0[0]-m2[0])**2.0 + (m0[1]-m2[1])**2.0)/v
    delta_t02 = (sync_ts_list[0][0] - sync_et_list[0])/F + (d02) - (sync_ts_list[2][0] - sync_et_list[2])/F
    
    delta_t.append(delta_t02)

    d12 = sqrt((m1[0]-m2[0])**2.0 + (m1[1]-m2[1])**2.0)/v
    delta_t12 = (sync_ts_list[1][1] - sync_et_list[1])/F + (d12) - (sync_ts_list[2][1] - sync_et_list[2])/F
    delta_t.append(delta_t12)

    return delta_t

# Form the optimization functions 
def functions(x0, y0, x1, y1, x2, y2, d01, d02, d12):
    """ Given observers at (x0, y0), (x1, y1), (x2, y2) and TDOA between observers d01, d02, d12, this closure
        returns a function that evaluates the system of three hyperbolae for given event x, y.
    """
    def fn(args):
        x, y = args
        a = np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.)) - np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.)) - d01
        b = np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.)) - d02
        c = np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.)) - d12
        return [a, b, c]
    return fn

# specify the Jacobian matrix of this system. This is just a collection of the partial derivatives of each function with respect to each independent variable
def jacobian(x0, y0, x1, y1, x2, y2, d01, d02, d12):
    def fn(args):
        x, y = args
        adx = (x - x1) / np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.)) - (x - x0) / np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.))
        bdx = (x - x2) / np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - (x - x0) / np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.))
        cdx = (x - x2) / np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - (x - x1) / np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.))
        ady = (y - y1) / np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.)) - (y - y0) / np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.))
        bdy = (y - y2) / np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - (y - y0) / np.sqrt(np.power(x - x0, 2.) + np.power(y - y0, 2.))
        cdy = (y - y2) / np.sqrt(np.power(x - x2, 2.) + np.power(y - y2, 2.)) - (y - y1) / np.sqrt(np.power(x - x1, 2.) + np.power(y - y1, 2.))

        return [
            [adx, ady],
            [bdx, bdy],
            [cdx, cdy]
        ]
    return fn

'''
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
'''

# the TDOA process 
def tdoa(measurements, V, delta_t):
    '''
    measurements is the COORD given above
    V is the propagation speed
    delta_t is calculated by the above function 
    '''
    xp = np.mean([x for x,y in measurements])
    yp = np.mean([y for x,y in measurements])

    x0, y0, = measurements[0]
    x1, y1 = measurements[1]
    x2, y2 = measurements[2]

    F = functions(x0, y0, x1, y1, x2, y2, delta_t[0] * V, delta_t[1] * V, delta_t[2] * V)
    J = jacobian(x0, y0, x1, y1, x2, y2, delta_t[0] * V, delta_t[1] * V, delta_t[2] * V)

    result,_  = opt.leastsq(F, x0=[xp, yp], Dfun=J)
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
            real_receive_length = 0
            print("3 val:", sync_ts_list)
            delta_t = calculate_delta_t(COORD, V, sync_et_list, sync_ts_list)
            result = tdoa(COORD,V,delta_t)
            print("Result:", result)
            return jsonify({"message": "Values extracted successfully!", "result": list(result)}) # np array cannot jsonify, so convert to list 
        else:
            return jsonify({"wait": "Waiting for more data"})

@app.route('/get_received_json', methods=['GET'])
def get_received_json():
    return json.dumps(sync_ts_list)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
