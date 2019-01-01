/*
 * Stepper.cpp - Stepper library for Wiring/Arduino - Version 1.1.0
 *
 * Original library        (0.1)   by Tom Igoe.
 * Two-wire modifications  (0.2)   by Sebastian Gassner
 * Combination version     (0.3)   by Tom Igoe and David Mellis
 * Bug fix for four-wire   (0.4)   by Tom Igoe, bug fix from Noah Shibley
 * High-speed stepping mod         by Eugene Kozlenko
 * Timer rollover fix              by Eugene Kozlenko
 * Five phase five wire    (1.1.0) by Ryan Orendorff
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Drives a unipolar, bipolar, or five phase stepper motor.
 *
 * When wiring multiple stepper motors to a microcontroller, you quickly run
 * out of output pins, with each motor requiring 4 connections.
 *
 * By making use of the fact that at any time two of the four motor coils are
 * the inverse of the other two, the number of control connections can be
 * reduced from 4 to 2 for the unipolar and bipolar motors.
 *
 * A slightly modified circuit around a Darlington transistor array or an
 * L293 H-bridge connects to only 2 microcontroler pins, inverts the signals
 * received, and delivers the 4 (2 plus 2 inverted ones) output signals
 * required for driving a stepper motor. Similarly the Arduino motor shields
 * 2 direction pins may be used.
 *
 * The sequence of control signals for 5 phase, 5 control wires is as follows:
 *
 * Step C0 C1 C2 C3 C4
 *    1  0  1  1  0  1
 *    2  0  1  0  0  1
 *    3  0  1  0  1  1
 *    4  0  1  0  1  0
 *    5  1  1  0  1  0
 *    6  1  0  0  1  0
 *    7  1  0  1  1  0
 *    8  1  0  1  0  0
 *    9  1  0  1  0  1
 *   10  0  0  1  0  1
 *
 * The sequence of control signals for 4 control wires is as follows:
 *
 * Step C0 C1 C2 C3
 *    1  1  0  1  0
 *    2  0  1  1  0
 *    3  0  1  0  1
 *    4  1  0  0  1
 *
 * The sequence of controls signals for 2 control wires is as follows
 * (columns C1 and C2 from above):
 *
 * Step C0 C1
 *    1  0  1
 *    2  1  1
 *    3  1  0
 *    4  0  0
 *
 * The circuits can be found at
 *
 * http://www.arduino.cc/en/Tutorial/Stepper
 */

#include "Arduino.h"
#include "Stepper.h"

/*
 *   constructor for four-pin version
 *   Sets which wires should control the motor.
 */
Stepper::Stepper(int number_of_steps, long whatSpeed, int motor_pin_1, int motor_pin_2,
                 int motor_pin_3, int motor_pin_4)
{

  this->number_of_steps = number_of_steps; // total number of steps for this motor
  setSpeed(whatSpeed);

  // Arduino pins for the motor control connection:
  this->motor_pin[0] = motor_pin_1;
  this->motor_pin[1] = motor_pin_2;
  this->motor_pin[2] = motor_pin_3;
  this->motor_pin[3] = motor_pin_4;

  // setup the pins on the microcontroller:
  for (int i = 0; i < 4; i++)
    pinMode(this->motor_pin[i], OUTPUT);
}

/*
 * Sets the speed in revs per minute
 */
void Stepper::setSpeed(long whatSpeed)
{
  this->step_delay = 60L * 1000L * 1000L / this->number_of_steps / whatSpeed;
}

/*
 * Moves the motor steps_to_move steps.  If the number is negative,
 * the motor moves in the reverse direction.
 */
void Stepper::step(int steps_to_move)
{

  if (steps_to_move != 0)
  {
    unsigned long last_step_time = 0; // time stamp in us of when the last step was taken

    int steps_left = abs(steps_to_move); // how many steps to take

    int direction = steps_left / -steps_to_move;

    // decrement the number of steps, moving one step each time:
    while (steps_left > 0)
    {
      unsigned long now = micros();
      int left = now - last_step_time;
      int esti = this->step_delay;

      //Stop slowly
      if (steps_left < 10)
      {
        esti += steps_left * 100;
      }

      // move only if the appropriate delay has passed:
      if (left >= esti)
      {
        // get the timeStamp of when you stepped:
        last_step_time = now;

        // decrement the steps left:
        steps_left--;

        int steps = (4 + (steps_left * direction) % 4) % 4;

        // step the motor to step number 0, 1, ..., {3 or 10}
        stepMotor(steps);
      }
    }
    //Reset motor to save power and motorlifetime
    resetMotor();
  }
}

/*
 * Moves the motor forward or backwards.
 */
void Stepper::stepMotor(int thisStep)
{
  for (int i = 0; i < 4; i++)
    digitalWrite(motor_pin[i], bitRead(0xCC, thisStep + i));
}
/*
 * Reset motor to save power and motorlifetime
*/
void Stepper::resetMotor()
{
  for (int i = 0; i < 4; i++)
    digitalWrite(motor_pin[i], LOW);
}

/*
  version() returns the version of the library:
*/
int Stepper::version(void)
{
  return 5;
}
