Things i have done
 - Started code idea
 - Wrote map as 2D araay (could be 3D in future)
 - Made a sonar system to detect how far away the walls are
 - Made a ping system to use the sonar in four directions and ping with a delay proportional to the distance
 -Made a template for the initial report
 -Made a Google drive to store items
 made a git hub repository to keep code visible


 18/2/26:
 - Testing
    - LCD working and made to display debugging info (distances to nearest wall)
    - LEDs are working as intended
    - Sonar function for back and right returning -1 indicating no wall, which is wrong
        - These functions use of minuses in their for loops instead of pluses, likely something to do with that
    -pining happens automatically, not sure why?
    - Using chars (to save memory space) caused overflow errors meaning large numbers (like 255) were in places where there should be nagatives, these have been changed to shorts

 - Debugging
    - Noticed the LEDs display in the wrong order, a problem of assigning wires to DigitalIns wrong
    - Realised that position[] was in form {y,x} which isnt very intuitive, so swapped to {x,y}
        - This means position[0] (x) gets checked against map[const][var]
        - and position[1] (y) gets checked against map[var][const]
    - I fixed the problem with the button by specifying it as a pulldown and making the ping condition false

    - fixing the 'no wall' problem:
        - After some testing and monitoring variables, I noticed two things:
        - 1. The west ping if-statement (as well as the north) found the max_dist=position[x or y]-1.
            - This was a mistake since, when counting from 0, the distance to pos 0 of an array is the value of your current position in it
            - Therefore changing it to max_dist=position[x or y] fixed it.
        - 2. The north ping condition had a typo where it looked for direction 1 again instead of 3. Fixing this fixed the left LED returning -1
