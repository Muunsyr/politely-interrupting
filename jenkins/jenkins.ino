#include <Adafruit_NeoPixel.h>

/* A notification light with a set number of colours and different modes.
   Controlled by serial.
*/

/* Globals - improve */
unsigned long blinkTimer;
unsigned long keepAliveTimer;
int led_state;
int command_mode = 1;
int red = 0;
int green = 0;
int blue = 0;
boolean on = true;
boolean blinking = false;

#define BAUD_RATE 9600
#define PIXEL_PIN    11    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 1

enum SerialCommands {
    // Commands to set LED lit status
    SET_BLINKING = 0x32,
    SET_STEADY_ON = 0x33,
    SET_OFF = 0x34,

    // Commands to set colour
    SET_BUILD_BROKEN = 0x36,
    SET_TESTS_FAILED = 0x37,
    SET_ALL_OK = 0x38,
    SET_MISSING_BUILDS = 0x39,
    NO_COMMS = 0x3A,
};

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    Serial.begin(BAUD_RATE);

    // Activate all the LEDs, blink for boot confirmation
    pixel.begin();
    setLEDs(80, 80, 80);
    delay(500);
    blinkTimer = millis();
    keepAliveTimer = blinkTimer;
    execCommand(0x3A); // init status to no connection
}

void loop()
{
    // get time, ensure counters haven't overflowed
    unsigned long tempTime = millis();
    if (tempTime < blinkTimer) {
        blinkTimer = 0;
        keepAliveTimer = 0;
    }
    // Blink led or set it steady
    if (blinking && (tempTime - blinkTimer >= 800)) {
        if (on) {
            setLEDs(0, 0, 0);
            on = false;
        } else {
            setLEDs(red, green, blue);
            on = true;
        }
        // update timer
        blinkTimer = tempTime;
    } else if (!blinking) {
        on = true;
        setLEDs(red, green, blue);
    }
    
    // Check when last update was received, change status if too old
    if (tempTime - keepAliveTimer > 20000) {
        execCommand(NO_COMMS);
        Serial.println("Where are you?");
        keepAliveTimer = tempTime;
    }
    
    // poll the serial buffer, execute commands
    if (Serial.available() > 0) {
      execCommand(Serial.read());
    }
}

// Set the PWM values for the LEDs
void setLEDs(int red, int green, int blue) {
//    analogWrite(LED_RED, 255 - red);
//    analogWrite(LED_GREEN, 255 - green);
//    analogWrite(LED_BLUE, 255 - blue);
  pixel.setPixelColor(0, pixel.Color(red,green,blue)); // First and only pixel
  pixel.show();
}

// apply LED effects
void execCommand(int command) {
    switch(command) {
    case SET_BLINKING:
        Serial.println("blinking");
        blinking = true;
        break;
    case SET_STEADY_ON:
        Serial.println("on");
        on = true;
        blinking = false;
        break;
    case SET_OFF:
        Serial.println("off");
        red = 0;
        green = 0;
        blue = 0;
        blinking = false;
        on = false;
        break;
    case SET_BUILD_BROKEN:
        Serial.println("broken");
        red = 230;
        green = 14;
        blue = 0;
        blinking = true;
        keepAliveTimer = millis();
        break;
    case SET_TESTS_FAILED:
        Serial.println("failed");
        red = 240;
        green = 160;
        blue = 0;
        blinking = true;
        keepAliveTimer = millis();
        break;
    case SET_ALL_OK:
        Serial.println("A-OK");
        red = 0;
        green = 20;
        blue = 110;
        on = true;
        blinking = false;
        keepAliveTimer = millis();
        break;
    case SET_MISSING_BUILDS:
        Serial.println("missing");
        red = 150;
        green = 35;
        blue = 0;
        blinking = true;
        keepAliveTimer = millis();
        break;
    case 0x3A:
        Serial.print("told ");
    default:
        Serial.println("nothing");
        red = 30;
        green = 0;
        blue = 70;
        on = true;
        blinking = false;
        break;
    }
}
