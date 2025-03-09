#include "raylib.h"
#include "raymath.h"
#include "Planet.h"
#include "Slider.h"
#include "Direction.h"
#include <iostream>
#include <format>


int main(){
    InitWindow(1145, 745, "Orbit Explorer");
    SetTargetFPS(60);

    std::vector<int> codepoints;
    for (int i=32; i<128; i++) { codepoints.push_back(i); }
    Font font16 = LoadFontEx("resources/arial.ttf", 16, &codepoints[0], int(codepoints.size()));

    // window:
    int height = GetScreenHeight();
    int width = GetScreenWidth();
    Vector2 center = {width/2.f, height/2.f}; 

    Planet planet{Vector2{320, 372}};
    planet.startPoint = planet.getPosition();
    
    Circle cof;
    cof.fillColor = YELLOW;
    cof.position = Vector2{width/2.f, height/2.f};
    cof.radius = 20;
    
    Slider forceSlider(10, 10, "Force");
    forceSlider.setRange(1000, 10000);
    forceSlider.setSliderValue(4000);
    Slider velSlider(10, 70, "Velocity");
    velSlider.setRange(0.0f, 10.0f);
    float initVel = 3.5f;
    velSlider.setSliderValue(initVel);
    Slider dirSlider(10, 130, "Direction");
    dirSlider.setRange(0.f, 360.f);
    float initAngle = 90.f;
    dirSlider.setSliderValue(initAngle);
    Direction direction({10, 190}, 30);
    Slider animSlider(10, 670, "Anim. Speed");
    animSlider.setRange(0.f, 5.f);
    animSlider.setSliderValue(1.5f);
    float animationSpeed{1.5f};
    
    planet.velocity = {std::cosf(initAngle * DEG2RAD) * initVel, -std::sinf(initAngle * DEG2RAD) * initVel};

    float minVel = 3.5f;
    float maxVel = 3.5f;
    float minDist = Vector2Distance(planet.getPosition(), center);
    float maxDist = minDist;


    const int TRACES_SIZE{1200};
    Vector2 traces[TRACES_SIZE];
    for (int i = 0; i < TRACES_SIZE; i++) {
        traces[i] = Vector2{-1, -1};
    }
    int traceIndex{0};
    int tillNextTrace{5};
    
    bool isTracing{true};
    bool isRunning{false};
    bool showHelp(true);

    while (!WindowShouldClose()){
        
        // poll input:
        if (IsKeyReleased(KEY_F)) {
            ToggleBorderlessWindowed();
        }
        if (IsKeyReleased(KEY_G)) { isRunning = !isRunning; } // 'G': go/stop
        if (IsKeyReleased(KEY_R)) { // 'R': reset planet to start position/velocity
            planet.setPosition(planet.startPoint);
            float v = velSlider.sliderValue;
            float a = dirSlider.sliderValue;
            planet.velocity = {std::cosf(a * DEG2RAD) * v, -std::sinf(a * DEG2RAD) * v};
            direction.velAngle = std::atan2(-planet.velocity.y, planet.velocity.x) * RAD2DEG;
            minVel = v; maxVel = v;
            minDist = Vector2Distance(planet.getPosition(), center); maxDist = minDist;
        } 
        if (IsKeyReleased(KEY_T)) { isTracing = !isTracing; }  // 'T': trace planet's path
        if (IsKeyReleased(KEY_C)) { // 'C': clear traces
            for (int i = 0; i < TRACES_SIZE; i++) {
                traces[i].x = -1.f;
            }
        }
        if (IsKeyReleased(KEY_SLASH)) {
            if (isRunning) { isRunning = false; }
            showHelp = !showHelp;
        }
        // check for resize
        int newheight = GetScreenHeight();
        int newwidth = GetScreenWidth();
        if (newheight != height || newwidth != width) {
            width = newwidth;
            height = newheight;
            Vector2 currentPlanetVectorToCenter = center - planet.getPosition();
            center = {width/2.f, height/2.f};
            cof.position = center;
            planet.setPosition(center - currentPlanetVectorToCenter);
        }
        
        // read controls:
        float startDir = dirSlider.sliderValue;
        direction.startAngle = startDir;
        
        // run sim:
        if (isRunning) {
            // inputs
            animationSpeed = animSlider.getSliderValue();
            float force = forceSlider.sliderValue; // 
            float distance;
            const int divisor = 5;
            for (int i = 0; i < divisor; i++) {
                // advance planet
                planet.setPosition(planet.getPosition() + planet.velocity * animationSpeed / divisor);
                // calculate and apply force
                Vector2 centripetalDir = (center - planet.getPosition()); // whither the center
                distance = Vector2Length(centripetalDir); // how far
                centripetalDir /= distance; // normalize vector
                float impulse = animationSpeed * force / (std::pow(distance, 2) * divisor); // how strong a push?
                planet.velocity += centripetalDir * impulse; // apply push
            }
            // update readouts
            direction.velAngle = std::atan2(-planet.velocity.y, planet.velocity.x) * RAD2DEG;
            if (distance > maxDist) { maxDist = distance; }
            if (distance < minDist) { minDist = distance; }
            float velMag = Vector2Length(planet.velocity);
            if (velMag > maxVel) { maxVel = velMag; }
            if (velMag < minVel) { minVel = velMag; }
        }
        
        // drawing:
        BeginDrawing();
        ClearBackground(BLACK);

        // controls:
        if (isTracing) {
            if (isRunning) {
                tillNextTrace -= 1;
                if (tillNextTrace == 0) {
                    tillNextTrace = 5;
                    traceIndex += 1;
                    if (traceIndex >= TRACES_SIZE)
                        traceIndex = 0;
                    traces[traceIndex] = planet.getPosition();
                }
            }
            for (int i = 0; i < TRACES_SIZE; i++) {
                if (traces[i].x >= 0)
                    DrawPixelV(traces[i], WHITE);
            }
        }

        forceSlider.draw();
        velSlider.draw();
        dirSlider.draw();
        direction.draw();
        animSlider.yCord = height - 85;
        animSlider.setSliderValue(animSlider.getSliderValue());
        animSlider.draw();

        // display measurements:
        float left = width - 235;
        float right = width - 150;
        DrawTextEx(font16, TextFormat("Vel: %.2f", Vector2Length(planet.velocity)), Vector2{left, 10}, 16, 0.8f, WHITE); // vel
        DrawTextEx(font16, TextFormat("Min / Max: %.2f / %.2f", minVel, maxVel), Vector2{right, 10}, 16, 0.8f, WHITE); // min/max vel
        DrawTextEx(font16, TextFormat("Dist: %.2f", Vector2Length(planet.getPosition() - center)), Vector2{left, 30}, 16, 0.8f, WHITE); // dist
        DrawTextEx(font16, TextFormat("Min / Max: %.2f / %.2f", minDist, maxDist), Vector2{right, 30}, 16, 0.8f, WHITE); // min/max dist
        DrawTextEx(font16, TextFormat("Current Pos: (%03d, %03d)",  int(planet.getPosition().x), int(planet.getPosition().y)), Vector2{left, 50}, 16, 0.8f, WHITE);
        DrawTextEx(font16, TextFormat("Start Pos: (%03d, %03d)",  int(planet.startPoint.x), int(planet.startPoint.y)), Vector2{left, 70}, 16, 0.8f, WHITE);

        // draw axes:
        DrawLineEx(Vector2{0, height/2.f}, Vector2{float(width), height/2.f}, 1.f, WHITE);
        DrawLineEx(Vector2{width/2.f, 0}, Vector2{width/2.f, float(height)}, 1.f, WHITE);

        //DrawTextEx(font16, TextFormat("W/H: %d/%d", newwidth, newheight), Vector2{500, 10}, 16, 0.8f, WHITE); // vel

        // draw bodies:
        cof.draw();
        planet.draw(); // planet on top in case of overlap
        
        
        // draw help screen:
        if (showHelp) {
            DrawRectangle(center.x - 88, center.y - 80, 176, 160, BLACK);
            DrawRectangleLines(center.x - 88, center.y - 80, 176, 160, WHITE);
            //'F': toggle fullscreen
            DrawTextEx(font16, "'F': toggle fullscreen", Vector2{center.x - 75, center.y - 70}, 16, 0.8f, WHITE);
            DrawTextEx(font16, "'G': go/stop", Vector2{center.x - 75, center.y - 50}, 16, 0.8f, WHITE);
            DrawTextEx(font16, "'R': reset", Vector2{center.x - 75, center.y - 30}, 16, 0.8f, WHITE);
            DrawTextEx(font16, "'T': trace orbit", Vector2{center.x - 75, center.y - 10}, 16, 0.8f, WHITE);
            DrawTextEx(font16, "'C': clear traces", Vector2{center.x - 75, center.y + 10}, 16, 0.8f, WHITE);
            DrawTextEx(font16, "'?': this help screen", Vector2{center.x - 75, center.y + 30}, 16, 0.8f, WHITE);
            DrawTextEx(font16, "drag planet for new start", Vector2{center.x - 75, center.y + 50}, 16, 0.8f, WHITE);        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}


// check IsWindowResized()

// web compilation:

// em++ -std=c++20 -o game.html src/main.cpp src/Slider.cpp -Os -Wall \
-I ~/dev/emsdk/upstream/emscripten/cache/sysroot/include \
-L ~/dev/emsdk/upstream/emscripten/cache/sysroot/lib/libraylib.a -s USE_GLFW=3 -s ASYNCIFY --preload-file resources \
--shell-file minshell.html -DPLATFORM_WEB ~/dev/emsdk/upstream/emscripten/cache/sysroot/lib/libraylib.a
//

// put this back in to load FT libs
//-I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/opt/libpng/include/libpng16 \

// freetype cflags
// -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/opt/libpng/include/libpng16

// to run:
  // emrun game.html

// OR: