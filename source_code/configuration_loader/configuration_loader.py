import serial
import json
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-P', '--port', type=str, help='The serial port used for the comunication (example : com6)', required=True)
parser.add_argument('-p', '--PIN', type=str, help='The pin of the last configuration loaded on the board', default='0000')
parser.add_argument('-c', '--configuration_path', type=str, help='The path of the configuration file to load on the board',
                    default='configuration.json')
parser.add_argument('-l', '--listen', action='store_true', help='After the configuration loading, continue listening on the specified serial port in order to receive log messages')
args = parser.parse_args()

with open(args.configuration_path, 'r') as file:
    configuration = json.load(file)

STM32_serial = serial.Serial(args.port, 9600, timeout=2)
STM32_serial.write(configuration['PIN'].encode('utf-8'))
STM32_serial.write(
    bytes([configuration['area_sensor_delay'], configuration['barrier_sensor_delay'],
           configuration['alarm_duration']]))
STM32_serial.write(bytes([
    configuration['date_time']['seconds'],
    configuration['date_time']['minutes'],
    configuration['date_time']['hours'],
    configuration['date_time']['day_month'],
    configuration['date_time']['month'],
    configuration['date_time']['year']
]))

STM32_serial.write(args.PIN.encode('utf-8'))

if args.listen:
    while True:
        print(STM32_serial.readline().decode('utf-8'), end='', flush=True)

