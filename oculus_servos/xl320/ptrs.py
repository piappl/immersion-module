#!/usr/bin/python

import socket
import json
import ptr
import time
import sys



if __name__ == "__main__":

    if len(sys.argv)<2:
        print 'configuration file path missing!'
        exit()
    

    UDP_IP = "127.0.0.1"
    UDP_PORT = 5005
    if len(sys.argv) > 2:
        UDP_PORT = int(sys.argv[2])

    sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
    sock.bind((UDP_IP, UDP_PORT))
    p=ptr.PTR(json.load(file(sys.argv[1])))

    print "entering main loop"
    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        d= json.loads(data)#
        cr,cp,cy = p.get_rpy()

        print 'requested',d['roll'], d['pitch'], d['yaw']
        sr = abs(d['roll'] - cr)/0.02
        sp = abs(d['pitch'] - cp)/0.02
        sy = abs(d['yaw'] - cy)/0.02

        p.sendrpy(float(d['roll']),float(d['pitch']),float(d['yaw']))#, sr, sp, sy)
        time.sleep(0.001)
