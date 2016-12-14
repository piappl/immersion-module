#!/usr/bin/env python
import serial                     # we need to import the pySerial stuff to use
import sys
from time import sleep, time
import random
import math
import json
import dxl2


class PTR:

    def __init__(self, params, port = '/dev/ttyUSB0'):

        self.dynamixel = dxl2.Dynamixel(params['port'], int(params['baud']))

        self.yaw_limits =   [math.pi/180*float(params['yaw']['joint_upper_limit'])  ,math.pi/180*float(params['yaw']['joint_lower_limit'])]
        self.roll_limits =  [math.pi/180*float(params['roll']['joint_upper_limit']) ,math.pi/180*float(params['roll']['joint_lower_limit'])]
        self.pitch_limits = [math.pi/180*float(params['pitch']['joint_upper_limit']),math.pi/180*float(params['pitch']['joint_lower_limit'])]

        self.servo_id = [int(params['yaw']['id']), int(params['roll']['id']), int(params['pitch']['id'])]
        self.servo_range = [math.pi/180*float(params['yaw']['servo_range']), math.pi/180*float(params['roll']['servo_range']), math.pi/180*float(params['pitch']['servo_range'])]
        self.servo_steps = [float(params['yaw']['servo_steps_number']), float(params['roll']['servo_steps_number']), float(params['pitch']['servo_steps_number'])]
        self.servo_factor = [i/j for i,j in zip(self.servo_steps, self.servo_range)]


        self.prev_request = None

        #self.set_speed(100)
        print "setting zero"
        self.sendrpy(0,0,0)
        sleep(1)
        #self.set_speed(300)
        print "set"


    def get_rpy(self):

        current_pos = self.dynamixel.get_position(self.servo_id)
        #for i in self.servo_id:
        #    current_pos.append(self.dynamixel.get_position([i])[0])
        
        yaw = (current_pos[0] -self.servo_steps[0]/2)/self.servo_factor[0] 
        roll = -(current_pos[1] -self.servo_steps[1]/2)/self.servo_factor[1] 
        pitch = -(current_pos[2] -self.servo_steps[2]/2)/self.servo_factor[2] 

        return roll, pitch, yaw


    def sendrpy(self, r, p, y, sr=0, sp=0, sy=0):

        current_request = [r,p,y]
        
        if self.prev_request != None:
            for i in range(len(current_request)):
                if abs(current_request[i] - self.prev_request[i]) > np.pi/4:
                    print "Jump detected.\nRequested values rpy: %s \nPreviously requested values: %s \naborting" %(str(current_request), str(self.prev_request))
                    return

        self.prev_request = current_request

        y = min(max(y, self.yaw_limits[1]), self.yaw_limits[0])
        r = min(max(r, self.roll_limits[1]), self.roll_limits[0])
        p = min(max(p, self.pitch_limits[1]), self.pitch_limits[0])

        yaw = y * self.servo_factor[0] + self.servo_steps[0]/2
        roll = -r * self.servo_factor[1] + self.servo_steps[1]/2
        pitch = - p * self.servo_factor[2] + self.servo_steps[2]/2
        speed = [int(sy), int(sr), int(sp)]
        #self.set_yrp_reg([yaw, roll, pitch], speed)
        self.dynamixel.set_speed(self.servo_id, [int(s) for s in speed])
        self.dynamixel.set_goal(self.servo_id,[int(yaw), int(roll), int(pitch)])


