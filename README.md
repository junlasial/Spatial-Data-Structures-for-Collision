# Bounding Volumes (BV), BV Hierachies and Spatial Partitioning

This application allows you to test out different bounding volumes, splits for BV Hierchies and different types of spatial partitioning techniques.
This is a study on Spatial Data Structures in 3D Graphics. The content is an application of the concepts taught in <i>[Real-Time Collision Detection by Christer Ericson](https://www.amazon.com/Real-Time-Collision-Detection-Interactive-Technology/dp/1558607323)</i> textbook, specifically on the application of the common bounding volumes, bounding volume hierarchies and spatial partitioning techniques (OctTrees, BSPTrees). This was taught in CS350 module during my overseas exchange at Digipen, Redmond, USA.

Bounding Volume Hierarchies and Spatial Partitioning are 2 methods aimed at improving efficiency when dealing with large sets of objects or scenes. Bounding volume hierarchies organize objects hierarchically using bounding volumes, such as axis-aligned bounding boxes (AABBs) or spheres. Instead of dividing the space like Spatial Partitioning, BVHs build a tree where each node represents a bounding volume containing a subset of objects. Whether to use spatial partitioning or BV Hierarchies depend on different cases. Sometimes in scenarios where object distribution is uniform, object sizes vary significantly, or where complex spatial relationships need to be captured efficiently, spatial partitioning methods may offer better performance and memory utilization.

❗ Spatial Partitioning is <b>slow</b>, I did not include a loading screen or any optimisation so it might freeze a bit when enabling it.

## Screenshots

### Bounding Volumes
You may try out different types of BVs such as different spheres with varying levels of fit.
![Bounding Volumes Screenshot](https://github.com/matthias-ong/Spatial-Partitioning-w-OctTrees-BSPTrees/blob/main/screenshots/bounding_volumes.png)

### Bounding Volume Hierachies
![Bounding Volume Hierachies Screenshot](https://github.com/matthias-ong/Spatial-Partitioning-w-OctTrees-BSPTrees/blob/main/screenshots/bv_hierarchies.png)

### Spatial Partitioning (OctTrees, BSPTrees)
❗ Note: It might freeze a bit when enabling this feature in the application.
#### OctTree Level 2
![OctTree Level 2 Screenshot](https://github.com/matthias-ong/Spatial-Partitioning-w-OctTrees-BSPTrees/blob/main/screenshots/octtree_level2.png)
#### OctTree Level 1 (observe the splits)
![OctTree Level 1 Screenshot](https://github.com/matthias-ong/Spatial-Partitioning-w-OctTrees-BSPTrees/blob/main/screenshots/octtree_level1.png)

 
## Setup
I simply included all the necessary dependencies, it should run after pulling. I don't plan on maintaining this project so best practices isn't my focus.
Tested on: Windows 10, Intel Iris Plus Graphics, OpenGL 4.6, OpenGL Driver Intel Graphics Driver 30.0.101.1660, Visual Studio 2022
Refer to `README.txt` for more information.
