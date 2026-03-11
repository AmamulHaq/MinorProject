from flask import Flask, request
import mysql.connector

app = Flask(__name__)

# Connect to MySQL
db = mysql.connector.connect(
    host="localhost",
    user="root",
    password="",  # your MySQL password
    database="bms"
)
cursor = db.cursor()

@app.route("/insert_voltage", methods=["GET"])
def insert_voltage():
    voltage = request.args.get("voltage")

    if voltage:
        try:
            sql = "INSERT INTO voltage_table (voltage) VALUES (%s)"
            cursor.execute(sql, (voltage,))
            db.commit()
            print("Inserted:", voltage)
            return "OK", 200
        except Exception as e:
            print("DB Error:", e)
            return "DB Error", 500

    return "No data", 400

if __name__ == "__main__":
    # host=0.0.0.0 allows ESP32 to reach the Flask server over hotspot
    app.run(host="0.0.0.0", port=5000)