# MUTP-Code

17/2/26 and before:
 - Started code idea
 - Wrote map as 2D araay (could be 3D in future)
 - Made a sonar system to detect how far away the walls are
 - Made a ping system to use the sonar in four directions and ping with a delay proportional to the distance
 - Made a template for the initial report
 - Made a Google drive to store items
 made a git hub repository to keep code visible


 18/2/26:
 - Testing
    - LCD working and made to display debugging info (distances to nearest wall)
    - LEDs are working as intended
    - Sonar function for back and right returning -1 indicating no wall, which is wrong
        - These functions use of minuses in their for loops instead of pluses, likely something to do with that
    - pining happens automatically, not sure why?
    - Using chars (to save memory space) caused overflow errors meaning large numbers (like 255) were in places where there should be nagatives, these have been changed to shorts
