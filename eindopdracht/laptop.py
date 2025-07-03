import requests
import requests_raw
import time
import string

arduinoIP = "http://10.1.1.12"
rpiIP = "http://10.1.1.11"

while True:
    TheData = ""
    complete = True

    req = "GET /sensors/1/actual HTTP/1.0\r\n\r\n"
    responseSensor1 = requests_raw.raw(url=arduinoIP, data=req)
    if responseSensor1.status_code == 200:
        if responseSensor1.content.decode('ascii') != "-1.00\r\n":
            TheData += responseSensor1.content.decode('ascii').replace("\r\n", " ")
            print(responseSensor1.content.decode('ascii'))
        else:
            complete = False

    req = "GET /sensors/2/actual HTTP/1.0\r\n\r\n"
    responseSensor2 = requests_raw.raw(url=arduinoIP, data=req)
    if responseSensor2.status_code == 200:
        if responseSensor2.content.decode('ascii') != "-1.00\r\n":
            TheData += responseSensor2.content.decode('ascii').replace("\r\n", "")
            print(responseSensor2.content.decode('ascii'))
        else:
            complete = False

    if complete:
        req = "POST /data HTTP/1.0\r\nContent-Length: "
        req += str(len(TheData))
        req +="\r\n\r\n"
        req2 = req + TheData
        responseServer = requests_raw.raw(url=rpiIP, data=req2)


    time.sleep(12)