//-----------------------------------------------------------------
// Program: ray_trace.cpp
// Purpose: Program that demonstrates ray tracing using
//          multiple spheres as well as added realism with shadows. 
// Author:  John Gauch, Karshin Luong
// Date:    Spring 2019
//-----------------------------------------------------------------

#include <math.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <ctime>
#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Include ray tracing and phong shading code
#include "ray_classes.h"

//---------------------------------------
// Calculate random value between [min..max]
//---------------------------------------
float myrand(float min, float max)
{
   return rand() * (max - min) / RAND_MAX + min;
}

class Sphere
{
   public:
      Sphere3D sphere;
      Point3D center;
      ColorRGB color;

      Sphere(float x, float y, float z, float radius)
      {
         this->center.set(x, y, z);
         this->sphere.set(this->center, radius);
         this->color.set(myrand(0,255)+50,myrand(0,255)+50,myrand(0,255)+50);
      }
};

// Global variables
#define XDIM 900
#define YDIM 900
unsigned char image[YDIM][XDIM][3];
float position = 3;
string mode = "phong";
vector<Sphere> spheres;

//---------------------------------------
// Init function for OpenGL
//---------------------------------------
void ray_trace()
{
   // Define Phong shader
   Phong shader;
   
   // Define camera point
   Point3D camera;
   camera.set(0,0,-position);
   shader.SetCamera(camera);
   
   // Define light source
   ColorRGB color;
   color.set(255,255,255);
   Vector3D dir;
   dir.set(-1,-1,-1);
   dir.normalize();
   shader.SetLight(color, dir);
   
   // Set object properties
   color.set(200,0,100);
   shader.SetObject(color, 0.3, 0.4, 0.4, 10);
   
   // Perform ray tracing
   for (int y = 0; y < YDIM; y++)
   {
      for (int x = 0; x < XDIM; x++)
      {
         // Variables for ray tracing
         Point3D p;
         Vector3D n;
         float closestDistance = 999.99;
         int intersectSphere = 0;

         // Clear image
         image[y][x][0] = 0;
         image[y][x][1] = 0;
         image[y][x][2] = 0;
         
         // Define sample point on image plane
         float xpos = (x - XDIM/2) * 2.0 / XDIM;
         float ypos = (y - YDIM/2) * 2.0 / YDIM;
         Point3D point;
         point.set(xpos, ypos, 0);
         
         // Define ray from camera through image
         Ray3D ray;
         ray.set(camera, point);
         
         // For every sphere if there is an intersect, find the new closest distance and which sphere it was
         for (int i = 0; i < (int)spheres.size(); i++)
         {
            if(spheres.at(i).sphere.get_intersection(ray, p, n))
            {
               if(point.distance(p) < closestDistance)
               {
                  closestDistance = point.distance(p);
                  intersectSphere = i;
               }
            }
         }

         // Set object property at the intersecting sphere
         shader.SetObject(spheres.at(intersectSphere).color, 0.3, 0.4, 0.4, 10);

         // Perform sphere intersection
         if(spheres.at(intersectSphere).sphere.get_intersection(ray, p, n))
         {
            Ray3D rayToLight;
            rayToLight.set(p, dir);
            
            for (int i = 0 ; i < (int)spheres.size(); i++) 
            {
               // Determine shadow placement
               if(intersectSphere != i && spheres.at(i).sphere.get_intersection(rayToLight, p, n))
               {
                  shader.SetObject(spheres.at(intersectSphere).color,0.2,0,0,0);
               }
            }

            // Display surface normal
            if (mode == "normal")
            {
               image[y][x][0] = 127 + n.vx * 127;
               image[y][x][1] = 127 + n.vy * 127;
               image[y][x][2] = 127 + n.vz * 127;
            }
            
            // Calculate Phong shade
            if (mode == "phong")
            {
               ColorRGB theColor = spheres.at(intersectSphere).color;
               shader.GetShade(p, n, theColor);
               image[y][x][0] = theColor.R;
               image[y][x][1] = theColor.G;
               image[y][x][2] = theColor.B;
            }
         }
      }
   }
}
 
//---------------------------------------
// Init function for OpenGL
//---------------------------------------
void init()
{
   // Set new random seed
   srand(time(0));

   // Initialize OpenGL
   glClearColor(0.0, 0.0, 0.0, 1.0);

   // Clear all spheres
   spheres.clear();
   // Generate 8 random spheres
   for(int i = 0; i < 8 ; i++)
   {
      Sphere sphere = Sphere(myrand(-1,1),myrand(-1,1),myrand(0,3)+0.2,myrand(0.4,0.5));
      spheres.push_back(sphere);
   }

   // Perform ray tracing
   ray_trace();
}

//---------------------------------------
// Display callback for OpenGL
//---------------------------------------
void display()
{
   // Display image
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(XDIM, YDIM, GL_RGB, GL_UNSIGNED_BYTE, image);
   glFlush();
}

//---------------------------------------
// Keyboard callback for OpenGL
//---------------------------------------
void keyboard(unsigned char key, int x, int y)
{
   // End program
   if (key == 'q')
      exit(0);

   if (key == 'r')
   {
      init();
   }

   // Move camera position
   else if (key == '+' && position < 5)
      position = position * 1.1;
   else if (key == '-' && position > 1)
      position = position / 1.1;

   // Change display mode
   else if (key == 'n')
      mode = "normal";
   else if (key == 'p')
      mode = "phong";

   // Perform ray tracing
   ray_trace();
   glutPostRedisplay();
}

//---------------------------------------
// Main program
//---------------------------------------
int main(int argc, char *argv[])
{
   // Create OpenGL window
   glutInit(&argc, argv);
   glutInitWindowSize(XDIM, YDIM);
   glutInitWindowPosition(0, 0);
   glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
   glutCreateWindow("Ray Trace");
   // Print command menu
   cout << "Program commands:\n"
        << "   '+' - increase camera distance\n"
        << "   '-' - decrease camera distance\n"
        << "   'p' - show Phong shading\n"
        << "   'n' - show surface normals\n"
        << "   'q' - quit program\n"
        << "   'r' - draw new spheres\n";
   init();

   // Specify callback function
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutMainLoop();
   return 0;
}
