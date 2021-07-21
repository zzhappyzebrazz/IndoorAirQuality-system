import sqlite3
import sys
import time
import paho.mqtt.client as mqtt
import json

topic = ""
device1_topic = "myHiveMQ/device1/topic"
device2_topic = "myHiveMQ/device2/topic"
device3_topic = "myHiveMQ/device3/topic"

table1_cmd = "INSERT INTO DEVICE1_data VALUES(datetime('now','localtime'), (?), (?), (?))"
table2_cmd = "INSERT INTO DEVICE2_data VALUES(datetime('now','localtime'), (?), (?), (?))"
table3_cmd = "INSERT INTO DEVICE3_data VALUES(datetime('now','localtime'), (?), (?), (?))"
data_in = ""
msg_in = {
        "Dust": 0,
        "eCO2": 0,
        "eTVOC": 0,
    }
Dust = msg_in["Dust"]
eCO2 = msg_in["eCO2"]
eTVOC = msg_in["eTVOC"]
dbname = 'sDatabase.db'
sampleFreq = 5 #time in second

#intial mqtt client
client = mqtt.Client()

# set username and password
client.username_pw_set("hppzbr", "Lailano01")

# connect to HiveMQ Cloud on port 1883
client.connect("broker.mqttdashboard.com", 1883)

# subscribe to the topic "my/test/topic"
client.subscribe("myHiveMQ/device1/topic")
client.subscribe("myHiveMQ/device2/topic")
client.subscribe("myHiveMQ/device3/topic")

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected successfully")
    else:
        print("Connect returned result code: " + str(rc))

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    #print("Received message: " + msg.topic + " -> " + msg.payload.decode("utf-8"))
    topic = msg.topic
    data_in = str(msg.payload.decode("utf-8","ignore"))
    msg_in = json.loads(data_in)
    if topic == device1_topic:
        add_data(table1_cmd, msg_in["Dust"], msg_in["eCO2"], msg_in["eTVOC"])
    if topic == device2_topic:
        add_data(table2_cmd, msg_in["Dust"], msg_in["eCO2"], msg_in["eTVOC"])
    if topic == device3_topic:
        add_data(table3_cmd, msg_in["Dust"], msg_in["eCO2"], msg_in["eTVOC"])


# enable TLS
# client.tls_set(tls_version=mqtt.ssl.PROTOCOL_TLS)
#call back function
client.on_connect = on_connect
client.on_message = on_message
#function to insert data on table
def add_data(data_table, Dust, eCO2, eTVOC):
    conn = sqlite3.connect('sDatabase.db')
    curs = conn.cursor()
    command = ""
    curs.execute(data_table, (Dust, eCO2, eTVOC))
    conn.commit()
    conn.close()
    
#print out all the data in database
def displayDatabase():
    conn = sqlite3.connect('sDatabase.db')
    curs = conn.cursor()    
    print("\n Entire database contents: \n")
    for row in curs.execute("SELECT * FROM DEVICE1_data"):
        print(row)
    conn.close()
    
#retrieve the last data stored in database
def get_Data():
    conn = sqlite3.connect('sDatabase.db')
    curs = conn.cursor()
    for row in curs.execute("SELECT * FROM DEVICE1_data ORDER BY timestamp DESC LIMIT 1"):
        time = str(row[0])
        Dust = row[1]
        eCO2 = row[2]
        eTVOC = row[3]
    conn.close()
    return time, Dust, eCO2, eTVOC

#main function
def main():
    client.loop_start()
    while(True):
        conn=sqlite3.connect('sDatabase.db')
        curs=conn.cursor()
        print ("Last raw Data logged on database:")
        for row in curs.execute("SELECT * FROM DEVICE1_data ORDER BY timestamp DESC LIMIT 1"):
             print ("Device1 "+str(row[0])+" ==> Dust = "+str(row[1])+"	eCO2 ="+str(row[2]) + " eTVOC = " +str(row[3]))
        for row in curs.execute("SELECT * FROM DEVICE2_data ORDER BY timestamp DESC LIMIT 1"):
             print ("Device2 "+str(row[0])+" ==> Dust = "+str(row[1])+"	eCO2 ="+str(row[2]) + " eTVOC = " +str(row[3]))
        for row in curs.execute("SELECT * FROM DEVICE3_data ORDER BY timestamp DESC LIMIT 1"):
             print ("Device3 "+str(row[0])+" ==> Dust = "+str(row[1])+"	eCO2 ="+str(row[2]) + " eTVOC = " +str(row[3]))
            
        time.sleep(5)
        
    client.loop_stop()
#     conn=sqlite3.connect('sDatabase.db')
#     curs=conn.cursor()
#     print ("\nLast raw Data logged on database:\n")
#     for row in curs.execute("SELECT * FROM DEVICE1_data ORDER BY timestamp DESC LIMIT 1"):
#         print (str(row[0])+" ==> Dust = "+str(row[1])+"	eCO2 ="+str(row[2]) + " eTVOC = " +str(row[3]))

#main route
#Execute program
main()
        
