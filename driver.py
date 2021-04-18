from serial import Serial
from time import sleep
from struct import pack
from random import randint
from math import sin, pi

MAX_ITERATIONS = 10000

def calc(iteration):
    r = 100 * sin(2 * pi * iteration / MAX_ITERATIONS)
    g = 100 * sin(2 * pi * iteration / MAX_ITERATIONS * 5)
    b = 100 * sin(2 * pi * iteration / MAX_ITERATIONS * 15)
    return int(abs(r)), int(abs(g)), int(abs(b))

port = Serial(baudrate=115200)
port.port = '/dev/ttyACM0'
port.timeout = 1
port.open()

iteration = 0

while True:
    result = calc(iteration)
    port.write(pack('<BHBBBB', 0o300, 5, result[0], result[1], result[2], 0o300))
    port.flush()
    sleep(0.01)
    iteration += 1

port.close()