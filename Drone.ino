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
    bool retVal = true;
    BridgeClient client = server.accept();
    if(client)
    {
        if(isDroneConnection(client))
        {
            retVal = false;
        }
    }
    return retVal;
}

bool isDroneConnection(BridgeClient client)
{
    bool retVal = false;
    String command = client.readStringUntil('/');
    command.trim();
    Serial.println("command: " + command);
    if (command == "GET") //only allow gets
    {
        command = client.readStringUntil('/');
        Serial.println("command: " + command);
        if (command == "drone")
        {
            retVal = true;
        }
    }
    return retVal;
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
  String command = client.readStringUntil('/');
    command.trim();
    if (command == "GET") //only allow gets
    {
        command = client.readStringUntil('/');
        if (command == "drone")
        {
            String action = client.readStringUntil('/');
              if (action == "postMessage")
              {
                matrix.fillScreen(0);
                String message = client.readStringUntil('/');
                message.trim();
                message.replace("%20"," ");
                message.replace("%27","'");
                String color = client.readStringUntil('/');
                color.trim();
                color = color.substring(0, color.length() - 4);
                String r = color.substring(0,color.indexOf('-'));
                color = color.substring(r.length() + 1);
                String g = color.substring(0,color.indexOf('-'));
                color = color.substring(g.length() + 1);
                String b = color;
                client.stop();
                matrix.setTextColor(matrix.Color(r.toInt(), g.toInt(), b.toInt()));
                Serial.println(message);
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

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}