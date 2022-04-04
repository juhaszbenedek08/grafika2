# grafika2 - Orbifold visualization

## Specification

Create a program that displays a dodecahedral room inscribed in a sphere of radius √3 m. <br />
Use ray-tracing on a GPU.

In the center, the room contains an optically smooth golden object defined by an implicit equation f(x,y,z)=exp(ax^2+by^2-cz)-1 cut into a sphere of radius 0.3 m. <br />
Refractive index and extinction coefficient of gold: n/k: 0.17/3.1, 0.35/2.7, 1.5/1.9. <br />
a,b,c are positive non-integer numbers.

The walls of the room from the corner to 0.1 m are of diffuse-specular type. <br />
The other part of the wall are portals opening onto another similar room, but rotated 72 degrees about the center of the wall and mirrored onto the plane of the wall.

The room also contains a point source of light. <br />
The light source does not shine through the portal. <br />
Each room has its own light source. <br />
For the display, it is sufficient to step through the portals a maximum of 5 times.

The virtual camera faces the centre of the room and rotates around it.

Other parameters may be chosen individually, so as to make the image aesthetic.

# Result

![Orbifold](https://user-images.githubusercontent.com/59647190/161530215-7c384cb2-1163-492e-ac49-e37694ab8af8.png)
