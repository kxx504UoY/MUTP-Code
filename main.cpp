#include "mbed.h"
#include "TextLCD.h"

//------------------------------------------------------IO INITIALISATION------------------------------------------------------//
// Connect these nucleo pins to RS, E, D4, D5, D6 and D7 pins of the LCD
TextLCD lcd(D0, D1, D2, D3, D4, D5, TextLCD::LCD20x4);
DigitalOut fowLED(D6), leftLED(D8), righLED(D7), backLED(D9); //scanLED(D12);
DigitalIn pingButton(D10, PullUp);
DigitalIn moveButton(D11, PullUp);
DigitalIn rotateButton(D12, PullUp);

//----------------------------------------------------------VARIABLES----------------------------------------------------------//
//using shorts for memory efficiency
const short MAP_SIZE = 10; //currently one variable for size - a square map
const short map[MAP_SIZE][MAP_SIZE]={{1,1,1,1,1,1,1,1,1,1},
                                     {1,0,0,0,0,0,0,0,0,1},
                                     {1,0,0,1,1,1,1,1,0,1},
                                     {1,0,0,0,0,0,0,0,0,1},
                                     {1,0,0,0,1,1,1,0,1,1},
                                     {1,0,0,0,0,0,0,0,0,1},
                                     {1,0,0,0,0,0,0,0,0,1},
                                     {1,0,0,0,0,0,0,0,0,1},
                                     {1,0,0,0,0,0,0,0,0,1},
                                     {1,1,1,1,1,1,1,1,1,1}}; //0-empty, 1-wall
short orientation = 0; //0-E, 1-S, 2-W, 3-N
short position[] = {2,3}; //position on map in form {x,y}

short ping_delay=500;   //the delay between pinging each direction
short dist_delay=200;   //the delay for every empty space between you and a wall

short lives=2;  //how many lives player has

//-------------------------------------------------------SONAR FUNCTIONS-------------------------------------------------------//

/* ping
    Send out a ping and light up LEDs if there is a wall detected in foward direction

    returns the distance of nearest wall (-1 if there is none)
*/
short ping(short sonar_orien){
    
    short max_dist;
    short wall_dist = 1; //initialising how far away the closest wall is (1=right next to player)
    
    short direction = sonar_orien+orientation; //coincidentally adding up the two always gives the absolute number for direction
    if (direction>3) direction-=4;              //when reduced to a value within 0-3

    if(direction == 0){ //if east
        max_dist = MAP_SIZE - position[1]; //the distance to scan through

        for(int i=1; i<=max_dist; i++){
            if (map[position[1]][position[0]+i] == 1){
                //there is a wall, return its distance
                return wall_dist;
            }
            else{
                //no wall in this space, distance increased
                wall_dist+=1;
            }
        }
    }
    else if(direction == 1) //if south
    {
        max_dist = MAP_SIZE - position[1];

        for(int i=1; i<=max_dist; i++){
            if (map[position[1]+i][position[0]] == 1){
                //there is a wall, return its distance
                return wall_dist;
            }
            else wall_dist+=1;
        }
    }
    else if(direction == 2) //if west
    {
        max_dist = position[0];

        for(int i=1; i<=max_dist; i++){
            if (map[position[1]][position[0]-i] == 1){
                //there is a wall, return its distance
                return wall_dist;
            }
            else wall_dist+=1;
    }
    }
    else if(direction == 3) //if north
    {
        max_dist = position[1];

        for(int i=1; i<=max_dist; i++){
            if (map[position[1]-i][position[0]] == 1){
                //there is a wall, return its distance
                return wall_dist;
            }
            else wall_dist+=1;
        }
    }
    //if there was no wall detected
    return -1;
}


/* flash
    Parameters: LED - The LED to flash
                dir - The direction (0 to 3) to flash in
    
    Will flash the LED once, and then again after an amount of time proportional to the distance of the nearest wall
*/
void flash(DigitalOut LED, short dir){
    LED = true;
    thread_sleep_for(dist_delay);     //ping LED initially
    LED= false;

    short DelayCount = ping(dir);
    if (DelayCount>0 && DelayCount <=5){ 
        thread_sleep_for(dist_delay * DelayCount);
        LED = true;
        thread_sleep_for(dist_delay);     //ping LED again
        LED= false;
    }
    else {
        // printf("NO WALL");
        thread_sleep_for(ping_delay);
    }

    
}

/* ping_all
    pings in every direction clockwise starting from foward
    (at the moment it will only print how many spaces away the nearest wall is in each direction and doesn't return anything)
*/
void ping_all(){
    //indicate the sonar has started
    // scanLED=true;    
    //fow
    // lcd.locate(6,0);
    // lcd.printf("f:%i",  ping(0));
    flash(fowLED, 0);

    thread_sleep_for(ping_delay);

    //left
    // lcd.locate(11,1);
    // lcd.printf("r:%i", ping(1));
    flash(leftLED, 1);
    
    thread_sleep_for(ping_delay);
    
    //back
    // lcd.locate(6,1);
    // lcd.printf("b:%i", ping(2));
    flash(backLED, 2);
    
    thread_sleep_for(ping_delay);
    
    //right
    // lcd.locate(1,1);
    // lcd.printf("l:%i", ping(3));
    flash(righLED, 3);

    // scanLED=false;
}
//------------------------------------------------------MOVEMENT FUNCTIONS------------------------------------------------------//

/* crash
    if the player tries to move into a wall, this function will be called and remove a life
    If the player runs out of lives the game is over.
*/
void crash(){
    lives-=1;
    
    lcd.cls();
    lcd.locate(0,0);
    lcd.printf("CRASHED!!!");
    lcd.locate(0,1);

    if (lives>0) lcd.printf("Ship Damaged.");
    else {
        //game over
        lcd.printf("Ship Destroyed.");
        while(true);
    }
}

/* go_fowards()
    a function to detect if theres something in the next space fowards. 
    If there is, call crash(), otherwise, move the ship into the space.
*/
void go_fowards(){
    //if not a wall in front of me
    //position[0]+-1 OR position[1] +- 1 depending on direction
    // printf("position (%i,%i)",position[0], position[1]);
    if(orientation==0){ //if moving east
        if(map[position[1]][position[0]+1]==1){ //if next space is a wall 
            crash();
        }
        else{
            position[0]++;
            // lcd.cls();               debugging lines display distances to walls
            // lcd.locate(0,0);
            // lcd.printf("fowards.");
        }
    }
    if(orientation==1){ //if moving south
        if(map[position[1]+1][position[0]]==1){ //if next space is a wall 
            crash();
        }
        else{
            position[1]++;
            // lcd.cls();
            // lcd.locate(0,0);
            // lcd.printf("fowards.");
        }
    }
    if(orientation==2){ //if moving west
        if(map[position[1]][position[0]-1]==1){ //if next space is a wall 
            crash();
        }
        else{ 
            position[0]--;
            // lcd.cls();
            // lcd.locate(0,0);
            // lcd.printf("fowards.");
        }
    }
    if(orientation==3){ //if moving north
        if(map[position[1]-1][position[0]]==1){ //if next space is a wall 
            crash();
        }
        else{
            position[1]--;
            // lcd.cls();
            // lcd.locate(0,0);
            // lcd.printf("fowards.");
        }
    }
    lcd.cls();
}

/* rotate
    rotate the ship by an angle where, for example, -1 would be 90* left, 1 would be 90* right, and 2 would be 180* right
*/
void rotate(short angle){
    short newOrien = orientation + angle;
    while(newOrien>3) newOrien-=4;
    while(newOrien<0) newOrien+=4;  //normalise the orientation between 0 and 3
    
    orientation = newOrien;
}
//-------------------------------------------------------------MAIN-------------------------------------------------------------//

int main(){
    lcd.cls(); //clears the lcd screen
    lcd.locate(0,0); //sets the cursor to column 0 and row 1
    // lcd.printf("Hello World!\n"); //displays the message Hello World on the LCD

    while(true){
        if(pingButton == false){    //on press
            lcd.cls();
            lcd.locate(0,0);
            lcd.printf("  sonar active  ");
            ping_all();
            lcd.cls();
        }
        if(moveButton == false){ 
            lcd.cls();
            lcd.locate(0,0);
            lcd.printf("    moving ^    ");
            thread_sleep_for(ping_delay);
            go_fowards();
        }
        if(rotateButton == false){  //rotate 90* right on press
            lcd.cls();
            lcd.locate(0,0);
            lcd.printf("   rotated ->   ");
            thread_sleep_for(ping_delay);
            rotate(1);
            lcd.cls();
        }
    }
}