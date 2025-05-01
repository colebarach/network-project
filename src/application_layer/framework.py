from flask import Flask, render_template, request, session, Response, stream_with_context, jsonify
import subprocess, sys, secrets, threading, os, time 

app = Flask(__name__)

app.secret_key = secrets.token_hex(16)  # Set a secret key for sessions

rx_process = None
rx_output = []

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

    if request.method == 'POST':
        source_address = request.form['source_address']
        destination_address = request.form['address']
        serial_port = '/dev/ttyACM1'
        message = request.form['message'] + '\n'  # Append newline as required
        message_sent = message.strip()

        try:
            if message:
                result = subprocess.run(
                    ['../network_utils/build/tx', source_address, destination_address, serial_port],
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
        message_sent=message_sent
    )



@app.route('/receive/', methods=['GET', 'POST'])
def receive():
    return render_template('receive.html')

@app.route('/start_receive', methods=['POST'])
def start_receive():
    global rx_process
    global rx_output

    data = request.get_json()
    source = data.get('source')
    destination = data.get('destination')

    if not source or not destination:
        return jsonify({'success': False, 'error': 'Source and destination are required.'})

    # Start the rx process in the background
    def run_rx():
        global rx_process
        global rx_output

        # Define the rx command
        cmd = ['../application_tool/build/rx', source, destination, '/dev/ttyACM0']
        
        try:
            rx_process = subprocess.Popen(
                cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
            )
            
            # Continuously read from stdout and add data to rx_output
            while True:
                output = rx_process.stdout.readline()
                if output == '' and rx_process.poll() is not None:
                    break
                if output:
                    # Append new output to rx_output
                    rx_output.append(output.strip())
                time.sleep(0.1)  # Sleep to avoid high CPU usage during waiting
        except Exception as e:
            print(f"Error running rx: {e}")
        
        # Close stdout when done
        rx_process.stdout.close()

    # Start rx in a separate thread
    threading.Thread(target=run_rx, daemon=True).start()

    return jsonify({'success': True})

@app.route('/stop_receive', methods=['POST'])
def stop_receive():
    global rx_process
    if rx_process:
        rx_process.terminate()  # Terminate the rx process
        rx_process = None
    return jsonify({'success': True})

@app.route('/sse_output')
def sse_output():
    def generate():
        global rx_output
        while True:
            if rx_output:
                message = rx_output.pop(0)  # Get the first message in the output list
                yield f"data: {message}\n\n"  # SSE format
            time.sleep(0.1)  # Sleep to avoid high CPU usage during waiting

    return Response(generate(), mimetype='text/event-stream')

if __name__ == '__main__':
    app.run(debug=True)