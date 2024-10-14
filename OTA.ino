#include <WiFi.h>       // Wifi library for ESP32
#include <WiFiClient.h> // Library for creating WiFi client
#include <WebServer.h>  // Wifi Server Library to host a web server
#include <ESPmDNS.h>    // Library to assign a domain name to the host's local IP
#include <Update.h>     // Esp32 update library for Over-the-Air (OTA) updates

// Wi-Fi credentials for ESP32 AP (Access Point)
const char* host = "ESP32"; 
const char* ssid = "ESP32_AP";      // SSID for the ESP32 Access Point
const char* password = "12345678";  // Password for the AP (min. 8 characters)

// Initialize a web server on port 80 (HTTP standard port)
WebServer server(80); 

int ledPin = 23; // LED pin declaration

/* CSS Styling for the HTML page */
String style =
"<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
"input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
"#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
"#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
"form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

// HTML for login page
String loginIndex = 
"<form name=loginForm>"
"<h1>ESP32 Login</h1>"
"<input name=userid placeholder='User ID'> "
"<input name=pwd placeholder=Password type=Password> "
"<input type=submit onclick=check(this.form) class=btn value=Login></form>"
"<script>"
"function check(form) {"
"if(form.userid.value=='admin' && form.pwd.value=='admin')"
"{window.open('/serverIndex')} else"
"{alert('Error Password or Username')} }"
"</script>" + style;

// HTML for the server index page (firmware update page)
String serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='/update' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
"<label id='file-input' for='file'>   Choose file...</label>"
"<input type='submit' class=btn value='Update'>"
"<br><br>"
"<div id='prg'></div>"
"<br><div id='prgbar'><div id='bar'></div></div><br></form>"
"<script>"
// JavaScript to handle file selection and upload progress
"function sub(obj){"
"var fileName = obj.value.split('\\\\');"
"document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
"};"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
"$.ajax({"
"url: '/update'," // Send the file to '/update' URL for processing
"type: 'POST'," // Use POST method to upload the file
"data: data," // File data
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
// Show progress while uploading the firmware
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"$('#bar').css('width',Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"}," 
"success:function(d, s) {"
"console.log('success!') " // On success, log the message
"}," 
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>" + style;

void setup(void) {
  Serial.begin(115200); // Start serial communication at 115200 baud rate

  // Initialize the Wi-Fi Access Point (AP)
  WiFi.softAP(ssid, password);  // Create the ESP32 AP with defined SSID and password
  IPAddress IP = WiFi.softAPIP();  // Get the IP address of the AP
  Serial.print("AP IP address: ");
  Serial.println(IP); // Print the AP's IP address

  // Use mDNS for local hostname resolution (e.g., http://esp32.local)
  if (!MDNS.begin(host)) { 
    Serial.println("Error setting up MDNS responder!"); // If MDNS fails, print an error message
    while (1) {
      delay(1000); // Halt execution if MDNS fails
    }
  }
  Serial.println("mDNS responder started"); // MDNS successfully started

  // Route for login page
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex); // Send the login page HTML
  });
  
  // Route for server index page (firmware update page)
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex); // Send the server index page HTML
  });
  
  // Handle firmware file upload (POST request)
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK"); // Send success or failure message
    
    ESP.restart(); // Restart ESP32 after firmware upload
  }, []() {
    // Handle file upload and writing to ESP32's flash memory
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str()); // Print the name of the file being uploaded
      
      // Start the update with unknown size
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { 
        Update.printError(Serial); // Print error if update initialization fails
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // Flash firmware to ESP32's memory
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial); // Print error if writing fails
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      // Finalize the update and verify the firmware size
      if (Update.end(true)) { 
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize); // Print success message
      } else {
        Update.printError(Serial); // Print error if the update fails to end
      }
    }
  });
  
  // Start the server
  server.begin();
}
