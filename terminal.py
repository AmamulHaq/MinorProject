import serial
import mysql.connector
from time import sleep

# ----- SERIAL SETUP -----
# Change 'COM4' to your port and 9600 to your baudrate
ser = serial.Serial('COM4', 9600, timeout=1)
sleep(2)  # wait for serial connection to initialize

# ----- DATABASE SETUP -----
db = mysql.connector.connect(
    host="localhost",
    user="root",         # your MySQL username
    password="",         # your MySQL password
    database="bms"
)

cursor = db.cursor()

# ----- LOOP TO READ SERIAL AND INSERT INTO DB -----
try:
    while True:
        if ser.in_waiting > 0:
            # Read line from serial, decode and strip whitespace
            value = ser.readline().decode(errors='ignore').strip()

            if value:  # make sure it's not empty
                print(f"Voltage read: {value}")

                # Prepare SQL query
                sql = "INSERT INTO voltage_table (voltage) VALUES (%s)"  # replace 'voltage_table' with your table name
                cursor.execute(sql, (value,))
                db.commit()
                print("Inserted into database!")

except KeyboardInterrupt:
    print("Exiting program.")

finally:
    ser.close()
    cursor.close()
    db.close()