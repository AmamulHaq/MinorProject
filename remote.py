from flask import Flask, request
import mysql.connector

app = Flask(__name__)

# MySQL connection
db = mysql.connector.connect(
    host="localhost",
    user="root",         # your MySQL username
    password="",         # your MySQL password
    database="bms"
)
cursor = db.cursor()

@app.route("/insert_voltage", methods=['GET'])
def insert_voltage():
    voltage = request.args.get('voltage')
    if voltage:
        try:
            sql = "INSERT INTO voltage_table (voltage) VALUES (%s)"
            cursor.execute(sql, (voltage,))
            db.commit()
            return f"Inserted voltage: {voltage}", 200
        except Exception as e:
            return f"Database error: {e}", 500
    return "No voltage received", 400

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)