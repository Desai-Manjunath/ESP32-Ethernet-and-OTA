#define MYIPADDR 192,168,31,195 // IP address of the server you want to connect to, make sure that the server and the esp32 is connected to the same network
// Port number that the server is listening on
#define LISTENPORT 5000 
//defines the data transfer bw host and ESP
#define UARTBAUD 115200 
// Delay between connection attempts
#define DELAY 2000 
// ENC28J60 library for Ethernet communication
#include <UIPEthernet.h> 
// Create a client instance for TCP communication
EthernetClient client;        
String msg;         
uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};  // Custom MAC address for the enc28j60 ethernet module
IPAddress myIP(MYIPADDR); // storing the id address in the required format in the variable myIP

void setup() 
{
  // Initialize serial communication
  Serial.begin(UARTBAUD); 
  // Configure static IP address
  Ethernet.begin(mac); 
  // Log the IP, subnet, gateway, and DNS information for debugging
  Serial.print("localIP: ");
  Serial.println(Ethernet.localIP().toString());
  Serial.print("subnetMask: ");
  Serial.println(Ethernet.subnetMask().toString());
  Serial.print("gatewayIP: ");
  Serial.println(Ethernet.gatewayIP().toString());
  Serial.print("dnsServerIP: ");
  Serial.println(Ethernet.dnsServerIP().toString());
  Serial.println("Waiting to connect...");
  while(client.connected()!=true)
  {
    client.connect(myIP,LISTENPORT);// Try connecting to the server , you can add a timeout if you desire to
    delay(2000);
  }
  Serial.println(F("Client is connected to the server"));
  client.print("IR_sensor,1,123428"); //send data to the server  
}

void loop() 
{
  Ethernet.maintain();
 }
