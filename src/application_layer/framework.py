from flask import Flask, render_template

app = Flask(__name__)

@app.route('/')
def home():
    title = 'Welcome to Our Website!'
    return render_template('home.html', title=title)

@app.route('/main/')
def about():
    return render_template('main.html')

if __name__ == '__main__':
    app.run(debug=True)