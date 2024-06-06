# FractalErode
A customisable, interactive and realtime simulation of hydraulic and thermal erosion acting over 3D fractal terrains. Rendered in OpenGL with both C++ and OpenGL compute shader backends for the erosion simulation. imGUI provides users with a clean and simple interface.

Terrain is generated using various noise layering techniques, with Ken Perlin's 2002 improved noise algorithm acting as the base function. The erosion is an extended implementation of the original eulerian hydrualic and thermal erosion algorithms introduced by Musgrave et al. in 1989.

## Showcase
![fractal_terrain_water](https://github.com/James-Blackburn/FractalErode/assets/32494995/6d518486-bec9-400f-afcb-b3bad5a4607e)
|:--:| 
| *An eroded fractal landscape generated with 5000 steps of hydraulic and thermal erosion* |

![Screenshot 2024-06-06 144139](https://github.com/James-Blackburn/FractalErode/assets/32494995/04762b87-af3d-4d0b-b651-e73f507f6a5b)
|:--:| 
| *Renders of the different stages of the erosion process, the flow of sediment can be seen clearly in (b)* |

![Screenshot 2024-06-06 145607](https://github.com/James-Blackburn/FractalErode/assets/32494995/4bbcd289-3699-4389-bc29-1fd53cf44317)
|:--:| 
| *A jagged ravine formed via hydraulic erosion vs a smooth-walled ravine formed via a combination of hydraulic and thermal erosion.* |

## An Interactive Simulation
<img src="https://github.com/James-Blackburn/FractalErode/assets/32494995/640fd478-5d18-4651-8ea0-3a767546438e" with="400" height="400">\
Erosion process acting over a small patch of terrain.
