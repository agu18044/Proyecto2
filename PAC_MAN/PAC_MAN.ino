//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
 * Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 * Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 * Con ayuda de: José Guerra
 * IE3027: Electrónica Digital 2 - 2019
 */
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <EEPROM.h>

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};  

//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char const bitmap[],int columns, int index, char flip, char offset);

class fantasma{
    int fdir;
    int fx, fy;
    int sprite,sprite2;
public:
    fantasma(int x,int y,int sprite1);
    void dibujar_fantasma();
    void borrar_fantasma();
    void mover_fantasma();
    void choque_pacman(); 
};

unsigned char const mapa2[50][50] = {
"((((((((((((((((((((((((((((((((((((((((",
"(@@@@@@@@@@@@@@@@@#(@@@@@@@@@@@@@@@@@@#(",
"(@######@########@#(@########@#######@#(",
"(@#(((((@#(((((((@#(@#(((((((@#((((((@#(",
"(@#(((((@#(((((((@#(@#(((((((@#((((((@#(",
"(@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#(",
"(@######@##@##############@##@#######@#(",
"(@#(((((@#(@#(((((((((((((@#(@#((((((@#(",
"(@@@@@@@@#(@@@@@@@#(@@@@@@@#(@@@@@@@@@#(",
"(#######@#(######@#(@#######(@#########(",
"((((((((@#(((((((@#(@#(((((((@#(((((((((",
"#######(@#(@@@@@@@@@@@@@@@@#(@#(########",
"#######(@#(@#######H######@#(@#(########",
"((((((((@#(@#(((((#H#(((((@#(@#(((((((((",
"_@@@@@@@@@@@#(#HHHHHHHHH#(@@@@@@@@@@@@@_",
"_#######@##@#(###########(@##@#########_",
"((((((((@#(@#(((((((((((((@#(@#(((((((((",
"#######(@#(@@@@@@@@@@@@@@@@#(@#(########",
"#######(@#(######@##@#######(@#(########",
"((((((((@#(((((((@#(@#(((((((@#(((((((((",
"(@@@@@@@@#(@@@@@@@#(@@@@@@@#(@@@@@@@@@#(",
"(@######@#(@#######(######@#(@#######@#(",
"(@#(((((@#(@#(((((((((((((@#(@#((((((@#(",
"(@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@#(",
"(@######@########@##@########@#######@#(",
"(@#(((((@#(((((((@#(@#(((((((@#((((((@#(",
"(@#(((((@#(((((((@#(@#(((((((@#((((((@#(",
"(@@@@@@@@@@@@@@@@@#(@@@@@@@@@@@@@@@@@@#(",
"(##################(###################(",
"((((((((((((((((((((((((((((((((((((((((",
};

void inicio(void);
unsigned char monedas[32][40];
void pintar_mapa(void);
bool colisionCon(void);

int x=1,y=27,ax=15,by=14,dir=1, fant, vidas=3, puntaje=0, puntaje2=0,Nmonedas=0, turno=1, selec=0, seldir=1, tiempo=200,modo=1, jugadores=1;
byte nota;
long previousMillis = 0;        // will store last time LED was updated
long interval = 12000;           // interval at which to blink (milliseconds)
unsigned long currentMillis;

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  LCD_Init();
  LCD_Clear(0x00);
  pinMode(PC_7, INPUT);
  pinMode(PA_4, INPUT);
  pinMode(PD_7, INPUT);
  pinMode(PD_6, INPUT);
  pinMode(PF_4, INPUT_PULLUP);
  pinMode(PF_0, INPUT_PULLUP);
  Wire.begin();
 //LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);
    

}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
iniciojuego:
    fantasma A(15,14,1);
    fantasma B(17,14,2);
    fantasma C(19,14,3);
    //fantasma D(21,14,4);
    inicio();
  while(selec==0){
    if (digitalRead(PC_7) == LOW) {seldir=2;jugadores=2;}
    if (digitalRead(PD_7) == LOW) {seldir=1;jugadores=1;}
    if (digitalRead(PF_4) == LOW) {selec=1;goto jugador1;}
    switch(seldir){
      case 1:
        LCD_Sprite(64,128, 16, 16, jugadorD, 3,0, 0, 0);
        for(int a=64;a<80;a++){V_line(a, 160, 15, 0x0);}
        break;
      case 2:
        LCD_Sprite(64,160, 16, 16, jugadorD, 3,0, 0, 0);
        for(int a=64;a<80;a++){V_line(a, 128, 15, 0x0);}
        break;
        }  
      }
  //creacion del mapa 
jugador1: 
  LCD_Clear(0);
  nota=1;
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(nota);              // sends one byte  
  Wire.endTransmission();    // stop transmitting
  delay(1000);
  nota=0;
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(nota);              // sends one byte  
  Wire.endTransmission();    // stop transmitting
  pintar_mapa();
  LCD_Sprite(x*8, y*8, 16, 16, jugadorD, 3, 0, 0, 0);
  //LCD_Sprite(ax*8, by*8, 16, 16, FRAR, 2, 0, 0, 0);
  //jugador 1
  while(selec==1){
  //dir=1 derecha, dir=2 izquierda, dir=3 abajo, dir=4 arriba
  if (digitalRead(PD_6) == LOW) {dir=1;}
  if (digitalRead(PA_4) == LOW) {dir=2;}
  if (digitalRead(PC_7) == LOW) {dir=3;}
  if (digitalRead(PD_7) == LOW) {dir=4;}
  if(vidas==0){
        LCD_Clear(0);
        if(seldir==1 and jugadores==1){
        LCD_Print("Game Over", 80,32,2,0xF800,0x0);
        LCD_Print("Score :", 80,64,1,0xFFFF,0x0);
        LCD_Print(String(puntaje), 130,64,1,0xFFFF,0x0);
        while(1){
          if (digitalRead(PF_0) == LOW) {selec=0;puntaje=0;vidas=3;goto iniciojuego;}
          }
        //LCD_Print(String(Nmonedas), 130,100,1,0xFFFF,0x0);
                }
        if(seldir==2 and jugadores==2 and turno==1){
        LCD_Print("Game Over", 80,32,2,0xF800,0x0);
        LCD_Print("Score J1:", 80,64,1,0xFFFF,0x0);
        LCD_Print(String(puntaje), 130,100,1,0xFFFF,0x0);
        delay(5000);
        goto jugador2;
        }
         if(seldir==2 and jugadores ==2 and turno==2){
        LCD_Print("Game Over", 80,32,2,0xF800,0x0);
        LCD_Print("Score J2:", 80,64,1,0xFFFF,0x0);
        LCD_Print(String(puntaje2), 130,100,1,0xFFFF,0x0);
        delay(5000);
        //LCD_Print(String(Nmonedas), 130,100,1,0xFFFF,0x0);
               
       while(1){
        //LCD_Print("Game Over", 80,32,2,0xF800,0x0);
        if(puntaje>puntaje2){LCD_Print("Jugador 1 gana", 80,64,1,0xFFFF,0x0);}
        if(puntaje2>puntaje){LCD_Print("Jugador 2 gana", 80,64,1,0xFFFF,0x0);}
        if(puntaje=puntaje2){LCD_Print("Empate", 80,64,1,0xFFFF,0x0);}
        if (digitalRead(PF_0) == LOW) {selec=0;puntaje=0;puntaje2=0;vidas=3;goto iniciojuego;}
        } }
                }
  switch(dir){
    case 1:
      if(mapa2[y][x+1] == '@') {
        if(mapa2[y][x+2] == '_'){
          x=0;
          for(int a=287;a<320;a++){
            V_line(a, y*8, 15, 0x0);
            }
        }
        x++;
        if(x>40){x=40;}
        delay(tiempo);
        int comiendo = (x/4)%2;
        LCD_Sprite((x*8), y*8, 16, 16, jugadorD, 3, comiendo, 0, 0);
        for(int a=(x*8)-8;a<(x*8)+1;a++){
          if(a>8){
          V_line(a, y*8, 15, 0x0);
          }
          }
      if(monedas[y][x]=='1'){
        if((x==1 and y==3) or (x==1 and y==25) or (x==37 and y==3) or (x==37 and y==25)){tiempo=1;modo=2;previousMillis = currentMillis;}  
        puntaje=puntaje+10;Nmonedas--;}
      monedas[y][x]='0';
      
      }
      
     break;
    case 2:
      if(mapa2[y][x-1] == '@') {
        if(mapa2[y][x-2] == '_'){x=38;
          for(int a=0;a<32;a++){
            V_line(a, y*8, 15, 0x0);
            }
        }
        x--;
        if(x<1){x=0;}
        delay(tiempo);
        int comiendo = (x/8)%2;
        LCD_Sprite((x*8), y*8, 16, 16, jugadorI, 3, comiendo, 0, 0);
        for(int a=(x*8)+24;a>(x*8)+15;a--){
          if(a<311 and mapa2[y][x] == '@'){
          V_line(a, y*8, 15, 0x0);}
          }  
        if(monedas[y][x]=='1'){puntaje=puntaje+10;Nmonedas--;if((x==1 and y==3) or (x==1 and y==25) or (x==37 and y==3) or (x==37 and y==25)){tiempo=1;modo=2;previousMillis = currentMillis;}  }
        monedas[y][x]='0';
          }
          break;
    case 3:
       if(mapa2[y+1][x] == '@') {
          y++;
          if(y>29){y=28;}
          delay(tiempo);
          int comiendo = (y/4)%2;
          LCD_Sprite((x*8), y*8, 16, 16, jugadorAb, 3, comiendo, 0, 0);
          for(int a=((y-1)*8);a<(y*8);a++){
            H_line(x*8, a, 15, 0x0);
            }
           if(monedas[y][x]=='1'){puntaje=puntaje+10;Nmonedas--;if((x==1 and y==3) or (x==1 and y==25) or (x==37 and y==3) or (x==37 and y==25)){tiempo=1;modo=2;previousMillis = currentMillis;}  }
           monedas[y][x]='0';
           }
           
           break;
     
    case 4:
      if(mapa2[y-1][x] == '@') {
          y--;
         //if(y>29){y=28;}
          delay(tiempo);
          int comiendo = (y/4)%2;
          LCD_Sprite((x*8), y*8, 16, 16, jugadorAr, 3, comiendo, 0, 0);
          for(int a=((y+3)*8)-1;a>(y*8)+15;a--){
            H_line(x*8, a, 15, 0x0);
            }
           if(monedas[y][x]=='1'){puntaje=puntaje+10;Nmonedas--;if((x==1 and y==3) or (x==1 and y==25) or (x==37 and y==3) or (x==37 and y==25)){tiempo=1;modo=2;previousMillis = currentMillis;}  }
           monedas[y][x]='0';
           }
           
           break;        
    }
    //((x*8==(ax*8) and y*8<=(by*8)+16 ) or (y*8==(by*8) and x*8==(ax*8)+16 )or(x*8==ax*8 and (y*8)-16== by*8 )or ( y*8==by*8 and x*8==(ax*8)+16))
     
    
    if(Nmonedas==0){
      LCD_Clear(0);
      if(seldir==1 and jugadores ==1){
        LCD_Print("Has ganado", 80,32,2,0xF800,0x0);
        LCD_Print("Score:", 80,64,1,0xFFFF,0x0);
        LCD_Print(String(puntaje), 130,64,1,0xFFFF,0x0);
        //LCD_Print(String(Nmonedas), 130,100,1,0xFFFF,0x0);
        while(1){
          if (digitalRead(PF_0) == LOW) {selec=0;puntaje=0;vidas=3;goto iniciojuego;}
          }}    
        if(seldir==2 and jugadores==2 and turno==1){
        LCD_Print("Nivel terminado", 80,32,2,0xF800,0x0);
        LCD_Print("Score J1:", 80,64,1,0xFFFF,0x0);
        LCD_Print(String(puntaje), 130,100,1,0xFFFF,0x0);
        delay(5000);
        goto jugador2;
        }
         if(seldir==2 and jugadores ==2 and turno==2){
        LCD_Print("Nivel terminado", 80,32,2,0xF800,0x0);
        LCD_Print("Score J2:", 80,64,1,0xFFFF,0x0);
        LCD_Print(String(puntaje2), 130,100,1,0xFFFF,0x0);
        delay(5000);
        LCD_Clear(0);
        
        //LCD_Print(String(Nmonedas), 130,100,1,0xFFFF,0x0);
               
       while(1){
        //LCD_Print("Game Over", 80,32,2,0xF800,0x0);
        
        if(puntaje>puntaje2){LCD_Print("Jugador 1 gana", 80,64,1,0xFFFF,0x0);}
        if(puntaje2>puntaje){LCD_Print("Jugador 2 gana", 80,64,1,0xFFFF,0x0);}
        else if(puntaje=puntaje2){LCD_Print("Empate", 80,64,1,0xFFFF,0x0);}
        if (digitalRead(PF_0) == LOW) {selec=0;puntaje=0;puntaje2=0;vidas=3;goto iniciojuego;}
        } }
      }
      A.mover_fantasma();
      B.mover_fantasma();
      C.mover_fantasma();
      //D.mover_fantasma();
      //
      currentMillis = millis();
 
      if(currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
          if (modo == 2){
          tiempo=50;
          modo=1;}
  }}
/////////////////////////////////////////////////////////jugador 2
jugador2:
  vidas=3;
  turno=2;
  LCD_Clear(0);
  nota=1;
  Nmonedas=0;
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(nota);              // sends one byte  
  Wire.endTransmission();    // stop transmitting
  delay(1000);
  nota=0;
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(nota);              // sends one byte  
  Wire.endTransmission();    // stop transmitting
  pintar_mapa();
  LCD_Sprite(x*8, y*8, 16, 16, jugadorD, 3, 0, 0, 0);
  while(selec==1){
  //dir=1 derecha, dir=2 izquierda, dir=3 abajo, dir=4 arriba
  if (digitalRead(PD_6) == LOW) {dir=1;}
  if (digitalRead(PA_4) == LOW) {dir=2;}
  if (digitalRead(PC_7) == LOW) {dir=3;}
  if (digitalRead(PD_7) == LOW) {dir=4;}
  if(vidas==0){
        LCD_Clear(0);
        if(seldir==1 and jugadores==1){
        LCD_Print("Game Over", 80,32,2,0xF800,0x0);
        LCD_Print("Score :", 80,64,1,0xFFFF,0x0);
        LCD_Print(String(puntaje), 130,64,1,0xFFFF,0x0);
        while(1){
          if (digitalRead(PF_0) == LOW) {selec=0;puntaje=0;vidas=3;goto iniciojuego;}
          }
        //LCD_Print(String(Nmonedas), 130,100,1,0xFFFF,0x0);
                }
        if(seldir==2 and jugadores==2 and turno==1){
        LCD_Print("Game Over", 80,32,2,0xF800,0x0);
        LCD_Print("Score J1:", 80,64,1,0xFFFF,0x0);
        LCD_Print(String(puntaje), 130,100,1,0xFFFF,0x0);
        delay(5000);
        goto jugador2;
        }
         if(seldir==2 and jugadores ==2 and turno==2){
        LCD_Print("Game Over", 80,32,2,0xF800,0x0);
        LCD_Print("Score J2:", 80,64,1,0xFFFF,0x0);
        LCD_Print(String(puntaje2), 130,100,1,0xFFFF,0x0);
        delay(5000);
        LCD_Clear(0);
        //LCD_Print(String(Nmonedas), 130,100,1,0xFFFF,0x0);
       while(1){
        //LCD_Print("Game Over", 80,32,2,0xF800,0x0);
        if(puntaje>puntaje2){LCD_Print("Jugador 1 gana", 80,64,1,0xFFFF,0x0);}
        if(puntaje2>puntaje){LCD_Print("Jugador 2 gana", 80,64,1,0xFFFF,0x0);}
        else if(puntaje=puntaje2){LCD_Print("Empate", 80,64,1,0xFFFF,0x0);}
        if (digitalRead(PF_0) == LOW) {selec=0;puntaje=0;puntaje2=0;vidas=3;goto iniciojuego;}
        } }
                }

  switch(dir){
    case 1:
      if(mapa2[y][x+1] == '@') {
        if(mapa2[y][x+2] == '_'){
          x=0;
          for(int a=287;a<320;a++){
            V_line(a, y*8, 15, 0x0);
            }
        }
        x++;
        if(x>40){x=40;}
        delay(tiempo);
        int comiendo = (x/4)%2;
        LCD_Sprite((x*8), y*8, 16, 16, jugadorD, 3, comiendo, 0, 0);
        for(int a=(x*8)-8;a<(x*8)+1;a++){
          if(a>8){
          V_line(a, y*8, 15, 0x0);
          }
          }
      if(monedas[y][x]=='1'){
        if((x==1 and y==3) or (x==1 and y==25) or (x==37 and y==3) or (x==37 and y==25)){tiempo=1;modo=2;previousMillis = currentMillis;}  
        puntaje2=puntaje2+10;Nmonedas--;}
      monedas[y][x]='0';
      
      }
      
     break;
    case 2:
      if(mapa2[y][x-1] == '@') {
        if(mapa2[y][x-2] == '_'){x=38;
          for(int a=0;a<32;a++){
            V_line(a, y*8, 15, 0x0);
            }
        }
        x--;
        if(x<1){x=0;}
        delay(tiempo);
        int comiendo = (x/8)%2;
        LCD_Sprite((x*8), y*8, 16, 16, jugadorI, 3, comiendo, 0, 0);
        for(int a=(x*8)+24;a>(x*8)+15;a--){
          if(a<311 and mapa2[y][x] == '@'){
          V_line(a, y*8, 15, 0x0);}
          }  
        if(monedas[y][x]=='1'){puntaje2=puntaje2+10;Nmonedas--;if((x==1 and y==3) or (x==1 and y==25) or (x==37 and y==3) or (x==37 and y==25)){tiempo=1;modo=2;previousMillis = currentMillis;}  }
        monedas[y][x]='0';
          }
          break;
    case 3:
       if(mapa2[y+1][x] == '@') {
          y++;
          if(y>29){y=28;}
          delay(tiempo);
          int comiendo = (y/4)%2;
          LCD_Sprite((x*8), y*8, 16, 16, jugadorAb, 3, comiendo, 0, 0);
          for(int a=((y-1)*8);a<(y*8);a++){
            H_line(x*8, a, 15, 0x0);
            }
           if(monedas[y][x]=='1'){puntaje2=puntaje2+10;Nmonedas--;if((x==1 and y==3) or (x==1 and y==25) or (x==37 and y==3) or (x==37 and y==25)){tiempo=1;modo=2;previousMillis = currentMillis;}  }
           monedas[y][x]='0';
           }
           
           break;
     
    case 4:
      if(mapa2[y-1][x] == '@') {
          y--;
         //if(y>29){y=28;}
          delay(tiempo);
          int comiendo = (y/4)%2;
          LCD_Sprite((x*8), y*8, 16, 16, jugadorAr, 3, comiendo, 0, 0);
          for(int a=((y+3)*8)-1;a>(y*8)+15;a--){
            H_line(x*8, a, 15, 0x0);
            }
           if(monedas[y][x]=='1'){puntaje2=puntaje2+10;Nmonedas--;if((x==1 and y==3) or (x==1 and y==25) or (x==37 and y==3) or (x==37 and y==25)){tiempo=1;modo=2;previousMillis = currentMillis;}  }
           monedas[y][x]='0';
           }
           
           break;        
    }
    //((x*8==(ax*8) and y*8<=(by*8)+16 ) or (y*8==(by*8) and x*8==(ax*8)+16 )or(x*8==ax*8 and (y*8)-16== by*8 )or ( y*8==by*8 and x*8==(ax*8)+16))
     
    
    if(Nmonedas==0){
      LCD_Clear(0);
        LCD_Print("Has ganado", 80,32,2,0xF800,0x0);
        LCD_Print("Score:", 80,64,1,0xFFFF,0x0);
        LCD_Print(String(puntaje2), 130,64,1,0xFFFF,0x0);
        LCD_Print(String(Nmonedas), 130,100,1,0xFFFF,0x0);
        while(1){
          if (digitalRead(PF_0) == LOW) {goto iniciojuego;selec=0;puntaje2=0;vidas=3;}
          }
      }
      A.mover_fantasma();
      B.mover_fantasma();
      C.mover_fantasma();
      //D.mover_fantasma();
      //
      currentMillis = millis();
 
      if(currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
          if (modo == 2){
          tiempo=50;
          modo=1;}
  }}

  

}
//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char const bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
  for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k - 2;
     } 
  }
  }else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
  }
    
    
    }
  digitalWrite(LCD_CS, HIGH);
}

void pintar_mapa()
{
     for(int i = 0 ; i <40 ; i++){
         for(int j = 0 ; j < 30 ; j++){
               if(mapa2[j][i] == '(') {FillRect(i*8,j*8,8,8, 0x008D);}
               else if(mapa2[j][i] == '@') {LCD_Sprite((i*8)+4, (j*8)+4, 8, 8, M_pe, 1, 0, 0, 0); monedas[j][i]='1';Nmonedas++;}
               
         }
     }
     LCD_Sprite((1*8)+4, (3*8)+4, 8, 8, M_gra, 1, 0, 0, 0);
     LCD_Sprite((1*8)+4, (25*8)+4, 8, 8, M_gra, 1, 0, 0, 0);
     LCD_Sprite((37*8)+4, (3*8)+4, 8, 8, M_gra, 1, 0, 0, 0);
     LCD_Sprite((37*8)+4, (25*8)+4, 8, 8, M_gra, 1, 0, 0, 0);
}

void inicio(){
  LCD_Clear(0);
  LCD_Print("PAC-MAN", 80,32,2,0xFFE0,0x0);
  LCD_Print("1 JUGADOR", 80,128,1,0xFFFF,0x0);
  LCD_Print("2 JUGADOR", 80,160,1,0xFFFF,0x0);
  }

  fantasma::fantasma(int x , int y, int sprite1){
fx = x;
fy = y;
sprite=sprite1;
sprite2=sprite1;
fdir = random(1,5);
}

void fantasma::dibujar_fantasma(){
int anim = (fy/4)%2;
switch(sprite){
  case 1:   LCD_Sprite(fx*8, fy*8, 16, 16, FRAR, 2, anim, 0, 0); break;
  case 2:   LCD_Sprite(fx*8, fy*8, 16, 16, FYAR, 2, anim, 0, 0); break;
  case 3:   LCD_Sprite(fx*8, fy*8, 16, 16, FBAR, 2, anim, 0, 0); break;
  case 4:   LCD_Sprite(fx*8, fy*8, 16, 16, FPAR, 2, anim, 0, 0); break;
  case 5:   LCD_Sprite(fx*8, fy*8, 16, 16, FRAB, 2, anim, 0, 0); break;
  case 6:   LCD_Sprite(fx*8, fy*8, 16, 16, FYAB, 2, anim, 0, 0); break;
  case 7:   LCD_Sprite(fx*8, fy*8, 16, 16, FBAB, 2, anim, 0, 0); break;
  case 8:   LCD_Sprite(fx*8, fy*8, 16, 16, FPAB, 2, anim, 0, 0); break;
  case 9:   LCD_Sprite(fx*8, fy*8, 16, 16, FRD, 2, anim, 0, 0); break;
  case 10:  LCD_Sprite(fx*8, fy*8, 16, 16, FYD, 2, anim, 0, 0); break;
  case 11:  LCD_Sprite(fx*8, fy*8, 16, 16, FBD, 2, anim, 0, 0); break;
  case 12:  LCD_Sprite(fx*8, fy*8, 16, 16, FYD, 2, anim, 0, 0); break;
  case 13:  LCD_Sprite(fx*8, fy*8, 16, 16, FPD, 2, anim, 0, 0); break;
  case 14:  LCD_Sprite(fx*8, fy*8, 16, 16, FRI, 2, anim, 0, 0); break;
  case 15:  LCD_Sprite(fx*8, fy*8, 16, 16, FYI, 2, anim, 0, 0); break;
  case 16:  LCD_Sprite(fx*8, fy*8, 16, 16, FBI, 2, anim, 0, 0); break;
  case 17:  LCD_Sprite(fx*8, fy*8, 16, 16, FPI, 2, anim, 0, 0); break;
  case 18:  LCD_Sprite(fx*8, fy*8, 16, 16, FC, 2, anim, 0, 0); break;
  case 19:  LCD_Sprite(fx*8, fy*8, 16, 16, FCT, 2, anim, 0, 0); break;
  case 20:  LCD_Sprite(fx*8, fy*8, 16, 16, F_muerte, 2, anim, 0, 0); break;
  }
}

void fantasma::borrar_fantasma(){
  switch(fdir){
    case 1: if(mapa2[fy-1][fx] == '@' or mapa2[fy-1][fx] == 'H'){
            for(int a=((fy+3)*8)-1;a>(fy*8)+15;a--){H_line(fx*8,a , 15, 0x0);}} break;
    case 2:  if(mapa2[fy][fx+1] == '@' or mapa2[fy][fx+1] == 'H'){
              for(int a=(fx*8)-8;a<(fx*8)+1;a++){
              if(a>8){V_line(a, fy*8, 15, 0x0);}}}
                break;
    case 3: if(mapa2[fy][fx-1] == '@' or mapa2[fy][fx-1] == 'H'){
            for(int a=(fx*8)+24;a>(fx*8)+15;a--){
            if(a<311){V_line(a, fy*8, 15, 0x0);}}}
          break;
    case 4: if(mapa2[fy+1][fx] == '@' or mapa2[fy+1][fx] == 'H'){
              for(int a=((fy-1)*8);a<(fy*8);a++){
            H_line(fx*8,a , 15, 0x0);
            }
               }
          break;
  }
}

void fantasma::choque_pacman(){
if((fx*8>=x*8 and fx*8<x*8+16 and fy*8>y*8 and fy*8 <= y*8+16) or (x*8<=fx*8+16 and x*8>=fx*8 and fy*8==y*8) or (x*8==fx*8 and y*8==fy*8))  {
         
      if (modo==1){
        nota=3;dir=1;
      Wire.beginTransmission(4); // transmit to device #4
      Wire.write(nota);              // sends one byte  
      Wire.endTransmission();    // stop transmitting
      Wire.beginTransmission(4); // transmit to device #4
      Wire.write(0);              // sends one byte  
      Wire.endTransmission();    // stop transmitting
      for(int anim=2;anim<14;anim++){
      LCD_Sprite((x*8), y*8, 16, 16, muerte, 14, anim, 0, 0);
      delay(200);
      }
      for(int a=x*8;a<(x*8)+16;a++){
            V_line(a, y*8, 15, 0x0);}
      for(int a=fx*8;a<(fx*8)+16;a++){
            V_line(a, fy*8, 15, 0x0);} 
        vidas--;x=1;y=27;}
      if(modo==2){
        nota=2;
        Wire.beginTransmission(4); // transmit to device #4
        Wire.write(nota);              // sends one byte  
        Wire.endTransmission();    // stop transmitting
        Wire.beginTransmission(4); // transmit to device #4
        Wire.write(0);              // sends one byte  
        Wire.endTransmission();    // stop transmitting
        for(int anim=2;anim<4;anim++){
        LCD_Sprite((fx*8), fy*8, 16, 16, F_muerte, 4, anim, 0, 0);
        delay(200);
      }
      for(int a=x*8;a<(x*8)+16;a++){
            V_line(a, y*8, 15, 0x0);}
      for(int a=fx*8;a<(fx*8)+16;a++){
            V_line(a, fy*8, 15, 0x0);} 
        puntaje=puntaje+200;
        fy=14;fx=15;}
        
        
        }
      ///vdas
      }


void fantasma::mover_fantasma(){
  //borrar_fantasma();
  choque_pacman();
  int bolx=fx,boly=fy;
  // fdir 1 arriba   f2 derecha    3 izquierda   4 abajo
  if(fx==19 and fy==14){fdir=1;}
  if(fdir==1){if(mapa2[fy-1][fx] == '@' or mapa2[fy-1][fx] == 'H'){fy--;for(int a=((fy+3)*8)-1;a>(fy*8)+15;a--){H_line(fx*8,a , 15, 0x0);}}if(monedas[boly][bolx]=='1'){ LCD_Sprite((bolx*8)+4, (boly*8)+4, 8, 8, M_pe, 1, 0, 0, 0);}} else {fdir=random(1,5);}
  if(fdir==2){if(mapa2[fy][fx+1] == '@' or mapa2[fy][fx+1] == 'H'){fx++;for(int a=(fx*8)-8;a<(fx*8)+1;a++){
              if(a>8){V_line(a, fy*8, 15, 0x0);}}}if(monedas[boly][bolx]=='1'){ LCD_Sprite((bolx*8)+4, (boly*8)+4, 8, 8, M_pe, 1, 0, 0, 0);}} else {fdir=random(1,5);}
  if(fdir==3){if(mapa2[fy][fx-1] == '@' or mapa2[fy][fx-1] == 'H'){fx--;for(int a=(fx*8)+24;a>(fx*8)+15;a--){
            if(a<311){V_line(a, fy*8, 15, 0x0);}}}if(monedas[boly][bolx]=='1'){ LCD_Sprite((bolx*8)+8, (boly*8)+4, 8, 8, M_pe, 1, 0, 0, 0);}} else {fdir=random(1,5);}
  if(fdir==4){if(mapa2[fy+1][fx] == '@' or mapa2[fy+1][fx] == 'H'){fy++;for(int a=((fy-1)*8);a<(fy*8);a++){
            H_line(fx*8,a , 15, 0x0);
            }}if(monedas[boly][bolx]=='1'){ LCD_Sprite((bolx*8)+4, (boly*8)+4, 8, 8, M_pe, 1, 0, 0, 0);}} else {fdir=random(1,5);}
   if(modo==1){sprite = sprite2;}
   if(modo==2){sprite=18;}
  dibujar_fantasma();
 
    
  
  delay(50);
   //borrar_fantasma();
}
