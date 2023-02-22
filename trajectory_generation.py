#!/usr/bin/env python

from math import pi, sqrt, atan2, cos, sin
import numpy as np

import rospy
import tf
from std_msgs.msg import Empty
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Twist, Pose2D

#keep c = 10, V = 0.5 - 0.3 
#change kp first 

class Turtlebot():
    def __init__(self):
        rospy.init_node("turtlebot_move")
        rospy.loginfo("Press Ctrl + C to terminate")
        self.vel_pub = rospy.Publisher("cmd_vel_mux/input/navi", Twist, queue_size=10)
        self.rate = rospy.Rate(10)

        # reset odometry to zero
        self.reset_pub = rospy.Publisher("mobile_base/commands/reset_odometry", Empty, queue_size=10)
        for i in range(10):
            self.reset_pub.publish(Empty())
            self.rate.sleep()
        
        # subscribe to odometry
        self.pose = Pose2D()
        self.logging_counter = 0
        self.trajectory = list()
        self.odom_sub = rospy.Subscriber("odom", Odometry, self.odom_callback)

        self.previous_point = [0,0]
        self.previous_velocity= [0,0]
        self.vel_steering = 0.5
        self.vel = Twist()

        try:
            self.run()
        except rospy.ROSInterruptException:
            rospy.loginfo("Action terminated.")
        finally:
            # save trajectory into csv file
            np.savetxt('trajectory.csv', np.array(self.trajectory), fmt='%f', delimiter=',')
        
        

    def run(self):
        waypoints = [[0.5, 0], [0.5, -0.5], [1, -0.5], [1, 0], [1, 0.5],\
                      [1.5, 0.5], [1.5, 0], [1.5, -0.5], [1, -0.5], [1, 0],\
                      [1, 0.5], [0.5, 0.5], [0.5, 0], [0, 0], [0, 0]]
        for i in range(len(waypoints)-1):
            self.move_to_point(waypoints[i], waypoints[i+1])


    def move_to_point(self, current_waypoint, next_waypoint):
        # generate polynomial trajectory and move to current_waypoint
        
        # next_waypoint is to help determine the velocity to pass current_waypoint
        
        #Set boundaries condition for position 
        #for x
        Xstart = self.previous_point[0]
        Xend = current_waypoint[0]
        #for y
        Ystart = self.previous_point[1]
        Yend = current_waypoint[1]

        #Set boundaries condition for velocity # Decompose to Vx and Vy component using atan with position devitation
        # previous and next
        # deviation
        delta_x = Xend - Xstart
        delta_y = Yend - Ystart
        delta_theta = atan2(delta_y,delta_x)
        #for x
        VXstart = self.previous_velocity[0]
        # To be corrected 
        VXend = self.vel_steering*cos(delta_theta)
        #for y
        VYstart = self.previous_velocity[1]
        # to be corrected 
        VYend = self.vel_steering*sin(delta_theta)
        
        T=1 #1
        #compute coeff for x
        Bx = self.polynomial_time_scaling_3rd_order(Xstart, VXstart, Xend, VXend, T)
        #compute coeff for y
        By = self.polynomial_time_scaling_3rd_order(Ystart, VYstart, Yend, VYend, T)
 
        c=10 #10
        kp = 8 #8
        # we can set vel = self.vel
        for i in range(c*T):
            t = 0.1*i
            
            #linear velocity
            #compnent v_endx
            VXend = np.dot(np.array([[3*(t**2), 2*t, 1, 0]]),Bx)
            #component v_endy
            VYend = np.dot(np.array([3*(t**2), 2*t, 1, 0]),By)             

            self.vel.linear.x = sqrt(VXend**2 + VYend**2)
            
            #angular velocity
            #compute theta deviation: atan2(v_endy,v_endx)
            theta_deviation = atan2(VYend,VXend)
            #compute error: theta deviation - self.pose.theta
            if theta_deviation < -pi:
                theta_deviation = theta_deviation + 2*pi
            elif theta_deviation > pi:
                theta_deviation = theta_deviation - 2*pi
            else:
                theta_deviation = theta_deviation

            theta_error = theta_deviation - self.pose.theta
            # CCW correction for error>pi or error<-pi 

            #set kp
            self.vel.angular.z = kp*theta_error
            #publish velocities
            self.vel_pub.publish(self.vel)
            self.rate.sleep()
            #update the previous values
        self.previous_point = [current_waypoint[0], current_waypoint[1]]
        self.previous_velocity = [VXend, VYend]
            
        pass


    def polynomial_time_scaling_3rd_order(self, p_start, v_start, p_end, v_end, T):
        # input: p,v: position and velocity of start/end point
        #        T: the desired time to complete this segment of trajectory (in second)
        # output: the coefficients of this polynomial
        A = np.array([[p_start], [p_end], [v_start], [v_end]])
        X = np.array([[0,0,0,1],[T**3,T**2,T,1],[0,0,1,0],[3*T**2,2*T,1,0]])
        X_inv = np.linalg.inv(X)
        B = np.dot(X_inv, A)
        return B
        pass


    def odom_callback(self, msg):
        # get pose = (x, y, theta) from odometry topic
        quarternion = [msg.pose.pose.orientation.x, msg.pose.pose.orientation.y,\
                    msg.pose.pose.orientation.z, msg.pose.pose.orientation.w]
        (roll, pitch, yaw) = tf.transformations.euler_from_quaternion(quarternion)
        self.pose.theta = yaw
        self.pose.x = msg.pose.pose.position.x
        self.pose.y = msg.pose.pose.position.y

        # logging once every 100 times (Gazebo runs at 1000Hz; we save it at 10Hz)
        self.logging_counter += 1
        if self.logging_counter == 100:
            self.logging_counter = 0
            self.trajectory.append([self.pose.x, self.pose.y])  # save trajectory
            rospy.loginfo("odom: x=" + str(self.pose.x) +\
                ";  y=" + str(self.pose.y) + ";  theta=" + str(yaw))


if __name__ == '__main__':
    whatever = Turtlebot()
