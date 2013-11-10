PA10: Lighting
==============

Group Members
-------------
> 
> Eric Bryant
>  
> Bandith Phommounivong
>  
> Michael Mills
> 

How to run this project
-----------------------
> To run this project, use this command line argument: 
>   
> * ./AirHockey
>  

How to control the lights
-------------------------
> To move the lights forward/backward (along the z-axis): t/y
>  
> To move the lights left/right (along the x-axis): g/h
>  
> To move the lights up/down (along the y-axis): b/n
>  
> To turn ambient light off: 1
>  
> To turn ambient light on:  2
>  
> To turn distant light on:  3 (point and spot lights off)
>  
> To turn point light on:    4 (distant and spot lights off)
>  
> To turn spot light on:     5 (distant and point lights off)
>  
> To turn off specular and diffuse reflections off: 6
>  
> To turn back specular and diffuse reflections on: 7
  
  To add 1 to x compenant of spot light direction: 8

  To sub 1 to x compenant of spot light direction: 9

  To add 1 to y compenant of spot light direction: 0

  To sub 1 to y compenant of spot light direction: -

  To add 1 to z compenant of spot light direction: +

  To sub 1 to z compenant of spot light direction: =

Extra Credit Implemented
------------------------
> Light position can be moved using the controls outlined above (using t/y, g/h, and b/n)
> Spot light direction can be changed using the controls outlined above (using 8/9, 0/-, and +/=)   

What the lights are initially set to
------------------------------------
> The initial startup has distant light turned on, ambient light turned on

Recommended test run
--------------------
> In order to best see the effects of the individual light types (and to not get
> lost with what is turned on and off), it is recommended
> to run this program in the following order:
>  
> * Press 1 (to turn off the ambient light, now only distant light is on)
> * Press 4 (to see the point light in action, you can use the b and n keys to
> move it closer to/further from the table)
> * Press the 5 key (to turn the spotlight on)
> * Press the 6 key (everything should now be black)
> * Press the 2 key (to turn ambient light back on)
> * Press the 7 key (to turn the spotlight back on)
>  
> So the recommended key press sequence is 1, 4, 5, 6, 2, 7.  Enjoy!  

How to control the paddles
--------------------------
> To start/pause: f or F or select menu option through right click
> To quit: ESC or q or Q or select from menu
> To restart the game: R or select restart from the menu
> To reset the camera: p or P
>  
> Camera controls: J/L to rotate left/right
>                  I/K to change pitch
>                  U/O to zoom in/out
>  

Technical issues
----------------
>  
>  This project took quite a while to get a general understanding of how lighting works.
>  However, once we figured this out things weren't too difficult, save the spotlight.
>  Getting the spotlight to display in a spherical manner took quite a bit of effort.  
>  
>  The attenuation for our spotlight/distant light is still not perfect, as it is a bit more
>  dim than we would like to to see.  We might be able to curb this bahavior by adjusting ambient
>  light (by turning total ambient light down, we should be able to increase the appeared intensity
>  of our spotlight).  
>  
>  That said, we are quite pleased with how our lighting turned out.
>  

Where to put textures
---------------------
> Our texture loader expects all texture images to found in the relative
> directory assets/models/textures/
> 
> If your texture is not found in this directory, you will be warned and no
> texture will be loaded.
> 
> As far as .mtl files, they should be found in the same directory as the
> model you provide as a command line argument.
> 


Assimp Version
--------------
> * Version: 3.0 
> * Architecture: i386

Devil Version
-------------
> * Version: 1.7.8
> * Architecture: i386

Building This Example
---------------------

*This example requires GLM, devIL and assimp*

The excutable will be put in bin
