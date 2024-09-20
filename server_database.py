import socket # socket library for creating a server
import threading # Used to connect to multiple clients
import sqlite3 # SQLite library

header=1024 #defining the maximum size of the message that can be recieved
ip='192.168.1.15' # IP address of your device which will host the server
port=5000 # Port used to establish the connection
Format='utf-8' #data encoding format <netstat -an | find "your_port_number"> use this command to check if your port is in use 
addr=(ip,port) # create a tupple containing the IP address of the host and the port number
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # creating the server object, we are using TCP protocol
server.bind(addr) # Binding the server to the specified Port number

def handle_client(conn,add):
    """Function name - handle_client
       Arguemnets - conn,addr
       Returns - None
       Function - It handles the clients that are connected to the server"""
    print(f'New device connected:{add}')
    con_db=sqlite3.connect('sensor.db') # Establishing a connection to the database
    cur=con_db.cursor() # creating a cursor 
    cur.execute("CREATE TABLE Device1 (sensor text,sensor_value int,time int)") # creating a table with three coloumns, If a table already exists, add a try except block to handle exceptions and errors
    con_db.commit() # save the changes
    connected=True
    while connected:
        msg=conn.recv(header).decode(Format) # Wait to recieve a message from the client, header specifies the maximum size of the message that can be sent
        print(msg)
        msg=msg.split(',') # splitting the recieved message at commas
        cur.execute("INSERT INTO Device1 VALUES (:sensor,:sensor_value,:time)",{'sensor':msg[0],'sensor_value':int(msg[1]),'time':int(msg[2])}) # Adding the recieved data to the created table
        con_db.commit() #saving the changes
        cur.execute("SELECT * FROM Device1 WHERE sensor=:sensor",{'sensor':'IR_sensor'}) # Making a query to fetch the row of data with IR_sensor
        print(cur.fetchall()) # Fetchin all rows that has the namw IR_sensor
        con_db.close() # closing the connection to the data base
        connected=False 
    conn.close() # closing the connection to the client 
        
def start():
    server.listen() # Setting the socket to listen mode
    while True:
        conn,add=server.accept() # connecting to the client, returns a connection id and address
        print("Connection succesfull")
        thread=threading.Thread(target=handle_client, args=(conn,add)) # starting a new thread to handle the client 
        thread.start() #starting the thread
        print(f"Active Thread Count = {threading.active_count()-1}")
print("[Server is Listening]")
start() 
print("Data has been logged")
