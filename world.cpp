// world.cpp


#include "world.h"
#include "ship.h"
#include "main.h"
#include "gpuProgram.h"
#include "strokefont.h"

#include <sstream>


void World::start()

{
  // Create a ship at the centre of the world

  ship = new Ship( 0.5 * (worldMin+worldMax) );

  // Create some large asteroids

  // Pick a random position at least 20% away from the origin (which
  // is where the ship will be).

  asteroids.clear();
  shells.clear();

  for (int i=0; i<NUM_INITIAL_ASTEROIDS; i++) {

    vec3 pos;
    do {
      pos = vec3( randIn01(), randIn01(), 0 );
    } while ((pos - vec3( 0.5, 0.5, 0 )).length() < 0.20);

    asteroids.push_back( new Asteroid( pos % (worldMax - worldMin) + worldMin ) );
  }

  // Increase asteroid speed in later rounds

  for (unsigned int  i=0; i<asteroids.size(); i++) {
    asteroids[i]->velocity = ((1+round) * ASTEROID_SPEED_ROUND_FACTOR) * asteroids[i]->velocity;
    asteroids[i]->angularVelocity = ((1+round) * ASTEROID_SPEED_ROUND_FACTOR) * asteroids[i]->angularVelocity;
  }

  state = RUNNING;
}


void World::updateState( float elapsedTime )

{
  if (state == PAUSED)
    return;

  if (asteroids.size() == 0) {
    round++;
    start();
    return;
  }

  // See if any keys are pressed for thrust

  if (state == RUNNING) {

    if (rightKey == DOWN)
      ship->rotateCW( elapsedTime );

    if (leftKey == DOWN)
      ship->rotateCCW( elapsedTime );

    if (upKey == DOWN)
      ship->addThrust( elapsedTime );
  }

  // Update the ship

  ship->updatePose( elapsedTime );

  // Update the asteroids (check for asteroid collisions)

  for (unsigned int i=0; i<asteroids.size(); i++) {
    asteroids[i]->updatePose( elapsedTime );
    if (state == RUNNING && ship->intersects( *asteroids[i] ))
      gameOver();
  }

  // Update the shells (check for asteroid collisions)

  for (unsigned int i=0; i<shells.size() && i >= 0; i++) {
    shells[i]->elapsedTime += elapsedTime;

    if (shells[i]->elapsedTime > SHELL_MAX_TIME) { // remove an old shell

      shells.erase( shells.begin() + i );
      i--;

    } else { // move a not-too-old shell

      vec3 prevPosition = shells[i]->centrePosition();
      shells[i]->updatePose( elapsedTime );

      // Check for shell/asteroid collision

      Segment path( prevPosition, shells[i]->centrePosition() );

      // YOUR CODE HERE
      //
      // Check each of the asteroids to see if it has intersected the
      // shell's path.  If so, either (a) remove the asteroid if it is
      // too small or (b) break the asteroids into two.  Also
      // increment the score according to the asteroid's scoreValue.
      //
      // - An asteroid is removed if (asteroids->scaleFactor * ASTEROID_SCALE_FACTOR_REDUCTION < MIN_ASTEROID_SCALE_FACTOR).
      //
      // - A split asteroid should add velocities to the two
      //   sub-asteroids in opposite directions perpendicular to the
      //   direction of the shell.
      // 
      // - the sub-asteroid scaleFactor and scoreValue should be
      //   modified from those of the parent asteroid.

      for (unsigned int j=0; j<asteroids.size(); j++) {

        if (state == RUNNING && asteroids[j]->intersects( path )){

          // update score
          score += asteroids[j]->scoreValue;

          // stop shell from hitting other asteroids
          shells.erase(shells.begin()+i);
          i--;

          // remove asteroid
          if (asteroids[j]->scaleFactor * ASTEROID_SCALE_FACTOR_REDUCTION < MIN_ASTEROID_SCALE_FACTOR) {
            asteroids.erase(asteroids.begin()+j);
            j--;
          // Split asteroid	
          } else {  
            float x = asteroids[j]->position.x;
            float y = asteroids[j]->position.y;
            float vx = asteroids[j]->velocity.x * ASTEROID_SPEED_INCREASE;
            float vy = asteroids[j]->velocity.y * ASTEROID_SPEED_INCREASE;

            // create left and right splits with new velocity and scale factor
            Asteroid *aleft = new Asteroid( vec3(x, y, 0) );
            aleft->velocity = vec3(vx, -vy, 0);
            aleft->scoreValue = asteroids[j]->scoreValue * 2;
            aleft->scaleFactor = asteroids[j]->scaleFactor * ASTEROID_SCALE_FACTOR_REDUCTION;

            Asteroid *aright = new Asteroid( vec3(x, y, 0) );
            aright->velocity = vec3(-vx, vy, 0);
            aright->scoreValue = asteroids[j]->scoreValue * 2;
            aright->scaleFactor = asteroids[j]->scaleFactor * ASTEROID_SCALE_FACTOR_REDUCTION;

            asteroids.push_back(aleft);
            asteroids.push_back(aright);

            asteroids.erase(asteroids.begin()+j);
            j--;

            break;
          }
        }
      }

    }
  }
}


void World::draw()

{
  // Transform [worldMin,worldMax] to [(-1,-1),(+1,+1)].

  mat4 worldToViewTransform;

  worldToViewTransform
    = translate( -1, -1, 0 )
    * scale( 2.0/(worldMax.x-worldMin.x), 2.0/(worldMax.y-worldMin.y), 1 )
    * translate( -worldMin.x, -worldMin.y, 0 );

  // Draw all world elements, passing in the worldToViewTransform so
  // that they can append their own transforms before passing the
  // complete transform to the vertex shader.

  ship->draw( worldToViewTransform );

  for (unsigned int i=0; i<shells.size(); i++)
    shells[i]->draw( worldToViewTransform );

  for (unsigned int i=0; i<asteroids.size(); i++)
    asteroids[i]->draw( worldToViewTransform );

  // Draw the title

  drawStrokeString( "ASTEROIDS", 0, 0.85, 0.06, glGetUniformLocation( myGPUProgram->id(), "MVP"), CENTRE );

  // Draw messages according to game state

  if (state == BEFORE_GAME) 

    drawStrokeString( "PRESS 's' TO START, 'p' TO PAUSE DURING GAME", 0, -.9, 0.06, glGetUniformLocation( myGPUProgram->id(), "MVP"), CENTRE );

  else {

    // draw score

    stringstream ss;
    ss.setf( ios::fixed, ios::floatfield );
    ss.precision(1);
    ss << "SCORE " << score;
    drawStrokeString( ss.str(), -0.95, 0.75, 0.06, glGetUniformLocation( myGPUProgram->id(), "MVP"), LEFT );

    if (state == AFTER_GAME) {

      // Draw "game over" message

      drawStrokeString( "GAME OVER", 0, 0, 0.12, glGetUniformLocation( myGPUProgram->id(), "MVP"), CENTRE );
      drawStrokeString( "PRESS 's' TO START, 'p' TO PAUSE DURING GAME", 0, -0.9, 0.06, glGetUniformLocation( myGPUProgram->id(), "MVP"), CENTRE );
    }
  }
}


void World::gameOver()

{
  state = AFTER_GAME;
}


void World::togglePause()

{
  if (state == RUNNING)
    state = PAUSED;
  else if (state == PAUSED)
    state = RUNNING;
}
