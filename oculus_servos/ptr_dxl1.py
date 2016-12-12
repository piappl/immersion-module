#!/usr/bin/env python
import serial                     # we need to import the pySerial stuff to use
import sys
from time import sleep, time
import random
import math
import json

SPEED_LIMIT = 0.5

# important AX-12 constants
AX_WRITE_DATA = 3
AX_READ_DATA = 2
SYNC_WRITE = 131 #'\0x83'


class PTR:

    def __init__(self, params, port = '/dev/Dynamixel2USB'):

        baud = params['baud']

        self.yaw_limits =   [math.pi/180*float(params['yaw']['joint_upper_limit'])  ,math.pi/180*float(params['yaw']['joint_lower_limit'])]
        self.roll_limits =  [math.pi/180*float(params['roll']['joint_upper_limit']) ,math.pi/180*float(params['roll']['joint_lower_limit'])]
        self.pitch_limits = [math.pi/180*float(params['pitch']['joint_upper_limit']),math.pi/180*float(params['pitch']['joint_lower_limit'])]

        self.servo_id = [int(params['yaw']['id']), int(params['roll']['id']), int(params['pitch']['id'])]
        self.servo_range = [math.pi/180*float(params['yaw']['servo_range']), math.pi/180*float(params['roll']['servo_range']), math.pi/180*float(params['pitch']['servo_range'])]
        self.servo_steps = [float(params['yaw']['servo_steps_number']), float(params['roll']['servo_steps_number']), float(params['pitch']['servo_steps_number'])]
        self.servo_factor = [i/j for i,j in zip(self.servo_steps, self.servo_range)]


        self.s = serial.Serial()               # create a serial port object
        self.s.baudrate = baud              # baud rate, in bits/second
        self.s.port = port           # this is whatever port your are using
        self.s.open()
        self.set_speed(100)
        print "setting zero"
        self.sendrpy(0,0,0)
        sleep(1)
        self.set_speed(300)
        print "set"

    # set register values
    def setReg(self, ID,reg,values):
        length = 3 + len(values)
        checksum = 255-((ID+length+AX_WRITE_DATA+reg+sum(values))%256)          
        self.s.write(chr(0xFF)+chr(0xFF)+chr(ID)+chr(length)+chr(AX_WRITE_DATA)+chr(reg))
        for val in values:
            self.s.write(chr(val))
        self.s.write(chr(checksum))


    def syncSetPosition(self, rpy, speed):
        START_ADDRESS = 30 #'\0x1E'
        ID = 254 #'\0xfe' #broadcast id
        goal_values = [[int(v)%256,int(v)>>8] for v in rpy]
        speed_values = [[int(v)%256,int(v)>>8] for v in speed]
        length = (4+1)*3 + 4
        data_len = 4
        checksum = 255-((ID + length + SYNC_WRITE + START_ADDRESS + data_len + sum(self.servo_id) + sum([sum(i) for i in goal_values]) + sum([sum(i) for i in speed_values])) %256)          
        command = chr(255) + chr(255) + chr(ID) + chr(length) + chr(SYNC_WRITE) + chr(START_ADDRESS) + chr(data_len)
        for i in range(len(self.servo_id)):
            command = command + chr(self.servo_id[i]) + chr(goal_values[i][0])  + chr(goal_values[i][1])  + chr(speed_values[i][0])  + chr(speed_values[i][1])

        command = command + chr(checksum)

        self.s.write(command)

    def get_rpy(self):

        current_pos = []
        for i in self.servo_id:
            tmp = self.getReg(i,36,2)
            pos = tmp[0]+(tmp[1]<<8)

            current_pos.append(pos)
        
        yaw = (current_pos[0] -self.servo_steps[0]/2)/self.servo_factor[0] 
        roll = -(current_pos[1] -self.servo_steps[1]/2)/self.servo_factor[1] 
        pitch = -(current_pos[2] -self.servo_steps[2]/2)/self.servo_factor[2] 

        return roll, pitch, yaw

    def set_yrp_reg(self, rpy, speed):
        self.syncSetPosition(rpy,speed)

    def sendrpy(self, r, p, y, sr=0, sp=0, sy=0):

        y = min(max(y, self.yaw_limits[1]), self.yaw_limits[0])
        r = min(max(r, self.roll_limits[1]), self.roll_limits[0])
        p = min(max(p, self.pitch_limits[1]), self.pitch_limits[0])

        yaw = y * self.servo_factor[0] + self.servo_steps[0]/2
        roll = -r * self.servo_factor[1] + self.servo_steps[1]/2
        pitch = - p * self.servo_factor[2] + self.servo_steps[2]/2
        speed_factor = 1
        speed = [int(sy*speed_factor), int(sr*speed_factor), int(sp*speed_factor)]
        self.set_yrp_reg([yaw, roll, pitch], speed)


    def getReg(self, index, regstart, rlength):
        self.s.flushInput()   
        checksum = 255 - ((6 + index + regstart + rlength)%256)
        self.s.write(chr(0xFF)+chr(0xFF)+chr(index)+chr(0x04)+chr(AX_READ_DATA)+chr(regstart)+chr(rlength)+chr(checksum))
        vals = list()
        sleep(0.009)
        self.s.read()   # 0xff
        self.s.read()   # 0xff
        self.s.read()   # ID
        length = ord(self.s.read()) - 2
        self.s.read()   # toss error    
        while length > 0:
            vals.append(ord(self.s.read()))
            length = length - 1
        return vals

    def set_speed(self, speed):
        #setting speed
        speed = int(speed)
        for id in self.servo_id:
            self.setReg(id,32,((speed%256),(speed>>8)))
            sleep(0.005) #sleep, otherwise it does not work


