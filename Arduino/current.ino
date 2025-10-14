/*

Project Name: Arc
Project Author: Nathaniel Mugenyi

*/



int check = 13;//indicator

//Current Sesning with Arduino

void setup() {
  

  Serial.begin(9600);

  pinMode(check, OUTPUT);

}

void loop() {


  unsigned int x=0;
  float AcsValue= 0.0, Samples= 0.0, AvgAcs=0.0, AcsValueF=0.0;

  for(int x = 0; x < 150; x++){
    AcsValue = analogRead(A0);
    Samples = Samples + AcsValue;

    delay(3);

  }
  
  AvgAcs=Samples/150.0;
  AcsValueF = (2.5 - (AvgAcs * (5.0 / 1024.0)))/0.100;

  Serial.print(AcsValueF);
  delay(50);

}
