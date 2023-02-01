import serial,time

if __name__ == '__main__':
    print('Running. Press CTRL-C to exit.')
    with serial.Serial("/dev/ttyACM0", 115200, timeout=1) as arduino:
        time.sleep(0.1)
        if arduino.isOpen():
            print("Solar Turtle connected!".format(arduino.port))
            try:
                while True:
                    cmd=input("Enter command : ")
                    arduino.write(cmd.encode())
                    
                    while arduino.inWaiting()==0: pass
                    if arduino.inWaiting()>0:
                        print(arduino.readline())
                        #answer=str(arduino.readline())
                        #print("---> {}".format(answer))
                        arduino.flushInput()
                            
            except KeyboardInterrupt:
                print("CTRL-C pressed, exiting...")