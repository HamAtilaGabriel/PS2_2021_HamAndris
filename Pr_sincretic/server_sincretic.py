from flask import Flask
from flask import request

import serial
import smtplib, ssl
import datetime

app = Flask(__name__)
check = 0

ser = serial.Serial('COM3')
print(ser.name) 

def send_leak_mail():
    if check == 0:
        now = datetime.datetime.now()
        message = """S-a detectat o inundatie!""" + """\nData: """ + now.strftime("%d-%m-%y ora %H:%M:%S")
        context = ssl.create_default_context()
        with smtplib.SMTP("smtp.gmail.com", 587) as server:
            server.starttls(context=context)
            server.login("andris.laurentiu@gmail.com", 'BYYu$?nR:T;Ukt4') # Enable IMAP in contul google, iar apoi Enable less secure apps la https://myaccount.google.com/lesssecureapps
            server.sendmail('andris.laurentiu@gmail.com', 'kinect3600@gmail.com', message)

@app.route('/')
def hello_world():
    text = 'Proiect Sincretic 2021'
    temp = '- Temperatura este '
    temp_serial = ser.readline()
    temp_serial = temp_serial.decode()
	
    if temp_serial.find("!INUNDATIE!") == 0:
        check = 1
        send_leak_mail()
        return temp_serial
        
    string_butoane = '<p>LED1 State:<button onclick="document.location=\'led_off\'">LED OFF</button> <button onclick="document.location=\'led_on\'">LED ON</button></p>'
    color_picker = '<p>LED2 RGB Selector: <form method=\"get\" action=\"color\"><input name=\"colpicker\" type=\"color\"/> <input type=\"submit\" value=\"send\"></form></p>'
    text_form = '<p>Afiseaza text pe display: <form method=\"get\" action=\"mesaj\"><input name=\"msg\" type=\"text\"/> <input type=\"submit\" value=\"send\"></form></p>'

    return text + temp + temp_serial + string_butoane + color_picker + text_form
    
@app.route('/led_on')
def led_on():
    ser.write("`".encode())
    return "Am aprins ledul"

@app.route('/led_off')
def led_off():
    ser.write("~".encode())
    return "Am stins ledul"
    
@app.route('/color')
def color_picker():
    color=str(request.args['colpicker'])
    red = int("0x" + color[1:3], 16) * 99/255.0
    green = int("0x" + color[3:5], 16) * 99/255.0
    blue = int("0x" + color[5:7], 16) * 99/255.0
    
    color="$" + str(int(red)).zfill(2) + str(int(green)).zfill(2) + str(int(blue)).zfill(2) + "&"
    print(color)
    ser.write(color.encode())
    return "Am modificat culoarea RGB"    
    
@app.route('/mesaj')
def message_parser():
    mesaj = str(request.args['msg'])
    mesaj_serial = "#" + mesaj + "^"
    ser.write(mesaj_serial.encode())
    return "Am transmis mesajul " + mesaj
    
    
    