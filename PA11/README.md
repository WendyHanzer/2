PA11: Labyrinth
===============

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

What this project is
--------------------
> This is a rough draft for our Labyrinth project, and quite a rough
> draft at that.  We have our board completed (and looking good), and
> also have lighting correctly implemented. We also have controls to 
> rotate the board (via the arrow keys), and some testing tools so you
> can manually move the ball with the w, a, s, and d keys.

Technical issues
----------------
> We have run into some sligh issues thus far.  We were able to loada  mesh
> collision object that matches our board.  However, when loaded, the collision
> mesh does not align with the drawn mesh.  We were able to manually adjust the
> location to get them to line up, but when we attempted to apply rotation to 
> the collision mesh, the alignment with our drawn mesh was off again.
>  
> We think this is likely due to the center of the drawn model being different 
> than the center of the loaded collision mesh, causing the two to rotate about
> a different point.
>  
> We tried using GlutPostRedisplay and it seemed to get the mesh to line up closer
> to where it need to be, but this also caused the ball to stop moving.
>  
> We are currently working to resolve these issues, but once they have been solved
> it should be relatively easy to get the rest of the game up and properly running.

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
