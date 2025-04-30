from flask import Flask, render_template, request, session
import subprocess, sys, secrets

app = Flask(__name__)

app.secret_key = secrets.token_hex(16)  # Set a secret key for sessions

@app.route('/')
def home():
    # Check if there's piped input (data from stdin)
    piped_data = session.get('piped_data', None)  # Get data from session

    if not piped_data and not sys.stdin.isatty():
        piped_data = sys.stdin.read()  # Capture piped input
        session['piped_data'] = piped_data  # Save it in the session

    return render_template('home.html', piped_data=piped_data)

@app.route('/main/')
def about():
    return render_template('main.html')

@app.route('/send/', methods=['GET', 'POST'])
def send():
    if request.method == 'POST':
        source_address = request.form.get('source_address', '')
        destination_address = request.form.get('address', '')
        message = request.form.get('message', '')

        if message:
            # Append newline to the message
            message += '\n'

            print(f"Sending from {source_address} to {destination_address}: {message}")

            # Call tx executable with updated message
            subprocess.run([
                '../application_tool/build/tx',
                source_address,
                destination_address,
                message
            ])

    return render_template('send.html')


@app.route('/receive/')
def receive():
    terminal_data = "No input received."  # Default message

    try:
        # Run the 'rx' executable and capture its stdout
        result = subprocess.run(
            ['../application_tool/build/rx'],                 # Make sure this path is correct and executable
            stdout=subprocess.PIPE,   # Capture stdout
            stderr=subprocess.PIPE,   # Optional:  capture stderr for debugging
            text=True,                # Decode stdout as text (instead of bytes)
            check=True                # Raise CalledProcessError on failure
        )
        terminal_data = result.stdout.strip() or "Executable ran but gave no output."

    except subprocess.CalledProcessError as e:
        terminal_data = f"Executable failed:\n{e.stderr.strip()}"
    except FileNotFoundError:
        terminal_data = "Error: 'rx' executable not found."
    except Exception as e:
        terminal_data = f"Unexpected error: {str(e)}"

    return render_template('receive.html', terminal_data=terminal_data)

if __name__ == '__main__':
    app.run(debug=True)