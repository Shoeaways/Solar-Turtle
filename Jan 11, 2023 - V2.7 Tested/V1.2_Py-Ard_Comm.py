import serial,time

if __name__ == '__main__':
    print('Running. Press CTRL-C to exit.')
    with serial.Serial("/dev/ttyACM0", 9600, timeout=1) as arduino:
        time.sleep(0.1)
        if arduino.isOpen():
            print("Solar Turtle connected!".format(arduino.port))
            try:
                while True:
                    cmd=input("Enter command : ")
                    arduino.write(cmd.encode())
                    
                    while arduino.inWaiting()==0: pass
                    if arduino.inWaiting()>0:
                        answer=str(arduino.readline())
                        print(answer)

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

                        arduino.flushInput()
                            
            except KeyboardInterrupt:
                print("CTRL-C pressed, exiting...")