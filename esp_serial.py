import esp_serial

port = esp_serial.Serial('COM4',9600)

while( port.isOpen()):
    data = int(input("Enter 1 to ""ON"" the led and 0 to ""OFF"" the led: "))

    if(data == 1):
        port.write(str.encode('1'))
    elif(data == 0):
        port.write(str.encode('0'))
    else:
        print('Invalid input!!!!')  