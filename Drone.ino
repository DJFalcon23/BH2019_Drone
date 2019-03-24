
#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif

#define PIN 8 //data pin for arduino
#define WIDTH 32 //width of dotmatrix display
#define HEIGHT 8 //heigh of dotmatrix display

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(WIDTH, HEIGHT, PIN,
                            NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
                            NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG); // orient pixel 0 to top left of display, set in zigzag pattern

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};

// Listen to the default port 5555, the Yún webserver
// will forward there all the HTTP requests you send
BridgeServer server;
int x    = matrix.width();
int pass = 0;
int scrollDistance;

void setup() {
  //matrix startup
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(colors[0]);

  // Bridge startup
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  server.begin();
}

bool checkForConnection()
{
  BridgeClient client = server.accept();
  if(client)
  {
    matrix.fillScreen(0);
    return false;
  }
  else
  {
    return true;
  }
}

void loop() {
  // Get clients coming from server
  BridgeClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    Serial.print(F("found client"));
    process(client);

    // Close connection and free resources.
    client.stop();
  }

  delay(50); // Poll every 50ms
}

void process(BridgeClient client) {
  // read the command
  Serial.println(F("In process"));
  String command = client.readStringUntil('/');
  command.trim();
  Serial.println("command: " + command);
  if (command == "GET") //only allow gets
  {
    command = client.readStringUntil('/');
    Serial.println("command: " + command);
    if (command == "drone")
    {
      delay(65);
      String action = client.readStringUntil('/');
      Serial.println("action: " + action);
      if (action == "postMessage")
      {
        matrix.fillScreen(0);
        Serial.println("Found new code");
        String message = client.readStringUntil('/');
        message.trim();
        message = message.substring(0, message.length() - 4);
        message.replace("%20"," ");
        message.replace("%27","'");
        Serial.println("Message: " + message);
        client.stop();
        displayMessage(message, (0-message.length()*5));
      }
    }
  }
}

void displayMessage(String message, int scrollDist)
{
  while(checkForConnection())
  {
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    matrix.print(message);
    if (--x < scrollDist) {
      x = matrix.width();
    }
    matrix.show();
    delay(45);
  }
  matrix.fillScreen(0);
  matrix.setCursor(0, 0);
}