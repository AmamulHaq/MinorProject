from flask import Flask, request
import mysql.connector
from mysql.connector import Error

app = Flask(__name__)

# Database configuration
db_config = {
    'host': 'localhost',
    'user': 'root',
    'password': '',          # change if you have a password
    'database': 'bmos'
}

# Table name: batteryvoltages
# Columns: TimeStamp (auto), B1, B2, B3, B4, TotalVoltage, Temp

def insert_battery_data(b1, b2, b3, b4, total, temp):
    """Insert one row into batteryvoltages table."""
    try:
        conn = mysql.connector.connect(**db_config)
        cursor = conn.cursor()
        sql = """
            INSERT INTO batteryvoltages (B1, B2, B3, B4, TotalVoltage, Temp)
            VALUES (%s, %s, %s, %s, %s, %s)
        """
        cursor.execute(sql, (b1, b2, b3, b4, total, temp))
        conn.commit()
        cursor.close()
        conn.close()
        return True
    except Error as e:
        print(f"Database error: {e}")
        return False

@app.route('/insert', methods=['GET'])
def insert():
    """HTTP GET endpoint: /insert?B1=...&B2=...&B3=...&B4=...&TotalVoltage=...&Temp=..."""
    try:
        b1 = float(request.args.get('B1', 0))
        b2 = float(request.args.get('B2', 0))
        b3 = float(request.args.get('B3', 0))
        b4 = float(request.args.get('B4', 0))
        total = float(request.args.get('TotalVoltage', 0))
        temp = float(request.args.get('Temp', 0))
    except (TypeError, ValueError):
        return "Invalid parameters", 400

    if insert_battery_data(b1, b2, b3, b4, total, temp):
        print(f"Inserted: B1={b1}, B2={b2}, B3={b3}, B4={b4}, Total={total}, Temp={temp}")
        return "OK", 200
    else:
        return "Database error", 500

if __name__ == '__main__':
    # Run the Flask server on all network interfaces, port 5000
    app.run(host='0.0.0.0', port=5000, debug=True)