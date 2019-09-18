// serial rate
#define baudRate 57600
#define ledPin 13
#define serialDebug true

#include <MIDI.h>
#include <MPR121.h>
#include <Wire.h>


MIDI_CREATE_DEFAULT_INSTANCE();

// List used MPR121 pins
const int numSensors = 12;
const int sensors[numSensors] = {0,1,2,3,4,5,6,7,8,9,10,11};


// MIDI Notes and stuff
const int channel = 1;
int states[numSensors] = {0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
int rootNote = 34;
int scale[15] = {0, 2, 3, 5, 7, 8, 11, 12, 14, 15, 17, 19, 20, 23, 24};
const int numOctaves = 6;
int lastOctave = 0;
int octave[numOctaves] = {1, 0, 1, 0, 0, 2};

unsigned long lastNote = 0;


const int notes[numSensors] =   {36, 38, 40, 41, 43, 45, 47, 60, 72, 65, 69, 59};


// this is the touch threshold - setting it low makes it more like a proximity trigger
// default value is 40 for touch
const int touchThreshold = 2;
// this is the release threshold - must ALWAYS be smaller than the touch threshold
// default value is 20 for touch
const int releaseThreshold = 1;

 // FIX 1: adaptive thresholds
int inputAmounts[12] =      {  3,  4, 5, 6, 7, 8, 8, 7, 6, 5,  4,  3 };
int touchThresholds[12] =   {  1,  3, 4, 6, 7, 8, 8, 7, 6, 4,  3,  1 };
int releaseThresholds[12] = { -2, -1, 0, 2, 3, 4, 4, 3, 2, 0, -1, -2 };


// FIX 2 : take off your shoes


int i,j = 0;

void setup(){  
  pinMode(ledPin, OUTPUT);
  doBlink(1);
  
  Serial.begin(baudRate);
  //while(!Serial); // only needed for Arduino Leonardo or Bare Touch Board 
  doBlink(2);
  
  Wire.setSDA(18);
  Wire.setSCL(19);
  if(!MPR121.begin(0x5A)){ 
    Serial.println("error setting up MPR121");  
    switch(MPR121.getError()){
      case NO_ERROR:
        Serial.println("no error");
        break;  
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;      
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;      
    }
    while(1);
  }

  doBlink(3);


  MPR121.setTouchThreshold(touchThreshold);
  MPR121.setReleaseThreshold(releaseThreshold);  

/*
    // FIX 1: adaptive thresholds
  for(j=0;j<12;++j) {
    MPR121.setTouchThreshold( touchThresholds[j], j );
    MPR121.setReleaseThreshold( releaseThresholds[j], j );
  }*/

  
  // FIX 2 : dynamic thresholds
/*  for(j=0;j<12;++j) {
    touchThresholds[j] = MPR121.getBaselineData(j) + inputAmounts[j];
    releaseThresholds[j] = touchThresholds[j] - inputAmounts[j] / 2;
    MPR121.setTouchThreshold( touchThresholds[j], j );
    MPR121.setReleaseThreshold( releaseThresholds[j], j );
  }
*/

  doBlink(4);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  doBlink(5);

  for(j=0;j<6;++j) {
    lastOctave = 0;
    i = noteFor(j * 2);
    usbMIDI.sendNoteOn(i, 100, channel);
    MIDI.sendNoteOn(i, 100, channel); 
    delay(40);
    usbMIDI.sendNoteOff(i, 0, channel);
    MIDI.sendNoteOff(i, 0, channel); 
    delay(60);
  }
  
}

void doBlink(int n) {
  for (j=0;j<n;j++) {
    digitalWrite(ledPin, HIGH);
  delay(50);   
  digitalWrite(ledPin, LOW);
  delay(50); 
  }
  delay(200); 
}

void loop(){
   readMPRInputs();  
}

void readMPRInputs(){    
    MPR121.updateAll();
    for(i=0;i<numSensors;i++) {
      
      if(MPR121.isNewTouch(sensors[i])) {
        states[i] = noteFor(i);
        usbMIDI.sendNoteOn(states[i], 100, channel);
        MIDI.sendNoteOn(states[i], 100, channel);
        digitalWrite(ledPin, HIGH);
      }
      if (MPR121.isNewRelease(sensors[i])) {
        MIDI.sendNoteOff(states[i], 0, channel);
        usbMIDI.sendNoteOff(states[i], 0, channel);
        states[i] = 0;
        digitalWrite(ledPin, LOW);
      }
      
      if (serialDebug) {
        Serial.print(MPR121.getBaselineData(sensors[i])-MPR121.getFilteredData(sensors[i]), DEC);
        Serial.print(" ");
      }
    }
    if (serialDebug) {
      Serial.print(MPR121.getTouchThreshold(i), DEC);
      Serial.print(" ");
      Serial.print(MPR121.getReleaseThreshold(i), DEC);
      Serial.print(" ");
      Serial.println();
    }
}

/*
 int root = 44;
int scale[15] = {0, 2, 3, 5, 7, 8, 11, 12, 14, 15, 17, 19, 20, 23, 24};
const int numOctaves = 6;
int lastOctave = 0;
int octave[numOctaves] = {0, 1, 0, -1, -1, 1};
 */

int noteFor(int index) {
  
  if ( millis() > lastNote + 30000 ) {
    lastNote = millis();
    rootNote = random(30, 42);
  }
  lastOctave = (lastOctave + 1) % numOctaves;
  return rootNote + octave[lastOctave] * 12 + scale[index];
}
