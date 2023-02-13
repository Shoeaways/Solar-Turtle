"""
Main file for the communication between our Raspberry Pi to Arduino and Webserver
Rev 1.9 Changes
- 

TODO:
(Current Rev)
- Create a terminal style text box to display outputs on html
    - When a function is calledm trigger an event handler to update the terminal without refreshing the page
- Display data in the html file
    - When data is called, trigger an event handler which then uses AJAX to refresh the data to update
- Fill out the text outputs for each function
(Future Rev)
- Define multiple app routes to allow user to go through webpages 
- Make the templates for the errors


"""
import serial,time
from flask import Flask, render_template, request, redirect, url_for, flash, make_response, jsonify
app = Flask(__name__)

# Set global variable "arduino" as the serial connection to the arduino
global arduino
arduino = serial.Serial("/dev/ttyACM0", 9600, timeout=1)

# App Error handlers
"""
@app.errorhandler(400)
def bad_request():
    return make_response(render_template("400.html"))

@app.errorhandler(404)
def not_found():
    return make_response(render_template("404.html"))

@app.errorhandler(500)
def server_error():
    return make_response(render_template("500.html"))

@app.errorhandler(503)
def service_unav():
    return make_response(render_template("503.html"))
"""

# Defining home page of app 
@app.route('/')
def home():
    return render_template("Solar Turtle.html", result = "initialized", cmd = "", num = "0")

# Defining the page that awaits a user input
@app.route('/getcommand', methods = ['GET', 'POST'])
def getcommand():
    if request.method == 'POST':
        cmdinput = request.form.get("command")

        cmdtemp = cmdinput.split()
        # If the array is just the command, continue
        if len(cmdtemp) == 1:
            tempcmd = cmdtemp[0]
            num = 0
        # If the array is two items, check if the first item is a valid string value. Otherwise convert it to an integer
        elif len(cmdtemp) == 2:
            tempcmd = cmdtemp[0].lower()
            if tempcmd == "quick" or tempcmd == "slow":
                num = tempcmd
            else:
                num = int(cmdtemp[0])
            tempcmd = cmdtemp[1]
        # If none of the above are true, return incorrect command message
        else:
            return render_template("Solar Turtle.html", result = "initialized", cmd = "invalid", num = "0")
        
        cmd = tempcmd.lower()

        if cmd == "help":
            return render_template("Solar Turtle.html", result = "initialized", cmd = "help", num = str(num))

        # If panel command is called, update the panel text
        elif cmd == "panel":
            # Ensure that num is within bounds
            if num > 125:
                #print("Provided angle " + str(num) + " is out of upper bounds, setting the panel to 125º")
                num = 125
            elif num < 80:
                #print("Provided angle " + str(num) + " is out of lower bounds, setting the panel to 80º")
                num = 80
            # Use tempcmd for the next line to be allowed
            tempcmd = cmd
            # Recombine the num and cmd message
            cmd = (str(num) + " " + tempcmd)                       

        # If forward or reverse command is called
        elif cmd == "forward" or cmd == "reverse":
            # Ensure that num is within bounds
            if num > 100:
                #print("Provided speed " + str(num) + " is out of upper bounds, setting the speed to 100%")
                num = 100
            elif num < 0:
                #print("Provided speed " + str(num) + " is out of lower bounds, setting the speed to 0%")
                num = 0
            # Use tempcmd for the next line to be allowed
            tempcmd = cmd
            # Recombine the num and cmd message
            cmd = (str(num) + " " + tempcmd)

        # If left or right turn command is called
        elif cmd == "left" or cmd == "right":
            # Ensure that num is within bounds
            if num > 360:
                #print("Provided angle " + str(num) + " is out of upper bounds, setting the turn to 360º")
                num = 360
            elif num < 0:
                #print("Provided angle " + str(num) + " is out of lower bounds, setting the turn to 0º")
                num = 0
            # Use tempcmd for the next line to be allowed
            tempcmd = cmd
            # Recombine the num and cmd message
            cmd = (str(num) + " " + tempcmd)

        # If the error command is called
        elif cmd == "error":
            # Use tempcmd for the next line to be allowed
            tempcmd = cmd
            # Recombine the num and cmd message
            cmd = (str(num) + " " + tempcmd)

        # If stop command is called
        elif cmd == "stop":
            # Check if this is a quick or slow stop
            if num == "quick" or num == "slow":
                tempcmd = cmd
                cmd = (num + " " + tempcmd)
        
        # If data command is called
        elif cmd == "data":
            tempcmd = "data"

        # If any other command is called, it is invalid and runs back around to the top code  
        else:
            return render_template("Solar Turtle.html", result = "initialized", cmd = "invalid", num = "0")

        # Send arduino the command
        arduino.write(cmd.encode())     
        arduino.flushInput()
        # If data is called, split the string into our data list and output
        if cmd == "data":
            answer = str(arduino.readline())
            dataList = answer.split("~")
            return render_template("Solar Turtle.html", result = "initialized", cmd = "data", num = str(num), dataList = jsonify(dataList))                            
        else:
            return render_template("Solar Turtle.html", result = "initialized", cmd = tempcmd, num = str(num))                    
    else:
        return redirect(url_for(home))

if __name__ == '__main__':
    app.run(host='0.0.0.0', port = '6969', debug = 'true')