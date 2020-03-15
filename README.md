# Mandelbrot by Alex Dietrich 

I worked on this project during a lecture for my master degree in Software Engineering at the University of Applied Sciences Technikum Wien. This is my first project including SDL Engine.  

## Installation 

Clone the project on your local machine. 
Start the project with Visual Studio. 

Download the SDL 2.0 Development Libaries at this site:<br/>
https://www.libsdl.org/download-2.0.php

Follow those instructions to configute the additional libraries, the linker and and compiler: <br/>
https://www.wikihow.com/Set-Up-SDL-with-Visual-Studio

### The Project should now compile!

#### Hint: You should compile in the Release-Mode. Otherwise change the Release-Folder in the following instruction to the Debug-Folder

## Usage

Copy the SDL2.dll in the Release-Folder were the .exe is located. Just execute the .exe File of the Release-Folder. The Mandelbrot-Window should now appear. You are able to use the controls below to navigate the view of the picture and explore the complex plane.:satisfied: 
 
### The controls for navigation are:<br/> 
#### Press "w" &rarr; Picture moves upwards <br/>
#### Press "s" &rarr; Picture moves downwards <br/>
#### Press "a" &rarr; Picture moves on the left <br/>
#### Press "d" &rarr; Picutre moves on the right <br/>
#### Press "page up" &rarr; Picture will zoom in <br/>
#### Press "page down" &rarr; Picture will zoom out <br/>
#### Press "up" &rarr; The details of the calculation will get better <br/>
#### Press "down" &rarr; The details of the calculation will get worse <br/>

## Attention 
If you increase the detail of the calculation, the execution time will slow down! <br/> 

## Screenshot <br />
![Screenshot](screenshot.jpg?raw=true  "Application Screenshot")

### Used Engine to visualize the render the picture on the screen
https://www.libsdl.org/index.php &rarr; SDL 2.0 <br/><br/>

Copyright © 2020, Alexander Dietrich 