from flask import Flask, render_template, request, session, Response, stream_with_context, jsonify
import subprocess, sys, secrets, threading, os, time 

app = Flask(__name__)

app.secret_key = secrets.token_hex(16)  # Set a secret key for sessions

rx_process = None
rx_output = []
rx_lock = threading.Lock()

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
    tx_output = ""
    tx_error = ""
    message_sent = ""
    source_address = ""
    destination_address = ""
    serial_port = ""
    message = ""

    if request.method == 'POST':
        source_address = request.form.get('source_address', '')
        destination_address = request.form.get('destination_address', '')
        serial_port = request.form.get('serial_port', '')
        message = request.form.get('message', '') + '\n'
        message_sent = message.strip()

        try:
            if message:
                result = subprocess.run(
                    ['../network_utils/build/tx', source_address, destination_address, serial_port], # Test Code
                    #['../application_tool/build/tx', source_address, destination_address, serial_port],
                    input=message.encode('utf-8'),
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE
                )

                # Capture outputs
                tx_output = result.stdout.decode('utf-8').strip()
                tx_error = result.stderr.decode('utf-8').strip()
                print("tx_error:",tx_error)

        except Exception as e:
            tx_error = f"Error during subprocess call: {str(e)}"

    return render_template(
    'send.html',
    tx_output=tx_output,
    tx_error=tx_error,
    message_sent=message_sent,
    source_address=source_address,
    destination_address=destination_address,
    serial_port=serial_port,
    message=message.strip()
)


def run_rx(source, destination, serial_port):
    global rx_process, rx_output
    try:
        rx_output.clear()
        rx_process = subprocess.Popen(
            ['../network_utils/build/rx', source, destination, serial_port], # Test Code
            ['../application_tool/build/rx', source, destination, serial_port],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        for line in rx_process.stdout:
            with rx_lock:
                rx_output.append(line.strip())

    except Exception as e:
        with rx_lock:
            rx_output.append(f"Error: {str(e)}")
    finally:
        rx_process = None


@app.route('/receive/', methods=['GET', 'POST'])
def receive():
    global rx_process

    source = destination = ""
    error = None

    source = ""
    destination = ""
    serial_port = ""

    if request.method == 'POST':
        source = request.form.get('source', '')
        destination = request.form.get('destination', '')
        serial_port = request.form.get('serial_port', '')
        action = request.form.get('action')

        if not source or not destination:
            error = "Source and destination are required."
        elif action == 'start':
            if rx_process is None:
                thread = threading.Thread(target=run_rx, args=(source, destination, serial_port), daemon=True)
                thread.start()
            else:
                error = "Receiver already running."
        elif action == 'stop':
            if rx_process:
                rx_process.terminate()
                rx_process = None
            else:
                error = "No receiver is running."

    return render_template(
    'receive.html',
    source=source,
    destination=destination,
    serial_port=serial_port,
    error=error
)


@app.route('/receive/output')
def receive_output():
    with rx_lock:
        return jsonify({'output': '\n'.join(rx_output)})

if __name__ == '__main__':
    app.run(debug=True)