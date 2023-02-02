from flask import Flask, render_template, jsonify

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('helloworld.html', value = "Hello Sunny")

if __name__ == "__main__":
    app.run(host='0.0.0.0', port = '8080', debug = 'true')