import serial
import csv

# Configure the serial port
ser = serial.Serial('COM2', 115200)  
ser.flushInput()

# Create a CSV file and write headers
csv_filename = 'test_data/high(35,0.01,1400)_clamp(225).csv'
with open(csv_filename, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(['Time', 'Temperature', 'Setpoint', 'PWM'])

    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            print(line)  # Optionally, print the received line to console

            # Split the CSV line
            data = line.split(',') 


            # Write data to CSV files
            writer.writerow(data)


