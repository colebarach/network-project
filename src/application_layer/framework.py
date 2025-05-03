from flask import Flask, render_template, request, session, Response, stream_with_context, jsonify
import subprocess, sys, secrets, threading, os, time 

app = Flask(__name__)

app.secret_key = secrets.token_hex(16)  # Set a secret key for sessions

tx_process = None
tx_input = ""
rx_process = None
rx_output = []
rx_lock = threading.Lock()

@app.route('/')
def about():
    return render_template('main.html')

# @app.route('/send/', methods=['GET', 'POST'])
# def send():
#     tx_output = ""
#     tx_error = ""
#     message_sent = ""
#     source_address = ""
#     destination_address = ""
#     serial_port = ""
#     message = ""

#     if request.method == 'POST':
#         source_address = request.form.get('source_address', '')
#         destination_address = request.form.get('destination_address', '')
#         serial_port = request.form.get('serial_port', '')
#         message = request.form.get('message', '') + '\n'
#         message_sent = message.strip()

#         try:
#             if message:
#                 result = subprocess.run(
#                     ['../transport_layer/build/tx', source_address, destination_address, serial_port], # Test Code
#                     #['../application_tool/build/tx', source_address, destination_address, serial_port],
#                     input=message.encode('utf-8'),
#                     stdout=subprocess.PIPE,
#                     stderr=subprocess.PIPE
#                 )

#                 # Capture outputs
#                 tx_output = result.stdout.decode('utf-8').strip()
#                 tx_error = result.stderr.decode('utf-8').strip()
#                 print("tx_error:",tx_error)

#         except Exception as e:
#             tx_error = f"Error during subprocess call: {str(e)}"

#     return render_template(
#     'send.html',
#     tx_output=tx_output,
#     tx_error=tx_error,
#     message_sent=message_sent,
#     source_address=source_address,
#     destination_address=destination_address,
#     serial_port=serial_port,
#     message=message.strip()
# )

@app.route('/send/', methods=['GET', 'POST'])
def send():
    global tx_process, tx_input
    tx_output = ""
    tx_error = ""
    message_sent = ""
    source_address = ""
    destination_address = ""
    serial_port = ""
    message = ""
    action = request.form.get('action')

    if request.method == 'POST':
        source_address = request.form.get('source_address', '')
        destination_address = request.form.get('destination_address', '')
        serial_port = request.form.get('serial_port', '')

    if action == 'stop':
        if tx_process is not None:
            print ("STOPPING")
            tx_process.terminate()
            tx_process = None
        else:
            error = "No transmitter is running."
    elif not source_address or not destination_address:
        error = "Source and destination are required."
    elif action == 'start':
        if tx_process is None:
            try:
                tx_process = subprocess.Popen(
                    ['../transport_layer/build/tx', source_address, destination_address, serial_port], # Test Code
                    # ['../application_tool/build/rx', source, destination, serial_port],
                    stdin=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True
                )
            except Exception as e:
                tx_process = None
        else:
            error = "Transmitter already running."

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

@app.route('/send/output/', methods=['GET', 'POST'])
def sendoutput():
    global tx_process, tx_input
    tx_output = ""
    message_sent = ""
    source_address = ""
    destination_address = ""
    serial_port = ""
    action = request.form.get('action')
    message = request.form.get('message')

    if request.method == 'POST':
        source_address = request.form.get('source_address', '')
        destination_address = request.form.get('destination_address', '')
        serial_port = request.form.get('serial_port', '')

    error = ""
    if tx_process is None:
        error = "No transmitter is running."
    else:
        tx_process.stdin.write (message + '\n')
        tx_process.stdin.flush ()

        try:
            tx_process.wait (timeout=1)
            error = "Transmitter timed out."
            tx_process = None
        except subprocess.TimeoutExpired:
            pass

    return render_template(
    'send.html',
    tx_error=error,
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
            ['../transport_layer/build/rx', source, destination, serial_port], # Test Code
            # ['../application_tool/build/rx', source, destination, serial_port],
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
    port = int(sys.argv [1])
    app.run(debug=True, port=port)