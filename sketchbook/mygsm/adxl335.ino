
//Analog read pins
const int xPin = 3;
const int yPin = 2;
const int zPin = 1;

//The minimum and maximum values that came from
//the accelerometer while standing still
//You very well may need to change these
int minVal = 265;
int maxVal = 402;


//to hold the caculated values
double x;
double y;
double z;

int xAng=0,yAng=0,zAng=0;

int ADXL335xAng(){return xAng;}
int ADXL335yAng(){return yAng;}

void ADXL335read(){

  //read the analog values from the accelerometer
  int xRead = analogRead(xPin);
  int yRead = analogRead(yPin);
  int zRead = analogRead(zPin);

  //convert read values to degrees -90 to 90 - Needed for atan2
  xAng = map(xRead, minVal, maxVal, -90, 90);
  yAng = map(yRead, minVal, maxVal, -90, 90);
  zAng = map(zRead, minVal, maxVal, -90, 90);

  //Caculate 360deg values like so: atan2(-yAng, -zAng)
  //atan2 outputs the value of -π to π (radians)
  //We are then converting the radians to degrees
  double x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  double y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  double z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);

}
