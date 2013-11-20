PA08: BULLET PHYSICS
====================

Group Members
-------------
> 
> Eric Bryant
>  
> Bandith
>  
> Michael
> 

How to run this project
-----------------------
> To run this project, use this command line argument order: 
>   
> * ./Matrix assets/models/texturedSphere.obj assets/models/hockeytable.obj assets/models/texturedCube.obj assets/models/texturedCylinder.obj
>  
> If you do not use this order, the models will be mixed up for the physics engine,
> and unexpected/undesired results will ensue.

How to control the objects
--------------------------
> * The cube and ground are static objects, they don't move but our objects can bounce off of them
> * To start the simulation: Press F
> * To move the sphere: use the up/down and left/right arrows on the keyboard
> * To move the cylinder: use the w/s and a/d keys to navigate the world
> * Be careful not to fall off!

What this project does
----------------------

> This project implements physics (and a bit somewhat poorly at that).
> It loads 4 models: our base board, a static cube, and two dynamic
> objects, a sphere and a cylinder.  All the objects can interact with
> eachother.
>  
> The sphere has a smaller coefficient of restitution than the cylinder,
> so it stops bouncing quicker than the cylinder.  Sometimes the physics
> does wacky things and the sphere, cylinder, or both get stuck to the
> plane.  I think this may be due to improperly resolving inner penetration
> but can't be sure.
> 
>  
> A brief overview of the classes involved:
> * Scene: contains an array of the meshes found in scene object, loads
>   loads the meshes and calls their initialization function.
> * Model: contains all the information needed by a mesh to load it's texture,
>   initialize it's geometry, and draw itself.
> * main: still keeps track of the actual movement of the objects throughout
>   our world, but now simply has to call init and draw to draw a model in
>   the world.
>  
> Updated the structure of the project to implement a model/mesh object
> which handles the shader and geometry for that object. 

The Good, Bad, and Ugly
-----------------------
> The hardest part of this project was simply installing bullet.  This has been
> a common theme for this class.  While the projects themselves are rather difficult,
> I find myself every week battling for multiple hours simply trying to install the
> latest libraries we need to use to make things work, and trying to properly include
> those files in the make file so the libraries are actually found and linked.
>  
> Beyond the infinite frustrations with installing Bullet, the project itself was not
> overwhelming, although time was not on our side and hence the project certainly has
> some loose ends to tie up.  However, we are pleased to have gotten some basic physics
> up and running before the deadline.
> 

A note on makefiles
------------------
> Apparently I don't know makefiles very well, the object will compile on my 
> machine with the provided makefile, and hopefully it will on yours as well.
> But for some reason, you have to save a change to main every time in order
> to recompile.  For example, editing model.h, saving it, and trying to
> recompile yields a message that nothing is to be done for make (because it
> thinks everything is up to date...not sure why it doesn't realize model.h
> has in fact changed

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
