import os
from datetime import datetime
from time import sleep

devices = ["192.168.178.56", "192.168.178.39"]

while True:
    time = datetime.now()
    f = open("output.csv", "a")
    f.write(time.strftime("%Y-%m-%d %H:%M:%S, "))

    for device in devices:
        response = os.system("ping -c 1 -i 0.2 " + device + " > /dev/null")
        if response == 0:
            f.write("1, ")
        else:
            f.write("0, ")
    f.write("END\r\n")
    f.close()
    sleep(300)
