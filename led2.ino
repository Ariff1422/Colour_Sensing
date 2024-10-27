// Define time delay before the next RGB colour turns ON to allow LDR to stabilize
#define RGBWait 200 //in milliseconds 

// Define time delay before taking another LDR reading
#define LDRWait 10 //in milliseconds 

#define LDR 0   //LDR sensor pin at A0
//#define LED 13  //Check Indicator to signal Calibration Completed

// Define colour sensor LED pin
//int ledArray[] = {2,3,4};

//placeholders for colour detected
int red = 0;
int green = 0;
int blue = 0;

//floats to hold colour arrays
float storedColors[5][3]; // 5 Colors, each with RGB values
float whiteArray[] = {0,0,0};
float blackArray[] = {0,0,0};
float greyDiff[] = {0,0,0};

char colourStr[3][5] = {"R = ", "G = ", "B = "};

void setup(){
  //setup the outputs for the colour sensor replaced by pinMode(A2, OUTPUT); and pinMode(A3, OUTPUT);
  /*for(int c = 0;c<=2;c++){
    pinMode(ledArray[c],OUTPUT);  
  }*/
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  //pinMode(LED,OUTPUT);   //Check Indicator -- OFF during Calibration

  //begin serial communication
  Serial.begin(9600);
  
  setBalance();  //calibration
  //digitalWrite(LED, HIGH); //Check Indicator -- ON after Calibration
}

void loop(){
  read_color(); // call this to get RGB values for 5 colors
}

void read_color() {

for (int colorI = 0; colorI < 5; i++) {
  Serial.print("Place Color ");
  Serial.print(i + 1);
  Serial.println(" for reading...");
  delay(5000);  // delay for five seconds for getting sample ready 
  
  Serial.print(colourStr[0]); //line 54 to 63 is for red
  digitalWrite(A2, HIGH);
  digitalWrite(A3, LOW);
  delay(RGBWait);
  storedColors[colorI][0] = getAvgReading(5);
  storedColors[colorI][0] = (storedColors[colorI][0] - blackArray[0]) / (greyDiff[0]) * 255;
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  delay(RGBWait);
  Serial.println(int(storedColors[colorI][0]));
  
  Serial.print(colourStr[1]); //line 65 to 74 is for green
  digitalWrite(A2, LOW);
  digitalWrite(A3, HIGH);
  delay(RGBWait);
  storedColors[colorI][1] = getAvgReading(5);
  storedColors[colorI][1] = (storedColors[colorI][1] - blackArray[1]) / (greyDiff[1]) * 255;
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  delay(RGBWait);
  Serial.println(int(storedColors[colorI][0]));
  
  Serial.print(colourStr[2]); //line 76 to 85 is for blue
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  delay(RGBWait);
  storedColors[colorI][2] = getAvgReading(5);
  storedColors[colorI][2] = (storedColors[colorI][2] - blackArray[2]) / (greyDiff[2]) * 255;
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  delay(RGBWait);
  Serial.println(int(storedColors[colorI][2]));
}


void setBalance(){
//set white balance
  Serial.println("Put White Sample For Calibration ...");
  delay(5000);           //delay for five seconds for getting sample ready
  //digitalWrite(LED,LOW); //Check Indicator OFF during Calibration
//scan the white sample.
//go through one colour at a time, set the maximum reading for each colour -- red, green and blue to the white array
  digitalWrite(A2, HIGH); //on red light
  digitalWrite(A3, LOW);
  delay(RGBWait);
  whiteArray[0] = getAvgReading(5);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  delay(RGBWait);
  digitalWrite(A2, LOW); //on green light
  digitalWrite(A3, HIGH);
  delay(RGBWait);
  whiteArray[1] = getAvgReading(5);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  delay(RGBWait);
  digitalWrite(A2, HIGH); // on blue light
  digitalWrite(A3, HIGH);
  delay(RGBWait);
  whiteArray[2] = getAvgReading(5);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  delay(RGBWait);



  /*for(int i = 0;i<=2;i++){ // replaced by lines 96 to 116
     digitalWrite(ledArray[i],HIGH);
     delay(RGBWait);
     whiteArray[i] = getAvgReading(5);         //scan 5 times and return the average, 
     digitalWrite(ledArray[i],LOW);
     delay(RGBWait);
  }*/
//done scanning white, time for the black sample.
//set black balance
  Serial.println("Put Black Sample For Calibration ...");
  delay(5000);     //delay for five seconds for getting sample ready 
//go through one colour at a time, set the minimum reading for red, green and blue to the black array
  digitalWrite(A2, HIGH); //on red light
  digitalWrite(A3, LOW);
  delay(RGBWait);
  blackArray[0] = getAvgReading(5);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  delay(RGBWait);
  digitalWrite(A2, LOW); //on green light
  digitalWrite(A3, HIGH);
  delay(RGBWait);
  blackArray[1] = getAvgReading(5);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  delay(RGBWait);
  digitalWrite(A2, HIGH); // on blue light
  digitalWrite(A3, HIGH);
  delay(RGBWait);
  blackArray[2] = getAvgReading(5);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  delay(RGBWait);
  for(int i = 0;i<=2;i++){
     /*digitalWrite(ledArray[i],HIGH);
     delay(RGBWait);
     blackArray[i] = getAvgReading(5);
     digitalWrite(ledArray[i],LOW);
     delay(RGBWait);*/
//the differnce between the maximum and the minimum gives the range
     greyDiff[i] = whiteArray[i] - blackArray[i];
     
     Serial.println(int(greyDiff[i]));
  }

//delay another 5 seconds for getting ready colour objects
  Serial.println("Colour Sensor Is Ready.");
  delay(5000);
  }


int getAvgReading(int times){      
//find the average reading for the requested number of times of scanning LDR
  int reading;
  int total =0;
//take the reading as many times as requested and add them up
  for(int i = 0;i < times;i++){
     reading = analogRead(LDR);
     total = reading + total;
     delay(LDRWait);
  }
//calculate the average and return it
  return total/times;
}
