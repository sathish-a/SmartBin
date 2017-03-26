#include <EEPROM.h>
#define TINY_GSM_MODEM_SIM900
#include <TinyGsmClient.h>
#include <SoftwareSerial.h>
#include <NewPing.h>
#include "TinyGPS++.h"
#include "SoftwareSerial.h"
// Absolute min and max eeprom addresses. Actual values are hardware-dependent.
// These values can be changed e.g. to protect eeprom cells outside this range.
const int EEPROM_MIN_ADDR = 0;
const int EEPROM_MAX_ADDR = 511;

// Returns true if the address is between the
// minimum and maximum allowed values, false otherwise.
//
// This function is used by the other, higher-level functions
// to prevent bugs and runtime errors due to invalid addresses.
boolean eeprom_is_addr_ok(int addr) {
  return ((addr >= EEPROM_MIN_ADDR) && (addr <= EEPROM_MAX_ADDR));
}

// Writes a sequence of bytes to eeprom starting at the specified address.
// Returns true if the whole array is successfully written.
// Returns false if the start or end addresses aren't between
// the minimum and maximum allowed values.
// When returning false, nothing gets written to eeprom.
boolean eeprom_write_bytes(int startAddr, const byte* array, int numBytes) {
  // counter
  int i;
  // both first byte and last byte addresses must fall within
  // the allowed range
  if (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes)) {
    return false;
  }
  for (i = 0; i < numBytes; i++) {
    EEPROM.write(startAddr + i, array[i]);
  }
  return true;
}

// Writes a string starting at the specified address.
// Returns true if the whole string is successfully written.
// Returns false if the address of one or more bytes fall outside the allowed range.
// If false is returned, nothing gets written to the eeprom.
boolean eeprom_write_string(int addr, const char* string) {

  int numBytes; // actual number of bytes to be written
  //write the string contents plus the string terminator byte (0x00)
  numBytes = strlen(string) + 1;
  return eeprom_write_bytes(addr, (const byte*)string, numBytes);
}

// Reads a string starting from the specified address.
// Returns true if at least one byte (even only the string terminator one) is read.
// Returns false if the start address falls outside the allowed range or declare buffer size is zero.
//
// The reading might stop for several reasons:
// - no more space in the provided buffer
// - last eeprom address reached
// - string terminator byte (0x00) encountered.
boolean eeprom_read_string(int addr, char* buffer, int bufSize) {
  byte ch; // byte read from eeprom
  int bytesRead; // number of bytes read so far
  if (!eeprom_is_addr_ok(addr)) { // check start address
    return false;
  }

  if (bufSize == 0) { // how can we store bytes in an empty buffer ?
    return false;
  }
  // is there is room for the string terminator only, no reason to go further
  if (bufSize == 1) {
    buffer[0] = 0;
    return true;
  }
  bytesRead = 0; // initialize byte counter
  ch = EEPROM.read(addr + bytesRead); // read next byte from eeprom
  buffer[bytesRead] = ch; // store it into the user buffer
  bytesRead++; // increment byte counter
  // stop conditions:
  // - the character just read is the string terminator one (0x00)
  // - we have filled the user buffer
  // - we have reached the last eeprom address 
  while ( (ch != 0x00) && (bytesRead < bufSize) && ((addr + bytesRead) <= EEPROM_MAX_ADDR) ) {
    // if no stop condition is met, read the next byte from eeprom
    ch = EEPROM.read(addr + bytesRead);
    buffer[bytesRead] = ch; // store it into the user buffer
    bytesRead++; // increment byte counter
  }
  // make sure the user buffer has a string terminator, (0x00) as its last byte
  if ((ch != 0x00) && (bytesRead >= 1)) {
    buffer[bytesRead - 1] = 0;
  }
  return true;
}

// Select your modem:
/*       METHODS          DESCRIPTION              PARAMETERS TO PASS                         RESPOONSE
  __________________________________________________________________________________________________________________________________

      ADD_BIN           To add new bin              function , level                          New Id of Dustbin , 1(Error)

      UPDT_LEV          To update the level         function , binid, level                   100(SUCCESS) , 1(ERROR)
                        of existing bin

      UPDT_LOC          To Update the location      function , binid, lat , lng               100(SUCCESS) , 1(ERROR)
                        of existing bin
  _____________________________________________________________________________________________________________________________________
*/
#define TRIGGER_PIN  6  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     5  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 100 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define LAT 13.083345;
#define LNG 80.231241;
#define port 80 // port number https:443 http:80
//airtelgprs.com
const char apn[]  = "airtelgprs.com";
const char user[] = "";
const char pass[] = "";
String binId = "";
const char server[] = "smartbin.hostei.com";
const char resource[] = "/smart.php";
const int BUFSIZE = 21;
char buf[BUFSIZE];
char myIDChar[BUFSIZE];

const int addrBIN = 5; // Address where Bind is stored till (5+21)
const int addrIsBIN = 2; // Address where BinId is present or not is saved

const int defValBIN = 99;
//99 => already present

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
SoftwareSerial SerialAT(9, 10); // RX, TX
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
SoftwareSerial gps_serial(11, 12); //RX=pin 11, TX=pin 12
TinyGPSPlus gps;

void setup() {
  // Set console baud rate
  Serial.begin(9600);
  delay(10);
  gps_serial.begin(9600);
  // Set GSM module baud rate
  SerialAT.begin(9600);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");


  Serial.print("Waiting for network...");

  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    delay(10000);
    return;
  }

  Serial.println(" OK");

  Serial.print("Connecting to ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" OK");


  //Check whether binId is known or not
  int x = EEPROM.read(addrIsBIN);
  if (x == defValBIN)
  {
    Serial.println("BINID PRESENT");
    eeprom_read_string(addrBIN, buf, BUFSIZE);
    delay(1000);
    String tmp(buf);
    binId = tmp;
    Serial.println(binId);
    findLocation();
  }
  else
  {
    Serial.println("BINID NOT PRESENT");
    addBin(0, &binId);
    while (binId != "")
    {
      Serial.println("BinId:");
      Serial.print(binId);
      binId.toCharArray(myIDChar, BUFSIZE);
      strcpy(buf, myIDChar);
      eeprom_write_string(addrBIN, buf);
      EEPROM.write(addrIsBIN, defValBIN);
      Serial.println("WRITTEN into EEPROM");
      eeprom_read_string(addrBIN, buf, BUFSIZE);
      Serial.println(buf);
      findLocation();
      break;
    }

  }

}

int prev = 0;

void loop() {


  int dist;
  delay(10000);


  int a[3];
  int i = 0;
  while (i <= 2)
  { dist = 18 - sonar.ping_cm();
    Serial.print("Ping: ");
    Serial.print(dist); // Send ping, get distance in cm and print result (0 = outside set distance range)
    Serial.println("cm");
    delay(2000);
    if (dist >= 0 && dist <= 18)
      a[i] = dist;
    else
      a[i] = 0;

      i++;
  }
  dist = 0;
  for (i = 0; i < 3; i++)
  {
    dist += a[i];
  }
  dist /= 3;
  Serial.print("Val: ");
  Serial.print(dist);
  Serial.print(" cm");
    if (dist != prev)
    {
      prev = dist;
      String res = "";
      updateLevel(dist, &res);
      while (res != "")
      {
        if (res == "100")
          Serial.println("Success!!");
        else if (res == "1")
          Serial.println("Failed!!");
        res = "";
        break;
      }
    }

}

void findLocation()
{
  Serial.println("Fetching Location");
  float lat = LAT;
  float lng = LNG;
  Serial.println("Latitude:");
  Serial.println(lat);
  Serial.println("Longitude:");
  Serial.println(lng);
  String res = "";
  updateLocation(String(lat, 6) , String(lng, 6) , &res);
  while (res != "")
  {
    if (res == "100")
      Serial.println("Success!!");
    else if (res == "1")
      Serial.println("Failed!!");
    res = "";
    break;
  }

  //  while(true){
  //  while (gps_serial.available()) //While there are characters to come from the GPS
  //  {
  //    char x = gps_serial.read();
  //    Serial.print(x);
  //    gps.encode(x);//This feeds the serial NMEA data into the library one char at a time
  //  }
  //    if (gps.location.isUpdated()) //This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
  //    {
  //      //Get the latest info from the gps object which it derived from the data sent by the GPS unit
  //      long lat = gps.location.lat();
  //      long lng = gps.location.lng();
  //      Serial.println("Latitude:");
  //      Serial.println(lat, 6);
  //      Serial.println("Longitude:");
  //      Serial.println(lng, 6);
  //      String res = "";
  //      updateLocation(lat , lng , &res);
  //      while (res != "")
  //      {
  //        if (res == "100")
  //          Serial.println("Success!!");
  //        else if (res == "1")
  //          Serial.println("Failed!!");
  //        res = "";
  //        break;
  //      }
  //      break;
  //    }
  //
  //  }

}

void addBin(int level, String *loc)
{
  Serial.println("Sending request to server");
  String data = "function=ADD_BIN&level=" + level;
  postToServer(data, loc);
}

void updateLevel(int level, String *loc)
{
  if (binId != "")
  {
    Serial.println("Sending request to server");
    String data = "function=UPDT_LEV&binid=" + binId + "&level=" + level;
    postToServer(data, loc);
  }
  else
  {
    Serial.println("Sending request to server failed: No BIN ID");
  }
}

void updateLocation(String lati , String lngi , String *loc)
{
  if (binId != "")
  {
    Serial.println("Sending request to server");
    String data = "function=UPDT_LOC&binid=" + binId + "&lat=" + lati + "&lng=" + lngi;
    postToServer(data, loc);
  }
  else
  {
    Serial.println("Sending request to server failed: No BIN ID");
  }
}


void postToServer(String data, String *loc)
{
  String extracted = "" ;
  Serial.print("Connecting to ");
  Serial.print(server);

  if (!client.connect(server, port))
  {
    Serial.println(" fail");
    delay(10000);
    return "none";
  }

  Serial.println(" OK");

  client.print(String("POST ") + resource + " HTTP/1.1\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Content-Type: application/x-www-form-urlencoded\r\n");
  client.print("Content-Length:");
  client.println(data.length());
  client.println();
  client.println(data);

  //    //DEBUG
  //    Serial.println("**POST REQUEST**");
  //    Serial.print(String("POST ") + resource + " HTTP/1.1\r\n");
  //    Serial.print(String("Host: ") + server + "\r\n");
  //    Serial.print("Content-Type: application/x-www-form-urlencoded\r\n");
  //    Serial.print("Content-Length:");
  //    Serial.println(data.length());
  //    Serial.println();
  //    Serial.println(data);
  //    Serial.println();
  //    Serial.println("**POST RESPONSE**");

  String response = "";
  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 10000L && response == "") {
    // Print available data
    int index = 0;

    boolean isFound = false;

    while (client.available())
    {
      char c = client.read();
      //Serial.print(c); // DEBUG
      if ( c == '{')
      {
        isFound = true;
      }
      if (isFound)
      {
        response += (c);
      }
      if (c == '}')
      {
        isFound = false;
        break;
      }
      timeout = millis();
    }

    extracted = getResponse("response", response);
    Serial.print(extracted);
    *loc = extracted;
  }

  Serial.println();


  if (client.connected()) client.stop();
  if (!client.connected())
  {
    Serial.println();
    Serial.println("disconnected.");
    client.stop();
  }

}



String getResponse(String key, String response)
{
  //Serial.print(response);
  int fir = response.indexOf('"');
  int sec = response.indexOf('"', fir + 1);
  String val = response.substring(fir + 1, sec);
  if ( val == key)
  {
    fir = response.indexOf('"', sec + 1);
    sec = response.indexOf('"', fir + 1);
    val = response.substring(fir + 1, sec);
    return val;
  }
}

