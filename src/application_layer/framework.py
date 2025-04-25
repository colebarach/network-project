from flask import Flask, render_template, request
import subprocess, sys

app = Flask(__name__)

@app.route('/')
def home():
    # Check if there's piped input (data from stdin)
    piped_data = None
    if not sys.stdin.isatty():
        piped_data = sys.stdin.read()  # Capture piped input
        # You could process or store the data here if needed

    return render_template('home.html', piped_data=piped_data)


@app.route('/main/')
def about():
    return render_template('main.html')

@app.route('/send/', methods=['GET', 'POST'])
def send():
    if request.method == 'POST':
        message = request.form['message']
        
        # Make a system call to echo the message in the terminal
        if message:
            # Use subprocess to run the echo command
            subprocess.run(['echo', message])  # This will print the message in the terminal
        
    return render_template('send.html')


@app.route('/receive/')
def receive():
    terminal_data = "No input provided"  # Default message
    if not sys.stdin.isatty():
        # Read data from stdin if it was piped
        terminal_data = sys.stdin.read().strip()
        
    return render_template('receive.html', terminal_data=terminal_data)

if __name__ == '__main__':
    app.run(debug=True)