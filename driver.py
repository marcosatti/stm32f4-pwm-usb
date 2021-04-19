from serial import Serial
from time import sleep
from struct import pack
from random import randint
from math import sin, pi
import asyncio
import signal
import time;


MAX_ITERATIONS = 10000

stop = False
port = None


def sigint_handler(signum, frame):
    global stop
    print('Quiting')
    stop = True


def calc(iteration):
    r = 100 * sin(2 * pi * iteration / MAX_ITERATIONS)
    g = 100 * sin(2 * pi * iteration / MAX_ITERATIONS * 5)
    b = 100 * sin(2 * pi * iteration / MAX_ITERATIONS * 15)
    return int(abs(r)), int(abs(g)), int(abs(b))


async def main():
    global port
    port = Serial(baudrate=115200)
    port.port = '/dev/ttyACM0'
    port.open()

    read_main_task = read_main()
    write_main_task = write_main()
    await asyncio.gather(read_main_task, write_main_task)


async def read_main():
    time_instant = time.time()

    while not stop:
        # Poll for bytes in the buffer, then try to read them.
        if port.in_waiting > 0:
            message = port.read_until()
            message = message.decode('utf-8')
            duration = time.time() - time_instant
            print(f'[{duration:.3f}] ' + message, end='')
        await asyncio.sleep(0.1)


async def write_main():
    iteration = 0

    while not stop:
        result = calc(iteration)
        port.write(pack('<BHBBBB', 0o300, 5, result[0], result[1], result[2], 0o300))
        port.flush()
        iteration += 1
        await asyncio.sleep(0.01)


signal.signal(signal.SIGINT, sigint_handler)
asyncio.run(main())
port.close()
