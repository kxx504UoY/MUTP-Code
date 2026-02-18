#include "mbed.h"
#include "TextLCD.h"

//------------------------------------------------VARIABLES------------------------------------------------//

TextLCD lcd(D0, D1, D2, D3, D4, D5, TextLCD::LCD20x4); // Connect these nucleo pins to RS, E, D4, D5, D6 and D7 pins of the LCD
DigitalOut fowLED(D6), leftLED(D7), righLED(D8), backLED(D9);
DigitalIn pingButton(D10);
//using shorts for memory efficiency
const short MAP_SIZE = 10; //currently one variable for size - a square map
const short map[MAP_SIZE][MAP_SIZE]={{1,1,1,1,1,1,1,1,1,1},
                                    {1,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,1},
                                    {1,1,1,1,1,1,1,1,1,1}}; //0-empty, 1-wall
short orientation = 0; //0-E, 1-S, 2-W, 3-N
short sonar_orien = 0;
short position[] = {2,2}; //easier to have position as its own variable instead of storing in map
short ping_delay=500;
short dist_delay=200;

//------------------------------------------------FUNCTIONS------------------------------------------------//

/* ping_fow
    Send out a ping and light up LEDs if there is a wall detected in foward direction

    returns the distance of nearest wall (-1 if there is none)
*/
short ping(short sonar_orien){
    
    short wall_dist = 1; //initialising how far away the closest wall is (1=right next to player)
    
    short direction = sonar_orien+orientation;
    if (direction>3) direction-=4;  //coincidentally adding up the two always gives the number for direction

    if(direction == 0){ //if east
        short max_dist = MAP_SIZE - position[1]; //the distance to scan through

        for(int i=1; i<=max_dist; i++){
            if (map[position[0]][position[1]+i] == 1){
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
        short max_dist = MAP_SIZE - position[0];

        for(int i=1; i<=max_dist; i++){
            if (map[position[0]+i][position[1]] == 1){
                //there is a wall, return its distance
                return wall_dist;
            }
            else wall_dist+=1;
        }
    }
    else if(direction == 2) //if west
    {
        short max_dist = position[1]-1;

        for(int i=1; i<=max_dist; i++){
            if (map[position[0]][position[1]-i] == 1){
                //there is a wall, return its distance
                return wall_dist;
            }
            else wall_dist+=1;
    }
    }
    else if(direction == 1) //if north
    {
        short max_dist = position[0] - 1;

        for(int i=1; i<=max_dist; i++){
            if (map[position[0]-i][position[1]] == 1){
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
    if (DelayCount>0) thread_sleep_for(dist_delay * DelayCount);
    else printf("NO WALL");

    LED = true;
    thread_sleep_for(dist_delay);     //ping LED again
    LED= false;
}

/* ping_all
    pings in every direction clockwise starting from foward
    (at the moment it will only print hqow many spaces away the nearest wall is in each direction and doesn't return anything)
*/
void ping_all(){
    //fow
    lcd.locate(0,0);
    lcd.printf("f:%i",  ping(0));
    flash(fowLED, 0);

    thread_sleep_for(ping_delay);

    //left
    lcd.locate(8,0);
    lcd.printf("l:%i", ping(1));
    flash(leftLED, 1);
    
    thread_sleep_for(ping_delay);
    
    //back
    lcd.locate(0,1);
    lcd.printf("b:%i", ping(2));
    flash(backLED, 2);
    
    thread_sleep_for(ping_delay);
    
    //right
    lcd.locate(8,1);
    lcd.printf("r: %i", ping(3));
    flash(righLED, 3);

}




int main() 
{
    lcd.cls(); //clears the lcd screen
    lcd.locate(0,0); //sets the cursor to column 0 and row 1
    // lcd.printf("Hello World!\n"); //displays the message Hello World on the LCD

    while(true){
        if(pingButton == true){
            ping_all();
        }

    }
}
























    
    // lcd.locate(0,1); //set cursor on row number 1 (bottom row)
    // lcd.printf("The value is %d", 25); //displays an integer value
