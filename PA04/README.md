@Author Bandith Phommounivong
This is my PA04. 
***Required Features***
-Attached is the OBJ file that website saids to have. I understand you said not to attach it but I don't know which instructions to follow. 
-My loader will load this file and any triangluated .obj file that is a cube. 
***********************

***Other Features***
-This project is a modified PA02 so it will retain function from that project. 
-I have used the tutorial as basis for this loader. The code will be very similar in places. However, I have modified the code to use arrays instead of Vectors in most cases.
-Will handle files with normals, textures, vertices, or any combinations of those but will only process vertices and discard other values.
-Colors are calculated. Negative coordinates will get a 0 while positive coordinates will get a 1. For example, (-1, 1, -1) vertex will have the color (0, 1, 0). 
********************

***Notes***
See Bugs Section.
***********

***Bugs***
-Does not work with quad. Feature was not implemented.
-Will only load cubes such as air hockey tables and boxes. 
-Loader will fail on these types: cones, suzanne the monkey from blender, spheres, and dragon. 
-I do not know why this is. I have printed out geometry array and all vertices are correctly placed based on reference for simple .obj such as any cube object. However, complex objects such spheres and monkey heads have too many vertices to trace every reference. My problem may be here. 
-The other possible reason that my model loader is not working correctly could be because I am not allocating enough space in my glBufferData. I calculate this size similarly to the tutorial, numOfVert * sizeof(Vertex). This works for simple objects. However, for complex objects I may not be allocating enough space. I doubt this my problem.
**********
