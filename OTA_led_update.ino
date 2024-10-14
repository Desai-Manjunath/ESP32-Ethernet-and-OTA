#include <WiFi.h>          // WiFi library for ESP32
#include <WiFiClient.h>     // Handles TCP connections
#include <WebServer.h>      // WebServer library to handle HTTP requests
#include <ESPmDNS.h>        // mDNS library to give a domain name to the local IP address
#include <Update.h>         // Library for over-the-air (OTA) firmware updates

// WiFi credentials and hostname
const char* host = "ESP32";           // Hostname for mDNS (e.g., http://ESP32.local)
const char* ssid = "Ap";              // SSID of the WiFi network
const char* password = "apbhat07";    // Password for the WiFi network

WebServer server(80);  // Initialize a web server object on port 80 (HTTP)

/* CSS style for HTML pages */
String style =
"<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
"input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
"#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
"#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
"form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

/* HTML for login page */
String loginIndex = 
"<form name=loginForm>"
"<h1>ESP32 Login</h1>"
"<input name=userid placeholder='User ID'> "
"<input name=pwd placeholder=Password type=Password> "
"<input type=submit onclick=check(this.form) class=btn value=Login></form>"
"<script>"
"function check(form) {"
"if(form.userid.value=='admin' && form.pwd.value=='admin')"
"{window.open('/serverIndex')}"  // If correct credentials are entered, redirect to the server index page
"else"
"{alert('Error Password or Username')}"  // Display an error for wrong login
"}"
"</script>" + style;  // Append the CSS style

/* HTML for server index page */
String serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
"<label id='file-input' for='file'>   Choose file...</label>"
"<input type='submit' class=btn value='Update'>"
"<br><br>"
"<div id='prg'></div>"
"<br><div id='prgbar'><div id='bar'></div></div><br></form>"
"<script>"
"function sub(obj){"
"var fileName = obj.value.split('\\\\');"
"document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
"};"
"$('form').submit(function(e){"
"e.preventDefault();"  // Prevent the default form submission
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"  // Create FormData object to hold file data
"$.ajax({"
"url: '/update',"
"type: 'POST',"  // Send file data to the server as a POST request
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"  // Show progress of the upload
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"$('#bar').css('width',Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!') "  // Log success message when file is successfully uploaded
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>" + style;

/* Setup function */
void setup(void) {
  Serial.begin(115200);   // Start serial communication for debugging
  pinMode(23,OUTPUT);     // Set pin 23 as output for controlling an LED

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  // Show dots while waiting for connection
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);  // Print the connected SSID
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  // Print the local IP assigned by WiFi router

  // Use mDNS for host name resolution (e.g., http://ESP32.local)
  if (!MDNS.begin(host)) { 
    Serial.println("Error setting up MDNS responder!");  // Print error if MDNS fails to start
    while (1) {
      delay(1000);  // Retry every second if MDNS fails
    }
  }
  Serial.println("mDNS responder started");

  // Serve the login page when accessing the root ("/") URL
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });

  // Serve the index page after successful login
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  // Handle firmware update request
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");  // Send success or failure message
    ESP.restart();  // Restart ESP32 after the update
  }, []() {
    HTTPUpload& upload = server.upload();  // Access the uploaded file data

    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());  // Log the filename of the uploaded file
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {  // Begin OTA update
        Update.printError(Serial);  // Print error if OTA update fails
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {  // Write file data to flash
        Update.printError(Serial);  // Print error if writing to flash fails
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {  // Complete the update
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);  // Log success and reboot
      } else {
        Update.printError(Serial);  // Print error if update fails to end correctly
      }
    }
  });

  server.begin();  // Start the web server
}

void loop(void) {
  server.handleClient();  // Handle any incoming HTTP requests

  // Example LED blink logic
  digitalWrite(23, HIGH);  // Turn the LED on
  delay(1000);  // Wait for 1 second
  Serial.println("Hello, LED ON");  // Log message when LED is on

  digitalWrite(23, LOW);  // Turn the LED off
  delay(1000);  // Wait for 1 second
  Serial.println("Hello, LED OFF");  // Log message when LED is off

  delay(1);  // Short delay to prevent excessive looping
}
