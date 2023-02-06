import serial,time
from flask import Flask, render_template, request, flash, redirect, url_for, make_response, jsonify

app = Flask(__name__)

global arduino
arduino = serial.Serial("/dev/ttyACM0", 9600, timeout=1)

@app.route('/')
def home():
    return render_template("helloworld.html", result = "initialized", cmd = "")

@app.route('/getcommand', methods = ["GET", "POST"])
def getcommand():
    if request.method == "GET":
        return redirect("/")
    if request.method == "POST":
        command = request.form.get("command")
        if command == "blink":
            arduino.write(command.encode())
            print(str(arduino.readline()))
            arduino.flushInput()
            return render_template("helloworld.html", result = "initialized" , cmd = "blinking")
        elif command == "stop":
            arduino.write(command.encode())
            print(str(arduino.readline()))
            arduino.flushInput()
            return render_template("helloworld.html", result = "initialized" ,cmd = "stop")
        else: 
            return render_template("helloworld.html", result = "initialized" ,cmd = "invalid")
    else:
        return redirect("/")
        
if __name__ == "__main__":
    app.run(host='0.0.0.0', port = '6969', debug = 'true')