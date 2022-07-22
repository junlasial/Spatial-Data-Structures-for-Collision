a. How to use parts of your user interface that is NOT specified in the assignment 
description:

Camera controls:
WASD to move around
Hold down Left Mouse Button and move mouse to look around
Mouse scroll to zoom
Hitting the ESC key will terminate the program.

After moving primitive objects in the world or making changes to the tree, remember to 
click on the "Update" button provided.

b. Any assumption that you make on how to use the application that, if violated, might 
cause the application to fail:

- While updating the trees, allow the application to load as it will take quite a while to construct the trees, BSP would take longer.
You can speed up the process by running in "Release" or increasing the Min Polygon Count.
- There should be enough memory required to run the application as it is quite memory intensive to create the trees.
- Moving the objects around at runtime and updating the tree will not work as I did not update the positions at runtime.

c. Which part of the assignment has been completed?

Everything that was mentioned in the handout except for Extra Credits and Serialization
1) Scene creation (8 objects)
2) Octree
I implemented it at a polygon level, you can see the code SpatialPartitioning.cpp
3) BSP Tree
Top down creation.
NOTE: I reduced the number of iterations in the looping through polygons to pick the best split plane as it was
too slow. This is seen in line 192 of SpatialPartitioning.cpp. This may result in some unoptimal split planes but
I had to speed up for my debugging purpose. If you want optimal split planes and have the time, feel free to iterate
every polygon.
4) Interactivity / Display 
- IMGUI Controls
- Random colours chosen for different nodes
NOTE: As I used randomness, sometimes the colours are similar to one another, please click "Update Tree" or rerun
the application to get better colours that are easier to see.

Extra credit:
NIL

d. Which part of the assignment has NOT been completed (not done, not working, 
etc.) and explanation on why those parts are not completed?
Serialisation, lack of time and understanding as to what to serialise.

e. Where the relevant source codes (both C++ and shaders, as applicable) for the 
assignment are located. Specify the file path (folder name), file name, function 
name (or line number). 

All source file are located in "1.0 - Simple Scene" folder
1. IMGUI Display for Trees is done in SimpleScene_Quad.cpp inside SimpleScene_Quad::Render().
2. Bulk of Rendering is done inside SimpleScene_Quad::Render().
3. Model loading using assimp is done in Model.cpp. This is where I load the BSPPolygon Model.
4. OctTrees and BSPTrees are all done in SpatialPartioning.cpp/h

f. Which machine did you test your application on. 
Tested on: Windows 10, Intel Iris Plus Graphics, OpenGL 4.6, OpenGL Driver Intel Graphics Driver 30.0.101.1660

g. The number of hours you spent on the assignment, on a weekly basis 
20 hours per week over 2.5+ weeks.

h. Any other useful information pertaining to the application
IF it gets to laggy and slow to build the trees, run in RELEASE mode or increase the minimum polygon count.
IF the colours are not very distinct from one another, please click "Update Tree" or Restart the program as
I am using Randomness to generate my colours.

Is it possible to give feedback on how to optimise my code and a brief overview on what data to serialise.
