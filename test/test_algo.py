from flask import Flask, request, jsonify
import json
import scipy
import numpy as np
from scipy.optimize import fsolve
import datetime
from math import sqrt
import time

COORD = [[0,0], [-0.39,0.26], [-0.74,0]]
V = 340
F = 44100
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

json_data = [
    {
    "id": 0,
    "event_ts": 10828184,
    "sync_ts": [10837460, 10849392, 10865216]
  },
  {
    "id": 1,
    "event_ts": 82892316,
    "sync_ts": [82901653, 82913456, 82929300]
  },
  {
    "id": 2,
    "event_ts": 86157589,
    "sync_ts": [86166957, 86178763, 86194541]
  }
]

sync_et_list = [10828184, 82892316, 86157589]
sync_ts_list = [[10837460, 10849392, 10865216],[82901653, 82913456, 82929300], [86166957, 86178763, 86194541]] 

delta_t = calculate_delta_t(COORD, V, sync_et_list, sync_ts_list)
print("delta t:", delta_t)

#result = tdoa(COORD,V,delta_t)
#print("Result:", result)

import numpy as np

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

measurements = [
    [0,0], 
     [-0.39,0.26], 
     [-0.74,0]]

xp = np.mean([x for x,y in measurements])
yp = np.mean([y for x,y in measurements])


import scipy.optimize as opt

x0, y0, = measurements[0]
x1, y1 = measurements[1]
x2, y2 = measurements[2]

F = functions(x0, y0, x1, y1, x2, y2, delta_t[0] * V, delta_t[1] * V, delta_t[2] * V)
J = jacobian(x0, y0, x1, y1, x2, y2, delta_t[0] * V, delta_t[1] * V, delta_t[2] * V)

x, y = opt.leastsq(F, x0=[xp, yp], Dfun=J)
print(x,y)