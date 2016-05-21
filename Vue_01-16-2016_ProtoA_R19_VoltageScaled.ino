// Author: Julian Ong
// Company: Intelligent Product Solutions
// Date: 06/25/2015
// Version: 1.0
// Microcontroller: Teensy 3.1

//****************************************************
//     |--------|
//     |[vLimit]|
//     |--------|
//     | 50%    | --> 50% duty cycle to lower temp
//     |--------|
//     |[vHigh] |
//     |--------|
//     | 75%    | --> 75% duty cycle to maintain temp
//     |--------|
//     |[vLow]  |
//     |--------|
//     | 95%    | --> 95% duty cycle to increase temp
//     |--------|
//****************************************************
// CHANGE THE PARAMETERS BELOW
//****************************************************

float vLimit = 1.52;   // should correspond to ~420°F
float vHigh = 1.42;   // should correspond to ~370°F
float vLow = 1.38;    // should correspond to ~350°F

//****************************************************
// CHANGE THE PARAMETERS ABOVE ONLY,
// DON'T CHANGE THE CODE BELOW
//****************************************************

// Inputs
int offMode = 18;
int concentrateMode = 17;
int flowerMode = 16;
int duoMode = 15;
int pushButton = 24;
// int Vsense = [number] ;  // Analog pin only, no need to declare

// Outputs
int pwmRed = 22;
int pwmGreen = 20;
int pwmBlue = 21;
int concentrateFET = 5;
int flowerFET = 23;
int dummy = 14;

// Constants
float step2sec = 1000/255;  // 1000/255 = 1sec glow/fade time
float multiplier = 0.10;  // 0.0 <= multiplier <= 1.0
int timeout2 = 300000;  // 1000 = 1sec
int timeout3 = 300000;  // 1000 = 1sec
int totalSample = 1;

// Variables
int counter = 0;
int Vtherm = 0;
int sum = 0;
int avg = 0;
float Vavg = 0.0;
boolean inhaleFlag = LOW;
int timer2 = 0;
int timer3 = 0;
boolean buttonPressed = LOW;

// Test var
float vOverLimit = 0.0;

void setup()  {
  
  Serial.begin(9600);
  // Configure inputs/outputs
  pinMode(offMode, INPUT);
  pinMode(concentrateMode, INPUT);
  pinMode(flowerMode, INPUT);
  pinMode(duoMode, INPUT);
  pinMode(pushButton, INPUT);
  pinMode(dummy, INPUT);
//  pinMode(VSense, INPUT);  // Analog pin only, no need to declare
  pinMode(pwmRed, OUTPUT);
  pinMode(pwmGreen, OUTPUT);
  pinMode(pwmBlue, OUTPUT);
  pinMode(concentrateFET, OUTPUT);
  pinMode(flowerFET, OUTPUT);
}

// Main loop
void loop()  {
  // Off Mode
  if ((digitalRead(offMode)) && (!digitalRead(concentrateMode)) 
  && (!digitalRead(flowerMode)) && (!digitalRead(duoMode)))  {
    // Reset everything (optional because power is OFF)
    analogWrite(pwmRed, 0);
    analogWrite(pwmGreen, 0);
    analogWrite(pwmBlue, 0);
    analogWrite(concentrateFET, 255);  // PMOS
    analogWrite(flowerFET, 0);
    timer2 = 0;
    timer3 = 0;
    counter = 0;
    avg = 0;
    sum = 0;
    Vavg = 0.0;
    inhaleFlag = LOW;
    Vtherm = 0;
  }
  
  // Concentrate Mode
  if ((digitalRead(concentrateMode)) && (!digitalRead(offMode)) 
  && (!digitalRead(flowerMode)) && (!digitalRead(duoMode)))  {
    // Reset timers
    timer2 = 0;
    timer3 = 0;
    // Turn off Flower FET and reset parameters
    analogWrite(flowerFET, 0);
    inhaleFlag = LOW;
    counter = 0;
    sum = 0;
    avg = 0;
    Vavg = 0.0;
    // Button pressed
    if (digitalRead(pushButton))  {
      // Turn on Concentrate FET
      digitalWrite(concentrateFET, LOW);
      // Solid Yellow
      for (int i = 0; i <= 255; i++)  {
        analogWrite(pwmRed, 85*multiplier);
        analogWrite(pwmGreen, 255*multiplier);
        analogWrite(pwmBlue, 0*multiplier);
        delay(step2sec);
      }
    }
    // Button not pressed
    else  {
      digitalWrite(concentrateFET, HIGH);
      // Yellow Pulses (Glow)
      for (int i = 0; i <= 255; i++)  {
        analogWrite(pwmRed, i*multiplier/3);
        analogWrite(pwmGreen, i*multiplier);
        analogWrite(pwmBlue, 0*multiplier);
        // Break loop if button is pressed
        if (digitalRead(pushButton))  {
          digitalWrite(concentrateFET, LOW);
          break;
        }
        delay(step2sec);
      }
      // Yellow Pulses (Fade)
      for (int i = 255; i >= 0; i--)  {
        analogWrite(pwmRed, i*multiplier/3);
        analogWrite(pwmGreen, i*multiplier);
        analogWrite(pwmBlue, 0*multiplier);
        // Break loop if button is pressed
        if (digitalRead(pushButton))  {
          digitalWrite(concentrateFET, LOW);
          break;
        }
        delay(step2sec);
      }
    }
  }
  // Flower Mode
  if ((digitalRead(flowerMode)) && (!digitalRead(offMode))
  && (!digitalRead(concentrateMode)) && (!digitalRead(duoMode)))  {
    // Reset Duo Mode timer
    timer3 = 0;
    // Engage in Flower Mode if not timeout
    if (timer2 <= timeout2)  {
      // PWM 95% if voltage is less than or equal to 
      if (Vavg <= vLow)  {
        // Orange Pulses (Glow)
        for (int i = 0; i <= 255; i++)  {
          analogWrite(pwmRed, i*multiplier);
          analogWrite(pwmGreen, i*multiplier);
          analogWrite(pwmBlue, 0*multiplier);
          //timer2 += step2sec;
          if (digitalRead(pushButton))  {
            timer2 = 0;
            buttonPressed = HIGH;
            break;
          }
          else  {
            buttonPressed = LOW;
          }
          delay(step2sec);
        }
        // Orange Pulses (Fade)
        for (int i = 255; i >= 0; i--)  {
          analogWrite(pwmRed, i*multiplier);
          analogWrite(pwmGreen, i*multiplier);
          analogWrite(pwmBlue, 0*multiplier);
          //timer2 += step2sec;
          if (digitalRead(pushButton))  {
            timer2 = 0;
            buttonPressed = HIGH;
            break;
          }
          else  {
            buttonPressed = LOW;
          }
          analogWrite(flowerFET, 242);
          delay(step2sec);
        }
      }
      // PWM 75% if voltage is in between
      if ((Vavg > vLow) && (Vavg <= vHigh))  {
        // Solid Green
        for (int i = 0; i <= 255; i++)  {
          analogWrite(pwmRed, 0*multiplier);
          analogWrite(pwmGreen, 255*multiplier);
          analogWrite(pwmBlue, 0*multiplier);
          if (digitalRead(pushButton))  {
            timer2 = 0;
            buttonPressed = HIGH;
            break;
          }
          else  {
            buttonPressed = LOW;
          }
          timer2 += step2sec;
          analogWrite(flowerFET, 191);
          delay(step2sec);
        }
      }
      // PWM 50% if voltage is greater than 
      if (Vavg > vHigh)  {
        // Solid Green
        for (int i = 0; i <= 255; i++)  {
          analogWrite(pwmRed, 0*multiplier);
          analogWrite(pwmGreen, 255*multiplier);
          analogWrite(pwmBlue, 0*multiplier);
          if (digitalRead(pushButton))  {
            timer2 = 0;
            buttonPressed = HIGH;
            break;
          }
          else  {
            buttonPressed = LOW;
          }
          timer2 += step2sec;
          analogWrite(flowerFET, 127);
          delay(step2sec);
        }
      }
      // Button Pressed
      if (buttonPressed == HIGH)  {
        timer2 = 0;
        if (Vavg >= vLimit)  {
          analogWrite(flowerFET, 192);  // Reduce PWM to 75%
        }
        else  {
          analogWrite(flowerFET, 255);  // Max PWM!
        }
        for (int i = 0; i <= 255; i++)  {
          analogWrite(pwmRed, 255*multiplier);
          analogWrite(pwmGreen, 0*multiplier);
          analogWrite(pwmBlue, 0*multiplier);
          delay(step2sec);
        }
      }
      // Read thermistor voltage
      Vtherm = analogRead(A12);
      Vavg = float(Vtherm*3.3)/1024;
      Serial.print("Vavg: ");
      Serial.println(Vavg);
    }
    // Engage in Timeout otherwise
    else  {
      analogWrite(flowerFET, 0);
      // Purple Pulses (Glow)
      for (int i = 0; i <= 255; i++)  {
        analogWrite(pwmRed, i/1.59375*multiplier);
        analogWrite(pwmGreen, i/7.96875*multiplier);
        analogWrite(pwmBlue, i/1.0625*multiplier);
        // Break timeout if button is pressed
        if (digitalRead(pushButton))  {
          timer3 = 0;
          //buttonPressed = HIGH;
          break;
        }
        else {
          buttonPressed = LOW;
        }
        delay(step2sec);
      }
      // Purple Pulses (Fade)
      for (int i = 255; i >= 0; i--)  {
        analogWrite(pwmRed, i/1.59375*multiplier);
        analogWrite(pwmGreen, i/7.96875*multiplier);
        analogWrite(pwmBlue, i/1.0625*multiplier);
        // Break timeout if button is pressed
        if (digitalRead(pushButton))  {
          timer3 = 0;
          //buttonPressed = HIGH;
          break;
        }
        else {
          buttonPressed = LOW;
        }
        delay(step2sec);
      }
    }
  }
  // Duo Mode
  if ((digitalRead(duoMode)) && (!digitalRead(offMode)) 
  && (!digitalRead(concentrateMode)) && (!digitalRead(flowerMode))) {
    // Reset Flower Mode timer
    timer2 = 0;
    // Engage in Duo Mode if not timeout
    if (timer3 <= timeout3)  {
      // PWM 95% if voltage is less than or equal to 
      if (Vavg <= vLow)  {
        // Orange Pulses (Glow)
        for (int i = 0; i <= 255; i++)  {
          analogWrite(pwmRed, i*multiplier);
          analogWrite(pwmGreen, i*multiplier);
          analogWrite(pwmBlue, 0*multiplier);
          //timer3 += step2sec;
          if (digitalRead(pushButton))  {
            timer3 = 0;
            buttonPressed = HIGH;
            break;
          }
          else  {
            buttonPressed = LOW;
          }
          analogWrite(flowerFET, 242);
          delay(step2sec);
        }
        // Orange Pulses (Fade)
        for (int i = 255; i >= 0; i--)  {
          analogWrite(pwmRed, i*multiplier);
          analogWrite(pwmGreen, i*multiplier);
          analogWrite(pwmBlue, 0*multiplier);
          //timer3 += step2sec;
          if (digitalRead(pushButton))  {
            timer3 = 0;
            buttonPressed = HIGH;
            break;
          }
          else  {
            buttonPressed = LOW;
          }
          delay(step2sec);
        }
      }
      // PWM 75% if voltage is in between 
      if ((Vavg > vLow) && (Vavg <= vHigh))  {
        // Solid Blue
        for (int i = 0; i <= 255; i++)  {
          analogWrite(pwmRed, 0*multiplier);
          analogWrite(pwmGreen, 0*multiplier);
          analogWrite(pwmBlue, 255*multiplier);
          timer3 += step2sec;
          if (digitalRead(pushButton))  {
            timer3 = 0;
            buttonPressed = HIGH;
            break;
          }
          else  {
            buttonPressed = LOW;
          }
          analogWrite(flowerFET, 191);
          delay(step2sec);
        }
      }
      // PWM 50% if voltage is greater than 
      if (Vavg > vHigh)  {
        // Solid Blue
        for (int i = 0; i <= 255; i++)  {
          analogWrite(pwmRed, 0*multiplier);
          analogWrite(pwmGreen, 0*multiplier);
          analogWrite(pwmBlue, 255*multiplier);
          timer3 += step2sec;
          if (digitalRead(pushButton))  {
            timer3 = 0;
            buttonPressed = HIGH;
            break;
          }
          else  {
            buttonPressed = LOW;
          }
          analogWrite(flowerFET, 127);
          delay(step2sec);
        }
      }
      // Button pressed Concentrate + Flower
      if (buttonPressed == HIGH)  {
        timer3 = 0;
        if (Vavg >= vLimit)  {
          analogWrite(flowerFET, 192);  // Reduce PWM to 75%
        }
        else  {
          analogWrite(flowerFET, 255);  // Max PWM!
        }
        digitalWrite(concentrateFET, LOW);  // Power on Concentrate
        // Solid Yellow
        for (int i = 0; i <= 255; i++)  {
          analogWrite(pwmRed, 85*multiplier/3);
          analogWrite(pwmGreen, 255*multiplier/3);
          analogWrite(pwmBlue, 0*multiplier);
          delay(step2sec);
        }
      }
      else  {
        digitalWrite(concentrateFET, HIGH);  // Power off Concentrate
      }
      // Read thermistor voltage
      Vtherm = analogRead(A12);
      Vavg = float(Vtherm*3.3)/1024;
      Serial.print("Vavg: ");
      Serial.println(Vavg);
    }
    // Engage in Timeout otherwise
    else  {
      analogWrite(flowerFET, 0);
      // Purple Pulses (Glow)
      for (int i = 0; i <= 255; i++)  {
        analogWrite(pwmRed, i/1.59375*multiplier);
        analogWrite(pwmGreen, i/7.96875*multiplier);
        analogWrite(pwmBlue, i/1.0625*multiplier);
        // Break timeout if button is pressed
        if (digitalRead(pushButton))  {
          timer3 = 0;
          //buttonPressed = HIGH;
          break;
        }
        else {
          buttonPressed = LOW;
        }
        delay(step2sec);
      }
      // Purple Pulses (Fade)
      for (int i = 255; i >= 0; i--)  {
        analogWrite(pwmRed, i/1.59375*multiplier);
        analogWrite(pwmGreen, i/7.96875*multiplier);
        analogWrite(pwmBlue, i/1.0625*multiplier);
        // Break timeout if button is pressed
        if (digitalRead(pushButton))  {
          timer3 = 0;
          //buttonPressed = HIGH;
          break;
        }
        else {
            buttonPressed = LOW;
        }
        delay(step2sec);
      }
    }
  }
}
