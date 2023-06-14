import time
import os
import asciichartpy as chart
import array as arr

clear = lambda: os.system('clear')
count = 0

def getValue(devName):
    f = open(devName, "r")
    value = ord(f.read(1)) - 48
    f.close()

    return value

values0 = arr.array('i',[])
values1 = arr.array('i',[])

while(True):
    value = getValue("/dev/GPIO17")
    values0.append(value)
    value = getValue("/dev/GPIO18")
    values1.append(value)

    count += 1

    clear()
    print("GPIO17")
    print(chart.plot(values0))
    print("GPIO18")
    print(chart.plot(values1))

    if(count > 70):
        del values0[:]
        del values1[:]
        count = 0

    time.sleep(0.2)