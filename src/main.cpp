#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>
#include <HardwareSerial.h>

#include "User_setup.h"

#define MSP_ATTITUDE 108

// Colors
#define BACKGROUND_COLOR TFT_BLACK
#define HORIZON_COLOR TFT_BLUE
#define SKY_COLOR TFT_CYAN
#define GROUND_COLOR TFT_BROWN
#define WHITE_COLOR TFT_WHITE
#define YELLOW_COLOR TFT_YELLOW
#define RED_COLOR TFT_RED

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite background = TFT_eSprite(&tft);
TFT_eSprite horizonSky = TFT_eSprite(&tft);
TFT_eSprite horizon = TFT_eSprite(&tft);
int displayWidth = 0;
int displayHeight = 0;
int displayCenterX = 0;
int displayCenterY = 0;

uint8_t requestMSP[] = {'$', 'M', '<', 0x00, MSP_ATTITUDE, (uint8_t)(MSP_ATTITUDE ^ 0x00)};

// PFD parameters
float pitch = 0;       // degrees
float roll = 0;        // degrees
float airspeed = 120;  // knots
float altitude = 4500; // feet
float heading = 180;   // degrees
float vspeed = 0;      // feet per minute

// Function declarations
void drawPFD();
void drawArtificialHorizon(float pitch, float roll);
void drawAirspeedIndicator(float speed);
void drawAltitudeIndicator(float altitude, float vspeed);
void drawVerticalSpeedIndicator(float vspeed, int x, int y);
void drawHeadingIndicator(float heading);
void drawCenterReticle();
void printMemoryInfo();
void sendMSPRequest();
bool readAttitude(int16_t &roll, int16_t &pitch, int16_t &yaw);

void setup()
{
    Serial.begin(9600);
    Serial1.begin(115200, SERIAL_8N1, 21, 22);

    printMemoryInfo();
    tft.init();

    displayHeight = tft.height();
    displayWidth = tft.width();
    displayCenterX = displayWidth / 2;
    displayCenterY = displayHeight / 2;

    tft.fillScreen(BACKGROUND_COLOR);

    if (background.createSprite(displayWidth, displayHeight) == nullptr)
        Serial.println("background Sprite not created :(");

    background.setPivot(displayCenterX, displayCenterY);

    if (horizon.createSprite(displayWidth, displayHeight) == nullptr)
        Serial.println("horizon Sprite not created :(");

    printMemoryInfo();
}

void loop()
{
    sendMSPRequest();
    if (false)
    {
        pitch = 50.0 * sin(millis() / 3000.0);
        roll = 120.0 * sin(millis() / 5000.0);
        airspeed = 120 + 20 * sin(millis() / 8000.0);
        altitude = 4500 + 500 * sin(millis() / 9000.0);
        heading = fmod(180 + 30 * sin(millis() / 6000.0), 360);
        vspeed = 500 * sin(millis() / 4000.0);
    }
    else
    {

        int16_t rollRaw, pitchRaw, yawRaw;
        readAttitude(rollRaw, pitchRaw, yawRaw);
        roll = rollRaw/10.0f;
        pitch = pitchRaw/10.0f;

        Serial.println(pitch);
        Serial.println(roll);
    }
    // Simulate changing flight parameters

    drawPFD();
}

void sendMSPRequest()
{
    Serial1.write(requestMSP, sizeof(requestMSP));
}

bool readAttitude(int16_t &roll, int16_t &pitch, int16_t &yaw)
{
    if (Serial1.available() < 11)
        return false;

    if (Serial1.read() != '$')
        return false;
    if (Serial1.read() != 'M')
        return false;
    if (Serial1.read() != '>')
        return false;

    uint8_t dataSize = Serial1.read();
    uint8_t cmd = Serial1.read();

    if (cmd != MSP_ATTITUDE || dataSize != 6)
        return false;

    roll = Serial1.read() | (Serial1.read() << 8);
    pitch = Serial1.read() | (Serial1.read() << 8);
    yaw = Serial1.read() | (Serial1.read() << 8);

    uint8_t checksum = Serial1.read(); // Not verified here, but could be added

    return true;
}

void printMemoryInfo()
{
    Serial.println("\n===== Memory Report =====");

    // Internal SRAM
    Serial.printf("Free SRAM (Heap): %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Largest SRAM Block: %d bytes\n", ESP.getMaxAllocHeap());

    // PSRAM (if available)
    if (psramFound())
    {
        Serial.printf("Total PSRAM: %d bytes\n", ESP.getPsramSize());
        Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
        Serial.printf("Largest PSRAM Block: %d bytes\n", ESP.getMaxAllocPsram());
    }
    else
    {
        Serial.println("No PSRAM detected!");
    }

    // Detailed heap info
    heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
    if (psramFound())
    {
        heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
    }
}

void drawPFD()
{
    background.fillSprite(BACKGROUND_COLOR);
    // Draw the artificial horizon
    drawArtificialHorizon(pitch, roll);

    // Draw the airspeed indicator
    // drawAirspeedIndicator(airspeed);

    // Draw the altitude indicator
    // drawAltitudeIndicator(altitude, vspeed);

    // Draw the heading indicator
    // drawHeadingIndicator(heading);

    // Draw the center reticle
    drawCenterReticle();

    background.pushSprite(0, 0);
}

void drawArtificialHorizon(float pitchDeg, float rollDeg)
{
    const int pitchStep = 5; // degrees between ladder lines
    const int ladderLineLength = 20;
    const int longLineLength = 40;

    horizon.fillSprite(BACKGROUND_COLOR);

    // Sky and ground rotating background
    for (int y = 0; y < displayHeight; ++y)
    {
        uint16_t color = (y < displayCenterY + pitchDeg * 2) ? SKY_COLOR : GROUND_COLOR;
        horizon.drawFastHLine(0, y, displayWidth, color);
    }

    // Pitch ladder lines
    for (int pitch = -45; pitch <= 45; pitch += pitchStep)
    {
        if (pitch == 0)
            continue; // skip center line here

        int y = displayCenterY - pitch * 2 + pitchDeg * 2;

        int lineLength = (pitch % 10 == 0) ? longLineLength : ladderLineLength;
        horizon.drawFastHLine(displayCenterX - lineLength / 2, y, lineLength, WHITE_COLOR);

        if (pitch % 10 == 0)
        {
            horizon.setTextColor(WHITE_COLOR);
            horizon.setTextSize(1);
            horizon.setCursor(displayCenterX + longLineLength / 2 + 2, y - 3);
            horizon.print(abs(pitch));
            horizon.setCursor(displayCenterX - longLineLength / 2 - 10, y - 3);
            horizon.print(abs(pitch));
        }
    }

    horizon.fillTriangle(displayCenterX - 5, displayCenterY - 80, displayCenterX + 5, displayCenterY - 80, displayCenterX, displayCenterY - 95, RED_COLOR);

    // Rotate horizon based on roll
    horizon.pushRotated(&background, rollDeg, BACKGROUND_COLOR);

    background.drawArc(displayCenterX, displayCenterY, 95, 121, 90, 270, SKY_COLOR, SKY_COLOR, false);
    background.drawArc(displayCenterX, displayCenterY, 95, 121, 0, 90, GROUND_COLOR, GROUND_COLOR, false);
    background.drawArc(displayCenterX, displayCenterY, 95, 121, 270, 360, GROUND_COLOR, GROUND_COLOR, false);
    background.drawFastHLine(0, displayCenterY, 25, WHITE_COLOR);
    background.drawFastHLine(displayWidth - 25, displayCenterY, 25, WHITE_COLOR);
    background.drawSmoothCircle(displayCenterX, displayCenterY, 95, BACKGROUND_COLOR, BACKGROUND_COLOR);

    // Roll scale arc
    for (int angle = -60; angle <= 60; angle += 10)
    {
        float rad = radians(angle);
        int r1 = 97;
        int r2 = (angle % 30 == 0) ? 112 : 107;
        int x1 = displayCenterX + r1 * sin(rad);
        int y1 = displayCenterY - r1 * cos(rad);
        int x2 = displayCenterX + r2 * sin(rad);
        int y2 = displayCenterY - r2 * cos(rad);
        background.drawLine(x1, y1, x2, y2, WHITE_COLOR);
    }
}

// void drawArtificialHorizon(float pitchDeg, float rollDeg)
// {
//     int diffPitch = ((int)pitchDeg);

//     background.fillRect(0, 0, displayWidth, displayCenterY, SKY_COLOR);
//     background.fillRect(0, displayCenterY, displayWidth, displayCenterY, GROUND_COLOR);
//     background.drawLine(0, displayCenterY + 1, displayWidth, displayCenterY + 1, WHITE_COLOR);
//     background.drawLine(0, displayCenterY, displayWidth, displayCenterY, WHITE_COLOR);
//     background.drawLine(0, displayCenterY - 1, displayWidth, displayCenterY - 1, WHITE_COLOR);

//     background.fillCircle(displayCenterX, displayCenterY, 98, WHITE_COLOR);

//     background.fillCircle(displayCenterX, displayCenterY, 95, SKY_COLOR);

//     horizon.fillSprite(BACKGROUND_COLOR);
//     horizon.fillCircle(displayCenterX, displayCenterY, 95, GROUND_COLOR);
//     horizon.fillRect(0, 0, displayHeight, displayCenterX + diffPitch, BACKGROUND_COLOR);

//     horizon.fillTriangle(displayCenterX - 3, displayCenterY - 80, displayCenterX + 3, displayCenterY - 80, displayCenterX, displayCenterY - 95, RED_COLOR);

//     horizon.pushRotated(&background, rollDeg, BACKGROUND_COLOR);
// }

void drawAirspeedIndicator(float speed)
{
    int x = 0;
    int width = 40;
    int height = 140;
    int centerY = displayHeight / 2;

    background.fillRect(x, centerY - height / 2, width, height, BACKGROUND_COLOR);
    background.drawRect(x, centerY - height / 2, width, height, WHITE_COLOR);

    float minSpeed = fmaxf(0.0f, speed - 60.0f);
    float maxSpeed = speed + 60.0f;

    for (int s = floor(minSpeed / 20) * 20; s <= maxSpeed; s += 20)
    {
        if (s < 0)
            continue;

        float pos = centerY - ((s - speed) * 2);

        if (pos >= centerY - height / 2 && pos <= centerY + height / 2)
        {
            background.drawFastHLine(x + 10, pos, 20, WHITE_COLOR);

            background.setTextColor(WHITE_COLOR, BACKGROUND_COLOR);
            background.setTextSize(1);
            background.setCursor(x + 5, pos - 6);
            background.print(s);

            if (s % 20 == 0)
            {
                for (int i = 1; i <= 3; i += 2)
                {
                    float subPos = pos + (i * 10);
                    if (subPos >= centerY - height / 2 && subPos <= centerY + height / 2)
                    {
                        background.drawFastHLine(x + 15, subPos, 15, WHITE_COLOR);
                    }
                }
            }
        }
    }

    background.fillRect(x, centerY - 15, width, 30, RED_COLOR);
    background.setTextColor(WHITE_COLOR, RED_COLOR);
    background.setTextSize(2);
    background.setCursor(x + 5, centerY - 8);
    background.print(int(speed));
}

void drawAltitudeIndicator(float altitude, float vspeed)
{
    int x = displayWidth - 40;
    int width = 40;
    int height = 140;
    int centerY = displayHeight / 2;

    background.fillRect(x, centerY - height / 2, width, height, BACKGROUND_COLOR);
    background.drawRect(x, centerY - height / 2, width, height, WHITE_COLOR);

    float minAlt = max(0.0f, altitude - 500.0f);
    float maxAlt = altitude + 500.0f;

    for (int a = floor(minAlt / 100) * 100; a <= maxAlt; a += 100)
    {
        if (a < 0)
            continue;

        float pos = centerY - ((a - altitude) * 0.4);

        if (pos >= centerY - height / 2 && pos <= centerY + height / 2)
        {
            background.drawFastHLine(x, pos, 20, WHITE_COLOR);

            background.setTextColor(WHITE_COLOR, BACKGROUND_COLOR);
            background.setTextSize(1);
            background.setCursor(x + 22, pos - 6);
            background.print(a / 100);

            if (a % 100 == 0)
            {
                for (int i = 1; i <= 4; i++)
                {
                    float subPos = pos + (i * 20);
                    if (subPos >= centerY - height / 2 && subPos <= centerY + height / 2)
                    {
                        background.drawFastHLine(x + 5, subPos, 15, WHITE_COLOR);
                    }
                }
            }
        }
    }

    background.fillRect(x, centerY - 15, width, 30, RED_COLOR);
    background.setTextColor(WHITE_COLOR, RED_COLOR);
    background.setTextSize(2);
    background.setCursor(x + 5, centerY - 8);
    background.print(int(altitude / 100));

    // drawVerticalSpeedIndicator(vspeed, x + width + 5, centerY);
}

void drawVerticalSpeedIndicator(float vspeed, int x, int centerY)
{
    int width = 15;
    int height = 80;

    background.fillRect(x, centerY - height / 2, width, height, BACKGROUND_COLOR);
    background.drawRect(x, centerY - height / 2, width, height, WHITE_COLOR);

    int needlePos = centerY - constrain(vspeed / 100, -3, 3) * (height / 6);

    background.fillTriangle(x, needlePos - 5,
                            x + width, needlePos,
                            x, needlePos + 5, RED_COLOR);

    for (int i = -2; i <= 2; i++)
    {
        if (i == 0)
            continue;
        int y = centerY - i * (height / 6);
        background.drawFastHLine(x + 3, y, 5, WHITE_COLOR);
        background.setTextColor(WHITE_COLOR, BACKGROUND_COLOR);
        background.setTextSize(1);
        background.setCursor(x + 10, y - 4);
        background.print(abs(i * 2));
    }
}

void drawHeadingIndicator(float heading)
{
    int centerX = displayWidth / 2;
    int topY = 0;
    int width = 180;
    int height = 20;

    background.fillRect(centerX - width / 2, topY, width, height, BACKGROUND_COLOR);
    background.drawRect(centerX - width / 2, topY, width, height, WHITE_COLOR);

    float minHeading = heading - 60;
    float maxHeading = heading + 60;

    for (int h = floor(minHeading / 10) * 10; h <= maxHeading; h += 10)
    {
        int normH = (h + 360) % 360;
        float pos = centerX + (h - heading) * 1.5;

        if (pos >= centerX - width / 2 && pos <= centerX + width / 2)
        {
            background.drawFastVLine(pos, topY + 5, 10, WHITE_COLOR);

            if (normH % 30 == 0)
            {
                background.setTextColor(WHITE_COLOR, BACKGROUND_COLOR);
                background.setTextSize(1);
                background.setCursor(pos - 6, topY + 15);
                background.print(normH / 10);
            }
        }
    }

    background.fillRect(centerX - 20, topY, 40, height, RED_COLOR);
    background.setTextColor(WHITE_COLOR, RED_COLOR);
    background.setTextSize(2);
    background.setCursor(centerX - 15, topY + 2);
    background.print(int(heading));
}

void drawCenterReticle()
{
    background.fillCircle(displayCenterX, displayCenterY, 5, YELLOW_COLOR);
    background.drawFastHLine(displayCenterX - 25, displayCenterY, 50, YELLOW_COLOR);
    background.drawFastVLine(displayCenterX, displayCenterY - 15, 10, YELLOW_COLOR);
}