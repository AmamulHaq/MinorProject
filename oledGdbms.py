from flask import Flask, request
import mysql.connector

app = Flask(__name__)

db = mysql.connector.connect(
    host="localhost",
    user="root",
    password="",
    database="bms"
)

cursor = db.cursor()

@app.route("/insert_voltage", methods=["GET"])
def insert_voltage():

    voltage = request.args.get("voltage")

    if voltage:
        sql = "INSERT INTO voltage_table (voltage) VALUES (%s)"
        cursor.execute(sql, (voltage,))
        db.commit()

        print("Inserted:", voltage)

        return "OK", 200

    return "No data", 400


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)