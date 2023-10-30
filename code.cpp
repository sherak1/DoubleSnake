#include "mbed.h"
#include "fonts.h"
#include "stm32f413h_discovery_lcd.h"
#define M_PI 3.14159265358979323846
#define GAME_BACKGROUND (uint32_t)LCD_COLOR_GRAY
#define REFRESH_PERIOD 0.1
#define GAME_PERIOD 0.3

#define MAX_X 34
#define MAX_Y 34

//varijable
InterruptIn leftBtn(p5);
InterruptIn upBtn(p6);
InterruptIn rightBtn(p7);
InterruptIn downBtn(p8);
AnalogIn potHor(p15);
AnalogIn potVer(p16);

Ticker gameTicker;
Ticker mainTicker;

//Mode -> u kojem se modeu igrica trenutno nalazi
enum Mode{
    MAIN_MENU,
    INSTRUCTIONS,
    GAME,
    END_GAME
};

enum Smjer {
    LEFT, UP, RIGHT, DOWN
};

Mode mode = MAIN_MENU;

Smjer smjerPrve = UP;
Smjer smjerDruge = DOWN;

int prvaX[MAX_X*MAX_Y] = {5, 5, 5, 5};
int prvaY[MAX_X*MAX_Y] = {30, 29, 28, 27};
int drugaX[MAX_X*MAX_Y] = {29, 29, 29, 29};
int drugaY[MAX_X*MAX_Y] = {4, 5, 6, 7};

int metaX, metaY;

int duzinaPrve = 4;
int duzinaDruge = 4;

int scorePrve = 0;
int scoreDruge = 0;

bool pobjednikPrvi = true;


void inicijalizirajIgru(){
    //inicijalizacija zmija na početku igre
    smjerPrve = LEFT;
    smjerDruge = RIGHT;

    prvaY[0] = prvaY[1] = prvaY[2] = prvaY[3] = 5;
    prvaX[0] = 30;
    prvaX[1] = 29;
    prvaX[2] = 28;
    prvaX[3] = 27;
    drugaY[0] = drugaY[1] = drugaY[2] = drugaY[3] =29;
    drugaX[0] = 4;
    drugaX[1] = 5;
    drugaX[2] = 6;
    drugaX[3] = 7;

    duzinaPrve = 4;
    duzinaDruge = 4;

    scorePrve = 0;
    scoreDruge = 0;
    
    mode = GAME;
}
//prikazuje ekran na displeju na početnoj
void mainMenu(){
    BSP_LCD_Clear(LCD_COLOR_GRAY);
    BSP_LCD_SetBackColor (LCD_COLOR_GRAY);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    //ivice zmija na početnoj 
    BSP_LCD_DrawRect(0, 0, 239, 239);
    BSP_LCD_DrawRect(1, 1, 237, 237);
    BSP_LCD_DrawRect(2, 2, 235, 235);
    BSP_LCD_DrawRect(3, 3, 233, 233);
    BSP_LCD_DrawRect(4, 4, 231, 231);
    BSP_LCD_FillRect(235,235,5,5);
   
    BSP_LCD_SetTextColor (LCD_COLOR_BLACK);
    BSP_LCD_SetFont(&Font24);
    BSP_LCD_DisplayStringAt(0, 100, (uint8_t*)"TWO SNAKES", CENTER_MODE);
    BSP_LCD_SetTextColor (LCD_COLOR_YELLOW);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(0, 150, (uint8_t*)"Press any button to start", CENTER_MODE);
    
    //zuta zmija
    for(int i=11;i<18;i++)
    { //pozicija zute zmije na početnoj 
        BSP_LCD_FillRect (5, 5+i*7, 5, 5);
        
        
    }
     for(int i=0;i<30;i++)
    {//pozicija zute zmije na početnoj
        BSP_LCD_FillRect (5+i*7,131 , 5, 5);
        
    }
    //pozicija glave zute zmije
   BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect (208, 131, 5, 5);
    
    //bijela zmija
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    
    for(int i=18;i>=11;i--) //rucno  
    {//pozicija bijele zmije na početnoj
        BSP_LCD_FillRect (230, 5+i*7, 5, 5);
        
    }
    
    for(int i=31;i>2;i--)
    {//pozicija bijele zmije na početnoj
        BSP_LCD_FillRect (5+i*7,82 , 5, 5);
        
    }
    //ozicija glave bijele zmije 
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect (26, 82, 5, 5);
}

//drugi dio displeja(instrukcije)
void uputstva(){
    BSP_LCD_Clear(GAME_BACKGROUND);
    BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
    BSP_LCD_SetBackColor (GAME_BACKGROUND);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    char buffer[64];
    
    sprintf(buffer, "INSTRUCTIONS \n");
    BSP_LCD_DisplayStringAt(70, 10, (uint8_t*)buffer, LEFT_MODE);
    sprintf(buffer, "Player 1: \n");
    BSP_LCD_DisplayStringAt(10, 30, (uint8_t*)buffer, LEFT_MODE);
    sprintf(buffer, "Left/Right: Potentiometer p15 \n");
    BSP_LCD_DisplayStringAt(10, 55, (uint8_t*)buffer, LEFT_MODE);
    sprintf(buffer, "Up/Down: Potentiometer p16 \n");
    BSP_LCD_DisplayStringAt(10, 70, (uint8_t*)buffer, LEFT_MODE);
    
    sprintf(buffer, "Player 2: \n");
    BSP_LCD_DisplayStringAt(10, 120, (uint8_t*)buffer, LEFT_MODE);
    
    sprintf(buffer, "Left: Button 5 \n");
    BSP_LCD_DisplayStringAt(10, 145, (uint8_t*)buffer, LEFT_MODE);
    
    sprintf(buffer, "Up: Button 6 \n");
    BSP_LCD_DisplayStringAt(10, 160, (uint8_t*)buffer, LEFT_MODE);
    
    sprintf(buffer, "Right: Button 7 \n");
    BSP_LCD_DisplayStringAt(10, 175, (uint8_t*)buffer, LEFT_MODE);
    
    sprintf(buffer, "Down: Button 8 \n");
    BSP_LCD_DisplayStringAt(10, 190, (uint8_t*)buffer, LEFT_MODE);
    
    sprintf(buffer, "Press any button to start!");
    BSP_LCD_DisplayStringAt(30, 220, (uint8_t*)buffer, LEFT_MODE);
}

//pozicija kvadratica
void drawSquare(int x, int y){ 
    BSP_LCD_FillRect (x*239/MAX_X+1, y*239/MAX_Y+1, 5, 5);
}

//pozicija mete
void drawTarget(int x, int y){
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect (x*239/MAX_X+1, y*239/MAX_Y+1, 5, 5);
}

//displej sa pokretanjem igrice
void igra(){
    char buffer[64]; 
    //uredjivanje pozadine
    BSP_LCD_Clear(GAME_BACKGROUND);
    BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
    BSP_LCD_SetBackColor (GAME_BACKGROUND);
    BSP_LCD_SetFont(&Font12);
    //ivice
    BSP_LCD_DrawRect(0, 0, 239, 239);
    BSP_LCD_DrawRect(1, 1, 237, 237);
    BSP_LCD_DrawRect(2, 2, 235, 235);
    BSP_LCD_DrawRect(3, 3, 233, 233);
    BSP_LCD_DrawRect(4, 4, 231, 231);
    BSP_LCD_FillRect(235,235,5,5);
    
    //score za player1
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    for(int i = 0; i < duzinaPrve - 1; i++) 
        drawSquare(prvaX[i], prvaY[i]);
    
    sprintf(buffer, "Player 1: %d", scorePrve);
    BSP_LCD_DisplayStringAt(5, 10, (uint8_t*)buffer, LEFT_MODE);
    
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    drawSquare(prvaX[duzinaPrve - 1], prvaY[duzinaPrve - 1]);

    //score za player2
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);

    for(int i = 0; i < duzinaDruge - 1; i++)
        drawSquare(drugaX[i], drugaY[i]);

    sprintf(buffer, "Player 2: %d", scoreDruge);
    BSP_LCD_DisplayStringAt(150, 10, (uint8_t*)buffer, LEFT_MODE);

    BSP_LCD_SetTextColor(LCD_COLOR_BLACK); 
    drawSquare(drugaX[duzinaDruge - 1], drugaY[duzinaDruge - 1]);

    drawTarget(metaX, metaY);
}

//sudar dvije zmije - game over
void krajIgre(){
    
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor (LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
    //ivice 
    BSP_LCD_DrawRect(0, 0, 239, 239);
    BSP_LCD_DrawRect(1, 1, 237, 237);
    BSP_LCD_DrawRect(2, 2, 235, 235);
    BSP_LCD_DrawRect(3, 3, 233, 233);
    BSP_LCD_DrawRect(4, 4, 231, 231);
    BSP_LCD_FillRect(235,235,5,5);
    
    BSP_LCD_SetFont(&Font24);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(0, 20, (uint8_t*)"GAME OVER", CENTER_MODE);
    
    //ispisivanje scoreova
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);

    char buffer[64];
    
    sprintf(buffer, "Player 1: %d", scorePrve);
    BSP_LCD_DisplayStringAt(0, 140, (uint8_t*)buffer, CENTER_MODE);

    sprintf(buffer, "Player 2: %d", scoreDruge);
    BSP_LCD_DisplayStringAt(0, 155, (uint8_t*)buffer, CENTER_MODE);
    
    //pobjednik
    BSP_LCD_SetFont(&Font24);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    if(pobjednikPrvi)
        BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Player 1 won!", CENTER_MODE);
    else
        BSP_LCD_DisplayStringAt(0, 80, (uint8_t*)"Player 2 won!", CENTER_MODE);
    
    //go again
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAt(140, 215, (uint8_t*)"Go again",LEFT_MODE);
    
    //povratak na main menu
    BSP_LCD_DisplayStringAt(8, 215, (uint8_t*)"Main menu", LEFT_MODE);
}

//postavljanje mete
void generisiMetu(){
    bool postavljena = false;
    while(!postavljena){
        postavljena = true;
        //pozivamo rand funkciju kako bi koordinata mete uvijek bila nasumicna
        metaX = (rand() % (MAX_X - 2)) + 1;
        metaY = (rand() % (MAX_Y - 2)) + 1;
        
        for(int i = 0; i < duzinaPrve; i++)
            if(prvaX[i] == metaX && prvaY[i] == metaY)
                postavljena = false;
        
        for(int i = 0; i < duzinaDruge; i++)
            if(drugaX[i] == metaX && drugaY[i] == metaY)
                postavljena = false;
    }
}

//ova funkcija ispisuje na displej potreban UI na osnovu varijele Mode
void display(){
    switch (mode) {
        case MAIN_MENU:
            mainMenu();
            break;
        case INSTRUCTIONS:
            uputstva();
            break;
        case GAME:
            igra();
            break;
        case END_GAME:
            krajIgre();
            break;
    }
}
//pomjeranje zmije za jedn polje, ukoliko zmija pojede metu prouduzava se za 1 kao i score
void gameTick(){
    if(mode != GAME) return; //zavrsava se
    
    int prvaGlavaX = prvaX[duzinaPrve - 1];
    int prvaGlavaY = prvaY[duzinaPrve - 1];
    
    switch(smjerPrve){
        case UP:
            prvaY[duzinaPrve - 1] = prvaY[duzinaPrve - 1] - 1;
            if(prvaY[duzinaPrve - 1] == 0)
                prvaY[duzinaPrve - 1] = MAX_Y - 1;
            break;
        case DOWN:
            prvaY[duzinaPrve - 1] = (prvaY[duzinaPrve - 1] + 1) % MAX_Y;
            if(prvaY[duzinaPrve - 1] == 0) prvaY[duzinaPrve - 1] = 1;
            break;
        case LEFT:
            prvaX[duzinaPrve - 1] = prvaX[duzinaPrve - 1] - 1;
            if(prvaX[duzinaPrve - 1] == 0)
                prvaX[duzinaPrve - 1] = MAX_X - 1;
            break;
        case RIGHT:
            prvaX[duzinaPrve - 1] = (prvaX[duzinaPrve - 1] + 1) % MAX_X;
            if(prvaX[duzinaPrve - 1] == 0) prvaX[duzinaPrve - 1] = 1;
            break;
    }
//produzavanje zmije kad pojede metu
    if(prvaX[duzinaPrve - 1] < metaX+1 &&prvaX[duzinaPrve - 1] > metaX-1 && prvaY[duzinaPrve - 1] < metaY+1 &&prvaY[duzinaPrve - 1] > metaY-1 ){
        scorePrve++;
        duzinaPrve++;
        prvaX[duzinaPrve - 1] = prvaX[duzinaPrve - 2];
        prvaY[duzinaPrve - 1] = prvaY[duzinaPrve - 2];
        generisiMetu();
    }else
        for(int i = 0; i < duzinaPrve - 2; i++){
            prvaX[i] = prvaX[i+1];
            prvaY[i] = prvaY[i+1];
        }
    prvaX[duzinaPrve - 2] = prvaGlavaX;
    prvaY[duzinaPrve - 2] = prvaGlavaY;
    
    int drugaGlavaX = drugaX[duzinaDruge - 1];
    int drugaGlavaY = drugaY[duzinaDruge - 1];
    
    switch(smjerDruge){
        case UP:
            drugaY[duzinaDruge - 1] = drugaY[duzinaDruge - 1] - 1;
            if(drugaY[duzinaDruge - 1] == 0)
                drugaY[duzinaDruge - 1] = MAX_Y - 1;
            break;
        case DOWN:
            drugaY[duzinaDruge - 1] = (drugaY[duzinaDruge - 1] + 1) % MAX_Y;
            if(drugaY[duzinaPrve - 1] == 0) drugaY[duzinaPrve - 1] = 1;
            break;
        case LEFT:
            drugaX[duzinaDruge - 1] = drugaX[duzinaDruge - 1] - 1;
            if(drugaX[duzinaDruge - 1] == 0)
                drugaX[duzinaDruge - 1] = MAX_X - 1;
            break;
        case RIGHT:
            drugaX[duzinaDruge - 1] = (drugaX[duzinaDruge - 1] + 1) % MAX_X;
            if(drugaX[duzinaPrve - 1] == 0) drugaX[duzinaPrve - 1] = 1;
            break;
    }

    if(drugaX[duzinaDruge - 1] < metaX+1 &&drugaX[duzinaDruge - 1] > metaX-1 && drugaY[duzinaDruge - 1] < metaY+1 &&drugaY[duzinaDruge - 1] > metaY-1 ){
        scoreDruge++;
        duzinaDruge++;
        drugaX[duzinaDruge - 1] = drugaX[duzinaDruge - 2];
        drugaY[duzinaDruge - 1] = drugaY[duzinaDruge - 2];
        generisiMetu();
    }else
        for(int i = 0; i < duzinaDruge - 2; i++){
            drugaX[i] = drugaX[i+1];
            drugaY[i] = drugaY[i+1];
        }
    drugaX[duzinaDruge - 2] = drugaGlavaX;
    drugaY[duzinaDruge - 2] = drugaGlavaY;
    
    //postavljanje pobjednika ukoliko neka zmija udari drugu i prelazak na game over
    
    for(int i = 0; i < duzinaPrve; i++){
        if(prvaX[duzinaPrve - 1] == prvaX[i] && prvaY[duzinaPrve - 1] == prvaY[i] && i != duzinaPrve - 1){
            pobjednikPrvi = false;
            mode = END_GAME;
            break;
        }
        if(drugaX[duzinaDruge - 1] == prvaX[i] && drugaY[duzinaDruge - 1] == prvaY[i]){
            pobjednikPrvi = true;
            mode = END_GAME;
            break;
        }
    }
    
    
    for(int i = 0; i < duzinaDruge; i++){
        if(prvaX[duzinaPrve - 1] == drugaX[i] && prvaY[duzinaPrve - 1] == drugaY[i]){
            pobjednikPrvi = false;
            mode = END_GAME;
            break;
        }
        if(drugaX[duzinaDruge - 1] == drugaX[i] && drugaY[duzinaDruge - 1] == drugaY[i] && i != duzinaDruge - 1){
            pobjednikPrvi = true;
            mode = END_GAME;
            break;
        }
    }
}
//smjer igraca 1
void player1(Smjer smjer){
    switch (mode) {
        case MAIN_MENU:
            mode = INSTRUCTIONS;
            break;
        case INSTRUCTIONS:
            mode=GAME;
            inicijalizirajIgru();
            break;
        case GAME:
            if(abs(smjerPrve - smjer) != 2)
                smjerPrve = smjer;
            break;
        case END_GAME:
            if(smjer == LEFT)
                mode = MAIN_MENU;
            else if(smjer == RIGHT)
                inicijalizirajIgru();
            break;
    }
}
//smjer igraca 2
void player2(Smjer smjer){
    switch (mode) {
        case MAIN_MENU:
            
            mode=INSTRUCTIONS;
            uputstva();
            break;
        case INSTRUCTIONS:
            mode=GAME;
            inicijalizirajIgru();
            break;
        case GAME:
            if(abs(smjerDruge - smjer) != 2)
                smjerDruge = smjer;
            break;
        case END_GAME:
            if(smjer == LEFT)
                mode = MAIN_MENU;
            else if(smjer == RIGHT)
                inicijalizirajIgru();
            break;
    }
}

void Left(){
    player2(LEFT);
}

void Up(){
    player2(UP);
}

void Right(){
    player2(RIGHT);
}

void Down(){
    player2(DOWN);
}

int main() {
    BSP_LCD_Init();
    srand(time(NULL));
    leftBtn.rise(&Left);
    upBtn.rise(&Up);
    rightBtn.rise(&Right);
    downBtn.rise(&Down);
    gameTicker.attach(&gameTick, GAME_PERIOD);
    mainTicker.attach(&display, REFRESH_PERIOD);

    generisiMetu();
    
    bool isActive = true;
    
    while (1) {
        if(isActive){
            if(potHor.read() > 1.f/3 && potHor.read() < 2.f/3 && potVer.read() > 1./3 && potVer.read() < 2.f/3)
                isActive = false;
        }else{
            isActive = true;
            if(potHor.read() < 1.f/3)
                player1(LEFT);
            else if(potHor.read() > 2.f/3)
                player1(RIGHT);
            else if(potVer.read() < 1.f/3)
                player1(DOWN);
            else if(potVer.read() > 2.f/3)
                player1(UP);
            else{
                isActive= false;
            }
        }
        
        wait(0.01);
    }
}