//-----------------------------------------------------------INCLUDES----------------------------------------------------------//
    #include "mbed.h"
    #include "TextLCD.h"

//------------------------------------------------------IO INITIALISATION------------------------------------------------------//
    // Connect these nucleo pins to RS, E, D4, D5, D6 and D7 pins of the LCD
    TextLCD lcd(D0, D1, D2, D3, D4, D5, TextLCD::LCD20x4);
    DigitalOut fowLED(D6), leftLED(D8), righLED(D7), backLED(D9); //scanLED(D12);
    DigitalIn pingButton(D10, PullUp);
    DigitalIn moveButton(D11, PullUp);
    DigitalIn rotateButton(D12, PullUp);
    
    AnalogIn rndPin(PA_0);    // A0 to measure noise
    // AnalogOut A_1(PA_1), A_2(PA_4), A_3(PB_1);       //NEED TO FIX FUNC!!!!
    DigitalIn  RotEncCLK(PB_1), RotEncDT(PB_12);        // for scanning rotary encoder
    // DigitalIn pauseSwitch(PD_9);    //bottom right pin to switch
//----------------------------------------------------------VARIABLES----------------------------------------------------------//
    //using shorts for memory efficiency
    const short MAP_X = 20; //How wide the map is
    const short MAP_Y = 10; //How long the map is
    short map[MAP_Y][MAP_X]=       {{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                                    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}}; //0-empty, 1-wall, 2-treasure, 3-portal (to minigame)
    short orientation = 0; //0-E, 1-S, 2-W, 3-N (Clockwise from east)
    short position[] = {1,1}; //position on map in form {x,y}
    short start_pos[] = {0,0};
    short relative_pos[] = {0,0}; //position relative to start

    short ping_delay=500;   //the delay between pinging each direction
    short dist_delay=200;   //the delay for every empty space between you and a wall

    short lives=2;  //how many lives player has

    short numProgLEDs = 4;
    const short TOTAL_TREASURE = numProgLEDs*2; //current implementation 8 treasures
    short FoundTreasure = 0;

//---------------------------------------------------------DECLARATIONS--------------------------------------------------------//
    short ping(short sonar_orien);
    void flash(DigitalOut LED, short dir);
    void ping_all();

    void crash();
    void go_fowards();
    void rotate(short angle);
    bool encoderRotated();
    bool rotatedCW();

    unsigned short random_generator();
    void rnd_position();
    bool isTreasure();
    bool isPortal();
    void teleport();

    void progOSD();
    void moveOSD();
    void rotateOSD(bool CW);
    void bordersOSD();
    bool isTreasure();
    void refreshOSD();
    // void updateProgLEDs();  

    void win();
    // bool paused();

//-------------------------------------------------------------MAIN------------------------------------------------------------//

    int main(){
        // startMenu();

        rnd_position();
        refreshOSD();
        //top-level loop
        while(true){
            // lcd.cls();
            // lcd.printf("Get looped idiot");
            if(pingButton == false){    //on press
                ping_all();

            }
            else if(moveButton == false){ 
                thread_sleep_for(ping_delay);
                go_fowards();
            }
            else if(encoderRotated()){  //rotate 90deg right on press (temp - will be replaced with encoder)
                // rotate(1);
                if(rotatedCW()) rotate(1);
                else            rotate(-1);

                thread_sleep_for(ping_delay);
            }
            // else if(paused()){
            //     lcd.cls();
            //     lcd.locate(0,0);
            //     lcd.printf("Paused");
            //     while(paused()) /*do nothing*/;
            //     lcd.cls();
            //     thread_sleep_for(ping_delay);

            // }
        }
    }

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
            max_dist = MAP_X - position[1]; //the distance to scan through

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
            max_dist = MAP_Y - position[1];

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
            thread_sleep_for(ping_delay);
        }

        
    }

    /* ping_all
        pings in every direction clockwise starting from foward
        (at the moment it will only print how many spaces away the nearest wall is in each direction and doesn't return anything)
    */
    void ping_all(){
        //indicate the sonar has started
        //fow
        flash(fowLED, 0);
        thread_sleep_for(ping_delay);

        //left
        flash(leftLED, 1);
        thread_sleep_for(ping_delay);
        
        //back
        flash(backLED, 2);
        thread_sleep_for(ping_delay);
        
        //right
        flash(righLED, 3);
    }

//------------------------------------------------------MOVEMENT FUNCTIONS-----------------------------------------------------//
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
        thread_sleep_for(ping_delay);
        refreshOSD();
    }

    /* go_fowards()
        a function to detect if theres something in the next space fowards. 
        If there is, call crash(), otherwise, move the ship into the space.
    */
    void go_fowards(){
        //if not a wall in front of me
        //position[0]+-1 OR position[1] +- 1 depending on direction
        if(orientation==0){ //if moving east
            if(map[position[1]][position[0]+1]==1){ //if next space is a wall 
                crash();
            }
            else{
                position[0]++;
                moveOSD();    //update movement info
            }
        }
        else if(orientation==1){ //if moving south
            if(map[position[1]+1][position[0]]==1){ //if next space is a wall 
                crash();
            }
            else{
                position[1]++;
                moveOSD();    //update movement info
            }
        }
        else if(orientation==2){ //if moving west
            if(map[position[1]][position[0]-1]==1){ //if next space is a wall 
                crash();
            }
            else{ 
                position[0]--;
                moveOSD();    //update movement info
            }
        }
        else if(orientation==3){ //if moving north
            if(map[position[1]-1][position[0]]==1){ //if next space is a wall 
                crash();
            }
            else{
                position[1]--;
                moveOSD();    //update movement info
            }
        }
        
        if(isTreasure()){
            progOSD();    //update progress info
            map[position[1]][position[0]] = 0;  //remove treasure one collected
        }
        else if(isPortal()) teleport();

        
    }

    /* rotate
        rotate the ship by an angle where, for example, -1 would be 90* left, 1 would be 90* right, and 2 would be 180* right
    */
    void rotate(short angle){
        short newOrien = orientation + angle;
        while(newOrien>3) newOrien-=4;
        while(newOrien<0) newOrien+=4;  //normalise the orientation between 0 and 3
        
        orientation = newOrien;

        if      (angle>0) rotateOSD(true);  //update rotation info
        else if (angle<0) rotateOSD(false); //true->CW, false->CCW
    }

    bool encoderRotated(){
        if (RotEncCLK==false) return true;
        else                return false;
    }

    bool rotatedCW(){
        if(RotEncDT==false) return true;
        else                return false;
    }

//--------------------------------------------------------MAP FUNCTIONS--------------------------------------------------------//
    /*random_generator
        generate a random number via noise on an alaog pin
            (credit to Rohan Kakade)
    */
    unsigned short random_generator(){
        unsigned short x = 0;
        unsigned short iRandom = 0;

        for (x = 0; x <= 32; x = x+ 2) {
            iRandom = iRandom + ((rndPin.read_u16() % 3) << x); // "<< x" rotates the value in yellow brackets by x bits to the left
            thread_sleep_for (10);
        }
        return iRandom; //return random value
    }

    /* rnd_position
        finds a random position on the map, if it is an empty space (0) then 
        it will set initial position to this random space (and reset the relative position)  
    */
    void rnd_position() {
        short posX, posY;

        do {
            posX = random_generator() % (MAP_X - 2) + 1;
            posY = random_generator() % (MAP_Y - 2) + 1;    //limit random number within map range
        } while (map[posY][posX] != 0);

        position[0] = posX;
        position[1] = posY;
        start_pos[0]= posX;
        start_pos[1]= posY;

        orientation = random_generator() % 4;
    }


    /* isTreasure
        Returns true if position on map is treasure (2)
        Returns false if not
    */
    bool isTreasure(){
        if (map[position[1]][position[0]] == 2){
            FoundTreasure+=1;
            // updateProgLEDs();
            return true;
        }
        else return false;
    }

    /* isPortal
        Checks if current space is a portal
    */
    bool isPortal(){
        if (map[position[1]][position[0]] == 4) return true;
        else return false;
    }

    /* teleport
        When player goes into a portal on the map, get teleported (load) a minigame
    */
    void teleport(){
        //open random minigame
    }

//-------------------------------------------------------------OSD-------------------------------------------------------------//

    //----------internal (called by general functions)----------//

    /* updateProgLEDs
        updates the progress LED outputs according to the percentage progress of the user
    */
    // void updateProgLEDs(){
    //     float toLight = (float(FoundTreasure)/TOTAL_TREASURE)*numProgLEDs;        //no. LEDs to Light
    //     if (toLight>=1) A_1=1;
    //     if (toLight>=2){
    //         A_2=1;
    //         A_3 = (numProgLEDs-toLight);    //e.g. if 2.5 out of 3 then A_3=0.5
    //     }
    //     // if (FoundTreasure==TOTAL_TREASURE) win();
        
    //                 //e.g. for 6 total treasure on 3 leds, having found
    //                 //5 you would see 2 fully lit and 1 half lit LEDs
    //                 //if you had all 6, i==toLight and you win
    // }

    //absolute compass (relative to map) 
    void updateCompOSD(){
        //clear compass on LCD
        lcd.locate(0,0);
        lcd.printf("  ");
        lcd.locate(0,1);        
        lcd.printf("  ");

        if(orientation==0)     {lcd.locate(1,0);
                                lcd.printf("r");}

        else if(orientation==1){lcd.locate(1,1);
                                lcd.printf("d");}

        else if(orientation==2){lcd.locate(0,1);
                                lcd.printf("l");}
                                
        else if(orientation==3){lcd.locate(0,0);
                                lcd.printf("u");}
    }

    //pos relative to start
    //if space change detected (relative pos starts at (0,0))
    void updatePosOSD(){
        relative_pos[0] = position[0]-start_pos[0];
        relative_pos[1] = position[1]-start_pos[1];
        lcd.locate(3,0);
        lcd.printf("(%i,%i)",relative_pos[0],relative_pos[1]);
    }


    //----------general functions (call these)----------//

    //Movement feedback (calls updatePosOSD)
    void moveOSD(){
        //telemetry animation
        lcd.locate(15,1);   //bottom right arrow
        lcd.printf("u");
        thread_sleep_for(200);
        lcd.locate(15,1);
        lcd.printf(" ");
        lcd.locate(15,0);
        lcd.printf("u");    //arrow shift up
        thread_sleep_for(200);
        lcd.locate(15,0);
        lcd.printf(" ");
        lcd.locate(15,1);
        lcd.printf("u");    //arrow shifts back down
        thread_sleep_for(200);
        lcd.locate(15,1);
        lcd.printf(" ");    //arrow disappears
        updatePosOSD();
    }

    //rotation feedback (calls updateCompTelem)
    void rotateOSD(bool CW){ //where dir is an arrow char (← or →)
        lcd.locate(15,1);
        lcd.printf("u");
        thread_sleep_for(200);
        lcd.locate(15,1);
        if(CW) lcd.printf("r");
        else   lcd.printf("l");

        thread_sleep_for(200);
        lcd.locate(15,1);
        lcd.printf("u");
        thread_sleep_for(200);
        lcd.locate(15,1);
        lcd.printf(" ");
        updateCompOSD();
    }

    //progress - displayed as a fraction out of total (calls updateProgLEDs)
    void progOSD(){
        lcd.locate(3,1);    //bottom middle
        lcd.printf("%i/%i", FoundTreasure, TOTAL_TREASURE);
        // updateProgLEDs();
        if(FoundTreasure==TOTAL_TREASURE) win();
    }

    void refreshOSD(){
        lcd.cls();
        bordersOSD();
        progOSD();
        updateCompOSD();
        updatePosOSD();
    }

    /* bordersOSD
        print the on screen seperating borders that make the information on screen more formatted and neat
    */
    void bordersOSD(){
        lcd.locate(2,0);
        lcd.printf("|");
        lcd.locate(2,1);
        lcd.printf("|");
        lcd.locate(14,0);
        lcd.printf("|");
        lcd.locate(14,1);
        lcd.printf("|");
    }

//------------------------------------------------------------PAGES------------------------------------------------------------//
    // bool paused(){
    //     if(pauseSwitch) return true;
    //     else return false;
    // }

    void winScreen(){
        lcd.cls();
        lcd.locate(0,0);
        lcd.printf("CONGRATULATIONS!");
        lcd.locate(0,1);
        lcd.printf("Found all gold!");
        while(true);
    }

    // void pauseScreen(){
    //     lcd.cls();
    //     lcd.locate(0,0);
    //     lcd.printf("    PAUSED...   "); 
    //     lcd.locate(0,1);
    //     lcd.printf("play: push lever");
    // }

    // void startScreen(){
    //     lcd.locate(0,0);
    //     lcd.printf("");
    // }