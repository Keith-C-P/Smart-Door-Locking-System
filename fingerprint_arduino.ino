#include <Adafruit_Fingerprint.h>

#define LOCK 12
#define BUTTON 11
#define DELAY 6000
#define ENROLLMENT 13 // make onboard led signify if in enrollment mode

bool enrollment = false;

SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

void setup(){
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LOCK, OUTPUT);
  pinMode(ENROLLMENT, OUTPUT);
  Serial.begin(9600);

  Serial.println("\n\nAdafruit finger detect test");
  finger.begin(57600);
  delay(5);

  while(true) {
    Serial.println("\n\nAdafruit finger detect test");
    if (finger.verifyPassword()) {
      Serial.println("Found fingerprint sensor!");
      break;
    } else {
      Serial.println("Did not find fingerprint sensor :(");
    }
    delay(5000); // waiting for 5 seconds before rechecking
  }

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }

  if (digitalRead(BUTTON) == LOW) {
    enrollment =  true;
    digitalWrite(ENROLLMENT, 1);
    Serial.println("###### ENROLLMENT MODE ######");
    Serial.println("Waiting for enrolled user's Fingerprint...");
  }
  else {
    digitalWrite(ENROLLMENT, 0);
    Serial.println("######## NORMAL MODE ########");
    Serial.println("Waiting for valid finger...");
  }
  //unlock();

}

void loop(){ // run over and over again
  int print = getFingerprintIDez();

  if (!(enrollment))
  {
    if (print != -1 || buttonPressed()) 
      unlock();
  }
  else {
    digitalWrite(LOCK, 0);
    Serial.print(".");
    if (print != -1) {
      bool enrollmentPrintFound = true;
      Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
      id = readnumber();
      if (id == 0) {// ID #0 not allowed, try again!
        return;
      }
      Serial.print("Enrolling ID #");
      Serial.println(id);

      uint8_t ok = getFingerprintEnroll();
      while (ok != FINGERPRINT_OK){
        ok = getFingerprintEnroll();
        Serial.println(ok);
      }
    }
  }
}

bool buttonPressed(){ // debounce the button
  if( digitalRead(BUTTON) == LOW){
    delay(10);
    if( digitalRead(BUTTON) == LOW){
      return true;
    }
    return false;
  }
  return false;
}

void unlock(){
  digitalWrite(LOCK, 0);
  delay(DELAY);
  digitalWrite(LOCK, 1);
}

uint8_t readnumber(void) { // get number from serial monitor
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}
// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}
/* TODO:
  - ADD PULLDOWN RESISTORS
*/