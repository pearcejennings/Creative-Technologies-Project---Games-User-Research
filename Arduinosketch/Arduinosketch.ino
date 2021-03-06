/**
 * GSR Sensor is connected to analog in A0
 */
const int GSR_SENSOR=A0;

/**
 * HR Sensor is connected to analog in A2
 */
const int HR_SENSOR=A1;

/**
 * The timestamp when the las heartbeat occurred.
 */
volatile unsigned long lastHearbeat = 0;

/**
 * The delta in milliseconds between the last heartbeats.
 */
volatile unsigned int hearbeatDelta = 0;

/**
 * Timeout counter as indicator, whether heartbeat data (interrupt) is available or not.
 */
volatile unsigned int heartbeatDataTimeout = 0;

/** 
 * Counter for processing loops used to control timed tasks. 
 */
unsigned int loopCounter = 0;

/** 
 * The timestamp of the last serial transmission. 
 */
unsigned long lastTransmission = 0;

/** 
 * The Transmission interval in milliseconds. 
 */
const unsigned int TRANSMISSION_INTERVAL = 100;

/**
 * Accumulated GSR values.
 * Accumulation will be used for grading.
 */
int gsrAccumulatedValues = 0;

/**
 * Accumulated HR values.
 * Accumulation will be used for grading.
 */
int HRAccumulatedValues = 0;



// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  attachInterrupt(0, interrupt, RISING);// Heartbeat detection: set interrupt 0,digital port 2
}


// the loop routine runs over and over again forever:
void loop() {
  
  loopCounter++;
  
  handleGSR();
  handleHeartbeat();

  
   // Check if serial transmission shall be performed:
   unsigned long now = millis();
   if(now - lastTransmission >= TRANSMISSION_INTERVAL) {
       lastTransmission = now;
       
       int gsrValue = computeGSRValue();
       int HRValue = computeHRValue();


        Serial.print("GSR>: "); Serial.print(analogRead(A0)); Serial.print("  ");
        Serial.print("HR>: "); Serial.print(analogRead(A1)); Serial.print("  ");
        
       

       // Send data via serial port:
       sendMessage(gsrValue, hearbeatDelta, HRValue, loopCounter);
       loopCounter = 0;

      
   }

    
  
  delay(500);        // delay in between reads for stability
}


/**
 * Send a message via serial port containing the given values.
 * 
 * Example: '{534;899;234;99}\r\n'
 * 
 * @param gsrValue The current GSR (galvanic skin response) value
 * @param heartbeatDelta The delta in milliseconds between the last two heatbeats, or 0, if invalid
 * @param HRValue the graded EMG value
 */
void sendMessage(int gsrValue, int heartbeatDeltaValue, int HRValue, int loopCounter) {
  Serial.print('{');
  Serial.print(gsrValue);
  Serial.print(';');
  Serial.print(heartbeatDeltaValue);
  Serial.print(';');
  Serial.print(HRValue);
  Serial.print(';');
  Serial.print(loopCounter);
  Serial.println('}');
}


/**
 * Takes measurement of the current GSR value and accumulates it for further grading.
 */
void handleGSR() {
  // read the GSR
  gsrAccumulatedValues += analogRead(GSR_SENSOR);
}


/**
 * Takes measurement of the current HR value and accumulates it for further grading.
 */
void handleHR() {
  // read the HR
  HRAccumulatedValues = analogRead(HR_SENSOR);
}


/**
 * Checks for heartbeat events or timeouts.
 */
void handleHeartbeat() {
  // Check indicator, if heartbeat data is available:
  if(heartbeatDataTimeout > 3000) {
      hearbeatDelta = 0; // no heart beat data available => invalidate
  }
  
  heartbeatDataTimeout++; // increase timeout counter
}


/**
 * Computes the graded GSR value using the accumulated GSR values.
 * @return the average value since last transmission
 */
int computeGSRValue() {
  int result =  gsrAccumulatedValues / loopCounter;
  gsrAccumulatedValues = 0; // reset for next run
  return result;
}


/**
 * Computes the graded HR value using the accumulated EMG values.
 * @return the average value since last transmission
 */
int computeHRValue() {
  //int result =  HRAccumulatedValues / loopCounter;
  //HRAccumulatedValues = 0; // reset for next run
  //return result;
  
  return HRAccumulatedValues;
}


/**
 * The ISR for heartbeat detection.
 */
void interrupt() {
  unsigned long now = millis();
  
  // Reset timeout counter (indicates that heartbeat data is available):
  heartbeatDataTimeout = 0;
  
  hearbeatDelta = now - lastHearbeat;
  lastHearbeat = now;
}
