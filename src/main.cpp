#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite background = TFT_eSprite(&tft);
TFT_eSprite horizonSky = TFT_eSprite(&tft);
TFT_eSprite horizon = TFT_eSprite(&tft);

// Display dimensions
#define WIDTH 320
#define HEIGHT 240

// PFD parameters
float pitch = 0;       // degrees
float roll = 0;        // degrees
float airspeed = 120;  // knots
float altitude = 4500; // feet
float heading = 180;   // degrees
float vspeed = 0;      // feet per minute

// Colors
#define BACKGROUND_COLOR TFT_BLACK
#define HORIZON_COLOR TFT_BLUE
#define SKY_COLOR TFT_CYAN
#define GROUND_COLOR TFT_DARKGREEN
#define WHITE_COLOR TFT_WHITE
#define YELLOW_COLOR TFT_YELLOW
#define RED_COLOR TFT_RED

// Function declarations
void drawPFD();
void drawArtificialHorizon(float pitch, float roll);
void drawAirspeedIndicator(float speed);
void drawAltitudeIndicator(float altitude, float vspeed);
void drawVerticalSpeedIndicator(float vspeed, int x, int y);
void drawHeadingIndicator(float heading);
void drawCenterReticle();
void printMemoryInfo();

void setup()
{
    Serial.begin(9600);
    printMemoryInfo();
    tft.init();
    tft.setRotation(1);
    // tft.setSwapBytes(true);
    tft.fillScreen(TFT_BLACK);

    background.setColorDepth(8);
    background.createSprite(320, 240);

    background.setPivot(160, 120);
    // background.setRotation(1);
    // background.setSwapBytes(true);

    horizonSky.setColorDepth(8);
    horizonSky.createSprite(200, 200);
    // horizonSky.setSwapBytes(true);

    horizon.setColorDepth(8);
    horizon.createSprite(200, 200);
    // horizon.setSwapBytes(true);

    printMemoryInfo();
}

void loop()
{
    // Simulate changing flight parameters
    pitch = 50.0 * sin(millis() / 1000.0);
    roll = 120.0 * sin(millis() / 1700.0);
    airspeed = 120 + 20 * sin(millis() / 8000.0);
    altitude = 4500 + 500 * sin(millis() / 9000.0);
    heading = fmod(180 + 30 * sin(millis() / 6000.0), 360);
    vspeed = 500 * sin(millis() / 4000.0);

    drawPFD();
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
    background.fillSprite(TFT_BLACK);
    // Draw the artificial horizon
    drawArtificialHorizon(pitch, roll);

    // Draw the airspeed indicator
    drawAirspeedIndicator(airspeed);

    // Draw the altitude indicator
    drawAltitudeIndicator(altitude, vspeed);

    // Draw the heading indicator
    drawHeadingIndicator(heading);

    // Draw the center reticle
    drawCenterReticle();

    background.pushSprite(0, 0);
}

void drawArtificialHorizon(float pitchDeg, float rollDeg)
{
    int diffPitch = ((int)pitchDeg);
    horizonSky.fillCircle(100, 100, 98, TFT_WHITE);
    horizonSky.fillCircle(100, 100, 95, TFT_BLUE);

    horizon.fillSprite(TFT_BLACK);
    horizon.fillCircle(100, 100, 95, TFT_BROWN);
    // horizon.fillRect(0, 80 + diffPitch, 160, 0 - diffPitch, TFT_BLACK);
    horizon.fillRect(0, 0, 200, 100 + diffPitch, TFT_BLACK);

    // int horizonSize = 40;
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;
    // // Clear area (240x240 centered box)
    // tft.fillRect(centerX - horizonSize / 2, centerY - horizonSize / 2, horizonSize, horizonSize, TFT_BLACK);

    // // Convert to radians
    // float pitchOffset = pitchDeg * 2; // scale: 2 pixels per degree
    // float rollRad = rollDeg * DEG_TO_RAD;

    // // Calculate rotated horizon line endpoints
    // float lineLength = 20;
    // float x1 = -lineLength;
    // float x2 = lineLength;
    // float y1 = pitchOffset;
    // float y2 = pitchOffset;

    // // Rotate points around (0,0)
    // float cosA = cos(rollRad);
    // float sinA = sin(rollRad);

    // float rx1 = x1 * cosA - y1 * sinA;
    // float ry1 = x1 * sinA + y1 * cosA;
    // float rx2 = x2 * cosA - y2 * sinA;
    // float ry2 = x2 * sinA + y2 * cosA;

    // // Shift into center
    // int sx1 = centerX + rx1;
    // int sy1 = centerY + ry1;
    // int sx2 = centerX + rx2;
    // int sy2 = centerY + ry2;

    // // Draw sky and ground halves
    // // tft.fillTriangle(centerX, centerY, sx1, sy1, sx2, sy2, TFT_BLUE); // Sky
    // // tft.fillTriangle(centerX, centerY, sx1, sy1, sx2, sy2, TFT_BLUE);
    // // tft.fillTriangle(centerX, centerY + 40, sx1, sy1 + 40, sx2, sy2 + 40, TFT_BROWN); // Ground

    // // Draw horizon line
    // tft.drawLine(sx1, sy1, sx2, sy2, TFT_WHITE);

    // // Draw center marker
    // horizon.drawLine(centerX - 5, centerY, centerX + 5, centerY, TFT_YELLOW);
    // horizon.drawLine(centerX, centerY - 5, centerX, centerY + 5, TFT_YELLOW);

    horizon.pushToSprite(&horizonSky, 0, 0, TFT_BLACK);
    horizonSky.pushRotated(&background, rollDeg, TFT_BLACK);
}

void drawRollIndicator(float roll)
{
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;
    int radius = 60;

    tft.drawCircle(centerX, centerY, radius, WHITE_COLOR);

    for (int angle = -60; angle <= 60; angle += 10)
    {
        if (angle == 0)
            continue;
        float rad = radians(angle);
        int x1 = centerX + radius * sin(rad);
        int y1 = centerY - radius * cos(rad);

        int markLength = (abs(angle) % 30 == 0) ? 12 : 6;
        int x2 = centerX + (radius - markLength) * sin(rad);
        int y2 = centerY - (radius - markLength) * cos(rad);

        tft.drawLine(x1, y1, x2, y2, WHITE_COLOR);

        if (abs(angle) % 30 == 0)
        {
            tft.setTextColor(WHITE_COLOR, BACKGROUND_COLOR);
            tft.setTextSize(1);
            x2 = centerX + (radius - 20) * sin(rad);
            y2 = centerY - (radius - 20) * cos(rad);
            tft.setCursor(x2 - 6, y2 - 4);
            tft.print(abs(angle));
        }
    }

    tft.fillTriangle(centerX - 5, centerY - radius - 10,
                     centerX, centerY - radius,
                     centerX + 5, centerY - radius - 10, RED_COLOR);
}

void drawAirspeedIndicator(float speed)
{
    int x = 0;
    int width = 40;
    int height = 140;
    int centerY = HEIGHT / 2;

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
    int x = WIDTH - 40;
    int width = 40;
    int height = 140;
    int centerY = HEIGHT / 2;

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
    int centerX = WIDTH / 2;
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
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;

    // background.fillTriangle(centerX - 10, centerY,
    //                         centerX, centerY - 10,
    //                         centerX + 10, centerY, YELLOW_COLOR);
    // background.fillTriangle(centerX - 10, centerY,
    //                         centerX, centerY + 10,
    //                         centerX + 10, centerY, YELLOW_COLOR);
    background.drawWideLine(centerX - 20, centerY, centerX + 20, centerY, 3, TFT_WHITE, TFT_BLACK);
    background.drawWideLine(centerX, centerY + 20, centerX, centerY - 20 , 3, TFT_WHITE, TFT_BLACK);
}