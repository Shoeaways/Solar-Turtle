import serial,time

if __name__ == '__main__':
    print('Running. Press CTRL-C to exit.')
    with serial.Serial("/dev/ttyACM0", 9600, timeout=1) as arduino:
        time.sleep(0.1)
        if arduino.isOpen():
            print("Solar Turtle connected!".format(arduino.port))
            try:
                while True:
                    # Request command from user
                    cmd=input("Enter command: Type 'help' for a list of commands\n")
                    # Help display if user requests it
                    if cmd == "help":
                        print("Current list of commands - Page 1 of 2\n")
                        print("NOTE: Any command with NUM requires a numerical value in place of NUM\n\n")
                        print("NUM Forward - Moves the turtle forward at a NUM speed of 0 - 100\n")
                        print("NUM Reverse - Moves the turtle backwards at a NUM speed of 0 - 100\n")
                        print("NUM Left - Turn the turtle left a given NUM angle of degrees from 0 - 360\n")
                        print("NUM Right - Turn the turtle right a given NUM angle of degrees from 0 - 360\n")
                        print("NUM Panel - Turn the solar panel to an angle between 150 - 225\n")
                        print("Stop - Stops the turtle\n")
                        print("Data - Displays the informational data of the turtle (RPY, Panel Angle, GPS coordinates, etc.)\n")
                        print("NUM Error - Changes the IMU error margin to the given NUM... BE CAREFUL, CHANGING THIS NUMBER CAN CAUSE MAJOR ISSUES\n")
                        print("help 2 - to display the second page of commands")
                    # Help page 2 (Need a way to iterate back to help)
                    elif cmd == "help 2":
                        print("Current list of commands - Page 2 of 2\n")
                        print("NOTE: Any command with NUM requires a numerical value in place of NUM\n\n")
                        print("There are no commands here yet...\n")
                    else:
                        # Send the command to the Arduino
                        arduino.write(cmd.encode())
                        
                        # Waits till Arduino recieves the message
                        while arduino.inWaiting()==0: pass
                        if arduino.inWaiting()>0:
                            answer=str(arduino.readline())

                            # If data is called, split the string into our data list and output
                            if cmd == "data":
                                dataList = answer.split("~")
                                tempString = "{}".format(dataList[0])
                                print("Roll: {}º".format(dataList[1]))
                                print("Pitch: {}º".format(dataList[2]))
                                print("Yaw: {}º".format(dataList[3]))
                                print("Long: {}º".format(dataList[4]))
                                print("Lat: {}º".format(dataList[5]))
                                print("Speed: {}º".format(dataList[6]))
                                print("Current Solar Panel Angle: {}º".format(dataList[7]))
                                print("System Voltage: {}V".format(dataList[8]))
                                print("Panel Voltage: {}V".format(dataList[9]))
                                #Add the System Current/Power and Panel Voltage/Current/Power here
                            else:
                                print(answer)

                            arduino.flushInput()
                            
            except KeyboardInterrupt:
                print("CTRL-C pressed, exiting...")