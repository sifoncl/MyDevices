// #include "LedController.hpp"

// #define DIN 27
// #define CS 26
// #define CLK 25

// #define Segments 4

// #define delayTime 200 // Delay between Frames

// LedController lc = LedController();  

// ByteBlock rocket= {
//   B00000000,
//   B00001111,
//   B00111110,
//   B11111101,
//   B00111110,
//   B00001111,
//   B00000000,
//   B00000000
// };

// ByteBlock rocketColumns;

// //sets all rows on all displays to 0
// void switchLED(){
//   static bool LEDON = false;
//   if(LEDON){
//     digitalWrite(13, LOW);
//   }else{
//     digitalWrite(13, HIGH);
//   }
//   LEDON = !LEDON;
// }

// void setup(){

//   lc.init(DIN,CLK,CS,Segments); // Pins: DIN,CLK,CS, # of Display connected


//   lc.makeColumns(rocket, &rocketColumns);
//     lc.setIntensity(15);

//   pinMode(13, OUTPUT);
    
// }

// void loop(){

//   lc.clearMatrix();
  
//   //Let the rocket fly in
//   for(int i = 0;i < 8*(Segments+1);i++){
//     delay(delayTime);

//     //blink led for each iteration
//     switchLED();

//     //if rocket not fully inside let it fly in and shift it
//     if(i < 8){
//       lc.moveRight(rocketColumns[i]);   
//     }else{
//       lc.moveRight();

//       delay(delayTime);
//       switch(i % 6){
//         case(3):
//         case(4):
//         case(5):
//           lc.moveUp();
//           break;

//         case(0):
//         case(1):
//         case(2):
//           lc.moveDown();
//           break;

//         default:
//           break;
//       }
//     }
        
//   }

//   delay(delayTime);

//   for(int i = 0;i < 8*(Segments+1);i++){
//     delay(delayTime);

//     //blink led for each iteration
//     switchLED();

//     //if rocket not fully inside let it fly in and shift it
//     if(i < 8){
//       lc.moveLeft(rocketColumns[i]);   
//     }else{
//       lc.moveLeft();

//       delay(delayTime);
//       switch(i % 6){
//         case(3):
//         case(4):
//         case(5):
//           lc.moveUp();
//           break;

//         case(0):
//         case(1):
//         case(2):
//           lc.moveDown();
//           break;

//         default:
//           break;
//       }
//     }
        
//   }

//   delay(delayTime);

// }


//     byte CountDigits[10][8] =
//     {
//       {0xe, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0xe, 0x0},  //0
//       {0x2, 0x6,  0xe,  0x6,  0x6,  0x6,  0x6, 0x0},  //1
//       {0xe, 0x1b, 0x3,  0x6,  0xc,  0x18, 0x1f, 0x0}, //2
//       {0xe, 0x1b, 0x3,  0xe,  0x3,  0x1b, 0xe, 0x0},  //3
//       {0x3, 0x7,  0xf,  0x1b, 0x1f, 0x3,  0x3, 0x0},  //4
//       {0x1f, 0x18, 0x1e, 0x3,  0x3,  0x1b, 0xe, 0x0}, //5
//       {0xe, 0x1b, 0x18, 0x1e, 0x1b, 0x1b, 0xe, 0x0},  //6
//       {0x1f, 0x3,  0x6,  0xc,  0xc,  0xc,  0xc, 0x0}, //7
//       {0xe, 0x1b, 0x1b, 0xe,  0x1b, 0x1b, 0xe, 0x0},  //8
//       {0xe, 0x1b, 0x1b, 0xf,  0x3,  0x1b, 0xe, 0x0}   //9
//     };
