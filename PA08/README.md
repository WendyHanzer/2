PA07: Texture Loading
=====================

Group Members
-------------
> 
> Eric Bryant
> Bandith
> Michael
> 

A note on makefiles
------------------
> Apparently I don't know makefiles very well, the object will compile on my 
> machine with the provided makefile, and hopefully it will on yours as well.
> But for some reason, you have to save a change to main every time in order
> to recompile.  For example, editing model.h, saving it, and trying to
> recompile yields a message that nothing is to be done for make (because it
> thinks everything is up to date...not sure why it doesn't realize model.h
> has in fact changed)

What this project does
----------------------

> This project implements a simple texture loader for mesh objects.
> It is capable of loading a multi-mesh object, and apply textures to each
> of the meshes.  If no texture image can be found for the mesh, a default
> texture is loaded.
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


The Good, Bad, and Ugly
-----------------------
> Simply put, this project was a bitch.  Not fully understanding textures
> made implementation and debugging quite difficult.  But it is mostly up and 
> working.  It should be able to load a model with multiple meshes and multiple
> textures and properly apply those.  
> 
> We have, however, encountered issues with drawing the first mesh in a scene
> of multiple meshes.  The object appears to be drawn in the code (it's draw
> draw function is called and ran, but nothing appears (this is only for some
> models).  
> 
> On certain installs of devil/assimp, the texture loader will segfault for
> some models, at times particularly models with no textures.  Currently
> working to resolve this.

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
