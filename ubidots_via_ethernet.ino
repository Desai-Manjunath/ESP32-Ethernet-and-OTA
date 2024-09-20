#define LISTENPORT 9012 // TCP Port number for ubidots website, this port is used for unsecure communication
#define UARTBAUD 115200 // Setting the baud rate for the Serial Communication
#define DELAY 2000 

#include <UIPEthernet.h> //Library used for interfacing ENC28J60 with ESP32 , Refer the Ethernet library documentation in the official Arduino website


EthernetClient client; // Creating an instance of the client       
const char *host = "industrial.api.ubidots.com"; //This is the end point for the ubidots communication
const char *token = "BBUS-4iHI00OFsTzAZxPcQrS5ITwe2hHwLA"; //Unique Token ID assigned by ubidots to each user, replace this with your token id
const char *variable_name = "data_send"; //This is the name of the variable created in hte ubidots interface that is meant to recieve data from the esp
const char *device_name = "esp32"; //this is the device name  in ubi dots application
const char *device_id="66e7cd827f1f25a561db3169"; // this a unique id assigned to each device by ubidots
uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // This is random hexadecimal value assinged as the MAC address of the ethernet module by the user
unsigned long latency=0,current_time=0; // variables used to check the latency 
unsigned int counter=0; 

/* Function name - senddata
   Arguements - Data(Whatever that is meant to be sent)
   Return - None
   Function - It sends data to the ubidots device from the esp32 
 */
void senddata(uint8_t data) 
{
  String payload ="ubidots/1.0|POST|BBUS-4iHI00OFsTzAZxPcQrS5ITwe2hHwLA|esp32=>data_send:"+data+"$day-hour=11$day-minute=23|end"; // this is the format in which the data has to be sent to the ubidots server using TCP, Refer the documentation of ubidots
  Serial.println("The message to be sent is in the format: "); 
  Serial.println(payload);

  if (client.connected()) //Checking if the connection still persists 
  {
    client.print(payload); //Sending the data to the server
    current_time=millis();// Recording the time at the which the message is sent 
    // Serial.println("Data Sent");
  } else 
  {
    Serial.println("Client not connected");
  }
}

void setup() 
{
  Serial.begin(UARTBAUD);
  Ethernet.begin(mac); //Initializing hte ethernet module
  delay(1000); // Give time to initialize Ethernet

  Serial.print("localIP: ");
  Serial.println(Ethernet.localIP().toString());

  Serial.println("Connecting to Ubidots...");
  while(client.connected()!=true)
  {
  if (client.connect(host, LISTENPORT)) // Attempting to connect to the Server 
  {
    Serial.println("Connected to Ubidots...");
    senddata(1); //Sending the data
  } else 
  {
    Serial.println("Trying again..."); // Keep trying to connect until the connection is succesfull, a connection timeout can be added if desired
    Serial.println("Connecting to Ubidots...");
    delay(1000);
  }
  }
}

void loop() {
  if (client.connected()) 
  {
    while (client.available()) // Checking for responses from the server after sending the message  
     {
      char c = client.read(); // Reading a single byte 
      if(counter<1)
      latency=millis()-current_time; // Calculating the time elapsed between sending the message and the recieving the response from the server
      Serial.println(c);
      if(counter<1)
      {
      Serial.println("Latency is:"); // Printing latency only once 
      Serial.print(latency);
      }
      counter+=1;
      
    }
  } else {
    Serial.println("Reconnecting..."); 
    if (client.connect(host, LISTENPORT)) // if the connection was lost, reconnecting to the server
    {
      Serial.println("Reconnected to Ubidots");
    } else 
    {
      Serial.println("Failed to reconnect");
    }
    delay(DELAY);
  }

  Ethernet.maintain(); // this function is used to maintain a constant IP address 
}
