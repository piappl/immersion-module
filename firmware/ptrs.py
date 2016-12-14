#!/usr/bin/python

import socket
import json
import time
import sys
import select
from numpy import sin, cos



if __name__ == "__main__":

    if len(sys.argv)<2:
        print 'configuration file path missing!'
        exit()
    


    UDP_IP = "0.0.0.0"
    UDP_CLIENT_PORT = 5005
    UDP_IMU_PORT = 5006


    client_sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

    client_sock.setblocking(0) 
    client_sock.bind((UDP_IP, UDP_CLIENT_PORT))

    imu_sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
    imu_sock.setblocking(0) 
    imu_sock.bind((UDP_IP, UDP_IMU_PORT))

    parameters = json.load(file(sys.argv[1])) 

    dxl_ver = parameters['dxl_version']

    if dxl_ver == '1':
        import ptr_dxl1 as ptr
    elif dxl_ver == '2':
        import ptr_dxl2 as ptr
    else:
        print "dxl version not recognized!"
        exit(0)

    p=ptr.PTR(parameters)

    print "entering main loop"
    last_time = time.time()-1
    d = {'roll':0, 'pitch':0,'yaw': 0}
    imu_d = {'roll':0, 'pitch':0,'yaw': 0}
    while True:
        rtr, rtw, ie = select.select([imu_sock, client_sock],[],[],0)

        if client_sock in rtr:
            client_data, addr = client_sock.recvfrom(1024) # buffer size is 1024 bytes
            d= json.loads(client_data)#

        if imu_sock in rtr:
            imu_data, addr = imu_sock.recvfrom(1024) # buffer size is 1024 bytes
            imu_d= json.loads(imu_data)#

        cr,cp,cy = p.get_rpy()

        print '\x1B[31;1mcurrent   \t %+.2f, %+.2f, %+.2f \x1B[0m' % (cr, cp, cy)
        imu_r = imu_d['roll']*cos(cy) + imu_d['pitch']*sin(cy)
        imu_p = imu_d['pitch']*cos(cy) - imu_d['roll']*sin(cy)

        roll = d['roll']  - imu_r
        pitch = d['pitch'] - imu_p
        yaw = d['yaw']   - imu_d['yaw'] 
        print '\x1B[32;1mrequested \t %+.2f, %+.2f, %+.2f \x1B[0m' % (roll, pitch, yaw)
        current_time = time.time()
        lapsed = current_time - last_time
        factor = 8
        sr = min(abs( roll - cr)/lapsed*factor,1023)
        sp = min(abs( pitch - cp)/lapsed*factor,1023)
        sy = min(abs( yaw - cy)/lapsed*factor,1023)
        #sr = abs( roll - cr)/0.04
        #sp = abs( pitch - cp)/0.04
        #sy = abs( yaw - cy)/0.04
        print '\x1B[33;1mspeed     \t %+.2f, %+.2f, %+.2f \x1B[0m' % (sr, sp, sy)
        last_time = current_time

        p.sendrpy(float(roll),float(pitch),float(yaw), sr, sp, sy)
	#print "Send"
        time.sleep(0.01)
