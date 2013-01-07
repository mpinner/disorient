#include <disorient.h>
#include <IRremote.h>



const int letterCount = 6;
String letters = " SHARI";

const int pinCount = 16;
const int pins[pinCount+1] =
{-1, // start array at 1 to keep consistent with datasheet
A2, A1, A0, 13, 12, 11, 10, A3,
9, 8, 7, 6, 5, 4, A5, 2
};


// col[xx] of leds = pin yy on led matrix
const int cols[8] = {pins[13], pins[3], pins[4], pins[10], pins[06], pins[11], pins[15], pins[16]};

// row[xx] of leds = pin yy on led matrix
const int rows[8] = {pins[9], pins[14], pins[8], pins[12], pins[1], pins[7], pins[2], pins[5]};


int RECV_PIN = 3;
IRrecv irrecv(RECV_PIN);
decode_results results;
// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state



int currentLetter = 0;
long previousMillis = 0; // stores the last time the letter was updated

long interval = 400; //duration to display each letter;

void setup() {
  
  letters.toUpperCase();
  
  Serial.begin(9600);

  irrecv.enableIRIn(); // Start the receiver

  
  // sets the pins as output
  for (int i = 1; i <= 16; i++) {
    pinMode(pins[i], OUTPUT);
  }

  // set up cols and rows
  for (int i = 0; i < 8; i++) {
    digitalWrite(cols[i], LOW);
  }

  for (int i = 0; i < 8; i++) {
    digitalWrite(rows[i], HIGH);
  }

return;  
}

void loop() {
  
  irrecv.enableIRIn(); // Re-enable receiver

  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;   

    currentLetter++; //increment letter
    if (currentLetter == letterCount) {
      currentLetter = 0; // or repeat
    }
    
  
  }
  
  
//  for(int i=0; i<letterCount; i++) {
    for (int j=0; j<20; j++) {
      //currentLetter = i;
      displayLetter(letters[currentLetter]);
    //  delayMicroseconds(10);
      if (irrecv.decode(&results)) {
            storeCode(&results);
            if (0xA90 == codeValue) {
              currentLetter = 0;
            }  
            if(0x90 == codeValue) {
              char newLetter = letters.charAt(currentLetter)+1;
              letters.setCharAt(currentLetter, newLetter);
            }
            if(0x890 == codeValue) {
              char newLetter = letters.charAt(currentLetter)-1;
              letters.setCharAt(currentLetter, newLetter);
            }
            if(0x490 == codeValue) {
              interval += 10;
            }
            if(0xC90 == codeValue) {
              interval -= 10;
            }
            

            irrecv.resume(); // resume receiver

      }
    }
//  }
  
}


void displayLetter(char letter) {
  displayEncodedLetter(font[letter]); 
  return;
}

void displayEncodedLetter(char * codedLetter) {
 
  
  for(int x=0; x<8; x++) {
    digitalWrite(cols[x],LOW);

    for (int y=0; y<8; y++) {
      if (codedLetter[x] & 1 << 7-y) {
        digitalWrite(rows[y],LOW);
      } else {
        digitalWrite(rows[y],HIGH);
      }
    
    }
    digitalWrite(cols[x],HIGH);
    delay(1);
    digitalWrite(cols[x],LOW);
  }
} 



void storeCode(decode_results *results) {
  codeType = results->decode_type;
  int count = results->rawlen;
  if (codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= codeLen; i++) {
      if (i % 2) {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else {
    if (codeType == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (codeType == SONY) {
      Serial.print("Received SONY: ");
    } 
    else if (codeType == RC5) {
      Serial.print("Received RC5: ");
    } 
    else if (codeType == RC6) {
      Serial.print("Received RC6: ");
    } 
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    codeValue = results->value;
    codeLen = results->bits;
  }
}



