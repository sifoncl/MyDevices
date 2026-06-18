// #include <Arduino.h>

// #define TOUCH_BUTTON_PIN 32
// #define TOUCH_THRESHOLD 5
// #define DEBOUNCE_TIME 50

// unsigned long lastTouchTime = 0;
// bool touchState = false;
// bool lastTouchState = false;
// int touchValue = 0;

// void touchButtonLoop()
// {
//     touchValue = touchRead(TOUCH_BUTTON_PIN);
//     bool currentTouch = (touchValue < TOUCH_THRESHOLD);

//     // Антидребезг
//     if (currentTouch != lastTouchState)
//     {
//         lastTouchTime = millis();
//     }

//     if ((millis() - lastTouchTime) > DEBOUNCE_TIME)
//     {
//         if (currentTouch != touchState)
//         {
//             touchState = currentTouch;

//             if (touchState)
//             {
//                 Serial.println("Touch button pressed");
//             }
//         }
//     }

//     lastTouchState = currentTouch;
// }

// void setup(){
//     Serial.begin(115200);

// };

// void loop()
// {
//     Serial.println(touchRead(TOUCH_BUTTON_PIN));
//     touchButtonLoop();
// }