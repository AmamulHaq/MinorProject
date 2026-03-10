import serial
import time

ser = serial.Serial('COM4', 9600)
time.sleep(2)

while True:
    if ser.in_waiting > 0:
        value = ser.readline().decode().strip()
        print("Analog Value:", value)

