import time

f0 = open("/dev/GPIODriver0", "r")
f1 = open("/dev/GPIODriver1", "r")

while(True):
    value0 = f0.read(1)
    value1 = f1.read(1)

    print(value0)
    print(value1)

    time.sleep(1)