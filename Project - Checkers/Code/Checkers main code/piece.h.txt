#include <string.h>
using namespace std;


int blacks[12][3], whites[12][3], kings[18][3];


void set_pieces(){

  int k = 0;
  for(int i=0; i<3; i++){
    for(int j=0; j<8; j++){
      if((i+j)%2==1){
        blacks[k][0] = i;
        blacks[k][1] = j;
        blacks[k][2] = 1;
        k+=1;
      }
    }
  }

  k = 0;
  for(int i=5; i<8; i++){
    for(int j=0; j<8; j++){
      if((i+j)%2==1){
        whites[k][0] = i;
        whites[k][1] = j;
        whites[k][2] = 1;
        k+=1;
      }
    }
  }
  
  /* TEST KING MOVES
  kings[0][0] = 4;
  kings[0][1] = 3;
  kings[0][2] = 3;
  startup[4][3] = 3;
  startup[5][2] = 0;
  */
}


void show_pieces(){
  Serial.println("SHOW");
  for(int i = 0; i<12; i++){
    for(int j = 0; j<3; j++){
      Serial.print(blacks[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }

  for(int i = 0; i<12; i++){
    for(int j = 0; j<3; j++){
      Serial.print(whites[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
}
