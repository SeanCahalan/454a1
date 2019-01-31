// ship.cpp


#include "ship.h"


void Ship::rotateCW( float deltaT )

{
  orientation = quaternion( - SHIP_ROTATION_SPEED * deltaT, vec3(0,0,1) ) * orientation;
}


void Ship::rotateCCW( float deltaT )

{
  orientation = quaternion( + SHIP_ROTATION_SPEED * deltaT, vec3(0,0,1) ) * orientation;
}


void Ship::addThrust( float deltaT )

{
  // Thrust is in the ship's +y direction.  Make sure to change the
  // velocity in that direction in the *world* coordinate system,
  // since the object velocity is in the world coordinate system.

  // YOUR CODE HERE

  float vX = velocity.x - sin( orientation.angle() ) * SHIP_THRUST_ACCEL * deltaT;
  float vY = velocity.y + cos( orientation.angle() ) * SHIP_THRUST_ACCEL * deltaT;

  velocity = vec3( vX, vY, 0);
}


Shell * Ship::fireShell()

{
  // YOUR CODE HERE (below, find the correct position, velocity, and orientation for the shell)

  vec3 Ps = vec3(
    position.x - sin( orientation.angle() ) * 10,
    position.y + cos( orientation.angle() ) * 10,
    0
  );  

  vec3 Vs = vec3(
    SHELL_SPEED * sin( orientation.angle() ) * -1,
    SHELL_SPEED * cos( orientation.angle() ),
    0
  );

  return new Shell( Ps, Vs, quaternion(1,0,0,0) );
}


// Ship model consisting of line segments
//
// These are in a ARBITRARY coordinate system and get remapped to the
// world coordinate system (with the centre of the ship at (0,0) and
// width SHIP_WIDTH) when the VAO is set up.


float Ship::verts[] = {

   3,0,  0,9,
   0,9, -3,0, 
  -3,0,  0,1,
   0,1,  3,0,

  9999
};

