#include <FastLED.h>
#define NUM_LEDS 32
#define LED_PIN 12

#include "board.h"
#include "piece.h"
#include <string.h>

CRGB leds[NUM_LEDS];

#define NUMBER_OF_SHIFT_CHIPS 4

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC 5

/* Optional delay between shift register reads.
*/
#define POLL_DELAY_MSEC 1

/* You will need to change the "int" to "long" If the
 * NUMBER_OF_SHIFT_CHIPS is higher than 2.
*/
#define BYTES_VAL_T unsigned long

int ploadPin = 11;       // Connects to Parallel load pin the 165
int clockEnablePin = 8;  // Connects to Clock Enable pin the 165
int dataPin = 9;         // Connects to the Q7 pin the 165
int clockPin = 10;       // Connects to the Clock pin the 165


BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;

int prev_turn = 0;
int cur_move = 1;  // White = 1 and Black = 0
                   // i.e, Current move first move is of White

int chosen_piece[] = { -1, -1 };
int chosen_piece_moves_rows[16], chosen_piece_moves_cols[16], chosen_cut_piece_moves_rows[16], chosen_cut_piece_moves_cols[16];
int total_moves = 0, total_cut_moves = 0;


// Convert sensor pin number to LED number and sensor number
int mapI(int i) { 
  int j;
  if (i > 23) {
    j = 55 - i;
    return j;
  } else if (i > 15) {
    j = 39 - i;
    return j;
  } else if (i > 7) {
    j = 23 - i;
    return j;
  } else {
    j = 7 - i;
    return j;
  }
}

int map_led_to_pin(int led){
  int pin;

}

BYTES_VAL_T read_shift_regs() {
  long bitVal;
  BYTES_VAL_T bytesVal = 0;

  /* Trigger a parallel Load to latch the state of the data lines,
    */
  digitalWrite(clockEnablePin, HIGH);
  digitalWrite(ploadPin, LOW);
  delayMicroseconds(PULSE_WIDTH_USEC);
  digitalWrite(ploadPin, HIGH);
  digitalWrite(clockEnablePin, LOW);

  /* Loop to read each bit value from the serial out line
     * of the SN74HC165N.
    */
  for (int i = 0; i < DATA_WIDTH; i++) {
    bitVal = digitalRead(dataPin);
    // Serial.print(i);
    // Serial.print(" ");
    // Serial.println(bitVal);

    // delay(10);

    /* Set the corresponding bit in bytesVal.
        */
    bytesVal |= (bitVal << i);
    //bytesVal |= (bitVal << i);

    /* Pulse the Clock (rising edge shifts the next bit).
        */
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(clockPin, LOW);
  }

  return (bytesVal);
}

// CONVERT PIN NUMBER TO ROW AND COLUMN NUMBER

int* conv_pin_to_rnc(int pin_no){
  int* rc = new int[2];
  int x,y;
  x = pin_no/4;
  rc[0] = 7 - x;
  y =  pin_no%4;
  if (rc[0]%2==1){
    rc[1] = 2*y;
  }
  else{
    rc[1] = 2*y+1;
  }
  return rc;
}


/*
CONVERT ROW AND COL (COORDINATES) INTO LED NUMBER
*/
int row_col_to_LED(int row, int col){
  int x, y;
  x = 7 - row;
  if (row%2==1){
    y = 4 - col/2;
  }
  else{
    y = (col-1)/2;
  }
  return (4*x)+y;
}



/* Dump the list of zones along with their current status.
*/

void display_pin_values() {
  // Serial.print("Pin States:\r\n");

  for (int i = DATA_WIDTH - 1; i >= 0; i--) {

    // Serial.print("  Pin-");
    int my_ref_val = mapI(i);
    // Serial.print(my_ref_val);
    // Serial.print(": ");

    int row, col;
    int* rc = conv_pin_to_rnc(my_ref_val);
    row = rc[0];
    col = rc[1];
    delete[] rc;
    // Serial.print(row);
    // Serial.print(",");
    // Serial.print(col);
    // Serial.print(" : ");
    
    
    int r, c;
    if ((my_ref_val / 4) % 2 == 1) {
      r = (my_ref_val / 4);
      c = 3 - (my_ref_val % 4);
      my_ref_val = r * 4 + c;
    }

    if ((pinValues >> i) & 1) {  
      curr_state[row][col] = 1;
      // Serial.print("HIGH");
      if(cur_move%2==0){
        leds[my_ref_val] = CRGB(255, 0, 0);
      }
      else{
        leds[my_ref_val] = CRGB(0, 0, 255);      
      }
      FastLED.show();

      //delay(1000);
    } else {
      curr_state[row][col] = 0;
      leds[my_ref_val] = CRGB(0,0,0);
      // Serial.print("LOW");
      FastLED.show();
    }

    // Serial.print("\r\n");
  }

  // Serial.print("\r\n");
}



bool hasPiece(int i) {
  unsigned long x = 1;
  return (pinValues & (x<<i));
}


void wrong_move(int row, int col){
  Serial.println("WRRRRRRRRRRRRONG");
  // int* rc = conv_pin_to_rnc(pno);
  int led_no = LED_ref[row][col];
  leds[led_no] = CRGB(102, 0, 102);
  FastLED.show();
  
}

// LED GLOW CODE
void LED_Glow(int led_no, int R, int G, int B){
  leds[led_no] = CRGB(R, G, B);
  FastLED.show();
}

// GLOW LEDs USING COORDINATES
void glow_Leds_using_coords(){
  int led_no;
  // WHITE PIECE TURN, GLOW LED IN RED COLOR
  // BLACK PIECE TURN, GLOW LED IN BLUE COLOR
  if (total_cut_moves == 0){
    for(int i = 0; i<total_moves; i++){
      led_no = LED_ref[chosen_piece_moves_rows[i]][chosen_piece_moves_cols[i]];
//      led_no = row_col_to_LED(chosen_piece_moves_rows[i], chosen_piece_moves_cols[i]);
      if (cur_move % 2 == 1)    // WHITE TURN
        LED_Glow(led_no, 255, 0, 0);
      else                      // BLACK TURN
        LED_Glow(led_no, 0, 0, 255);
    }
  }
  else{
    for(int i = 0; i<total_cut_moves; i++){
      led_no = LED_ref[chosen_cut_piece_moves_rows[i]][chosen_cut_piece_moves_cols[i]];
//      led_no = row_col_to_LED(chosen_cut_piece_moves_rows[i], chosen_cut_piece_moves_cols[i]);
      if (cur_move % 2 == 1)    // WHITE TURN
        LED_Glow(led_no, 255, 0, 0);
      else                      // BLACK TURN
        LED_Glow(led_no, 0, 0, 255);
    }
  }
}


/* SHOWS POSSIBLE PIECES U CAN MOVE FOR EACH PLAYER.
*/
void possible_choices() {
  Serial.print("possible Choice -\t");
  Serial.println(cur_move);

  if (cur_move % 2 == 1) {
    for (int i = 0; i < white_left; i++) {
      is_possible_move(whites[i][0], whites[i][1], 3);
    }
  } else {
    for (int i = 0; i < black_left; i++) {
      is_possible_move(blacks[i][0], blacks[i][1], 2);
    }
  }

  for(int i=0; i<total_moves; i++){
    Serial.print(chosen_piece_moves_rows[i]);
    Serial.print(',');
    Serial.println(chosen_piece_moves_cols[i]);
  }

  glow_Leds_using_coords();

}



// CHECK IF THE PASSED 'ROW, COL' PIECE HAS A POSSIBLE MOVE, AIDS THE 'possible_choices()' FUNCTION

void is_possible_move(int row, int col, int color) {
  int king_is_the = is_King(row, col, color);
  if (color == 2 || king_is_the>-1) {
    if (7 > row) {
      if (0 < col) {
        if (startup[row + 1][col - 1] == 0) {
          chosen_piece_moves_rows[total_moves] = row;
          chosen_piece_moves_cols[total_moves] = col;
          total_moves += 1;
        }
        if (startup[row + 1][col - 1] == 3 && (7 > row + 1)) {
          if (0 < col - 1) {
            if (startup[row + 2][col - 2] == 0) {
              chosen_cut_piece_moves_rows[total_cut_moves] = row;
              chosen_cut_piece_moves_cols[total_cut_moves] = col;
              total_cut_moves += 1;
              return;
            }
          }
        }
      }
      if (col < 7) {
        if (startup[row + 1][col + 1] == 0) {
          chosen_piece_moves_rows[total_moves] = row;
          chosen_piece_moves_cols[total_moves] = col;
          total_moves += 1;

        }
        if (startup[row + 1][col + 1] == 3 && (7 > row + 1)) {
          if (col + 1 < 7) {
            if (startup[row + 2][col + 2] == 0) {
              chosen_cut_piece_moves_rows[total_cut_moves] = row;
              chosen_cut_piece_moves_cols[total_cut_moves] = col;
              total_cut_moves += 1;
              return;
            }
          }
        }
      }
    }
  }
  if (color == 3 || king_is_the>-1) {
    if (0 < row) {
      if (0 < col) {
        if (startup[row - 1][col - 1] == 0) {
          chosen_piece_moves_rows[total_moves] = row;
          chosen_piece_moves_cols[total_moves] = col;
          total_moves += 1;
        }
        if (startup[row - 1][col - 1] == 2 && (0 < row - 1)) {
          if (0 < col - 1) {
            if (startup[row - 2][col - 2] == 0) {
              chosen_cut_piece_moves_rows[total_cut_moves] = row;
              chosen_cut_piece_moves_cols[total_cut_moves] = col;
              total_cut_moves += 1;
              return;
            }
          }
        }
      }
      if (col < 7) {
        if (startup[row - 1][col + 1] == 0) {
          chosen_piece_moves_rows[total_moves] = row;
          chosen_piece_moves_cols[total_moves] = col;
          total_moves += 1;
        }
        if (startup[row - 1][col + 1] == 2 && (0 < row - 1)) {
          if (col + 1 < 7) {
            if (startup[row - 2][col + 2] == 0) {
              chosen_cut_piece_moves_rows[total_cut_moves] = row;
              chosen_cut_piece_moves_cols[total_cut_moves] = col;
              total_cut_moves += 1;
              return;
            }
          }
        }
      }
    }
  }
}



// SELECTS THE PASSED PIECE AT 'ROW, COL' AND MAKES IT 'chosen_piece' AND THEN FINDS ITS POSSIBLE MOVES AND SHOWS IT

void cur_row_col(int row, int col, int pno) {
  Serial.print("Cur row col -");
  Serial.print(row);
  Serial.print('\t');
  Serial.print(col);
  Serial.print("\t");
  Serial.print(pno);
  int led_no;
  if (startup[row][col] == -1 || startup[row][col] == 0) {
    Serial.println("Wrong Cell");
    chosen_piece[0] = -1;
    chosen_piece[1] = -1;
  } else {
    if ((cur_move % 2 == 1 && startup[row][col] == 3) || (cur_move % 2 == 0 && startup[row][col] == 2)) {  // startup matrix, white = 3 and black = 2
      chosen_piece[0] = row;
      chosen_piece[1] = col;
      led_no = LED_ref[chosen_piece[0]][chosen_piece[1]];
      // led_no = row_col_to_LED(chosen_piece[0], chosen_piece[1]);
      LED_Glow(led_no, 0, 0, 0);
      find_piece_moves(row, col, startup[row][col]);
      Serial.println("Chosen Piece Move");
      show_chosen_moves();

      //find_moves(row, col);
      //update_board();
    } else {
      // wrong_move(row, col);
      chosen_piece[0] = -1;
      chosen_piece[1] = -1;
    }
  }
}

// CHECKS IF THE PASSED PIECE AT 'ROW, COL' IS A KING

int is_King(int row, int col, int color) {
  Serial.println("KING");
  Serial.println(row);
  Serial.println(col);
  Serial.println(color);
  int ind = -1;
  for (int i = 0; i < kings_made; i++) {
    Serial.println("CHECK");
    Serial.print(row);
    Serial.print(" , ");
    Serial.print(col);
    Serial.print(" , ");
    Serial.println(color);
    
    
    if (kings[i][0] == row && kings[i][1] == col && kings[i][2] == color) {
      ind = i;
      break;
    }
  }
  return ind;
}


void show_kings(){
  Serial.println("KINGSSSS");
  for(int i = 0; i<kings_made; i++){
    Serial.print("King #");
    Serial.print(i);
    Serial.print(" , ");
    Serial.print(kings[i][0]);
    Serial.print(" , ");
    Serial.print(kings[i][1]);
    Serial.print(" , ");
    Serial.println(kings[i][2]);


  }
}

// FINDS THE ALL NEXT MOVES THE PASSED PIECE AT 'ROW, COL' CAN MOVE

void find_piece_moves(int row, int col, int color) {
  Serial.println("FINDING MOVES OF ");
  Serial.print(row);
  Serial.print(" ");
  Serial.print(col);
  Serial.print(" ");
  Serial.print(color);


  total_moves = 0;
  total_cut_moves = 0;
  int king_is_the = is_King(chosen_piece[0], chosen_piece[1], startup[chosen_piece[0]][chosen_piece[1]]);
  show_kings();
  Serial.println(king_is_the);
  if(king_is_the>-1){
    Serial.println("THIS IS A KING");
  }
  if (color == 2) {
    if (7 > row) {
      if (0 < col) {
        if (startup[row + 1][col - 1] == 0) {
          chosen_piece_moves_rows[total_moves] = row + 1;
          chosen_piece_moves_cols[total_moves] = col - 1;
          total_moves += 1;
        } else if (startup[row + 1][col - 1] == 3 && (7 > row + 1)) {
          if (0 < col - 1) {
            if (startup[row + 2][col - 2] == 0){
              chosen_cut_piece_moves_rows[total_cut_moves] = row + 2;
              chosen_cut_piece_moves_cols[total_cut_moves] = col - 2;
              total_cut_moves += 1;
            }
          }
        }
      }
      if (col < 7) {
        if (startup[row + 1][col + 1] == 0) {
          chosen_piece_moves_rows[total_moves] = row + 1;
          chosen_piece_moves_cols[total_moves] = col + 1;
          total_moves += 1;
        } else if (startup[row + 1][col + 1] == 3 && (7 > row + 1)) {
          if (col + 1 < 7) {
            if (startup[row + 2][col + 2] == 0) {
              chosen_cut_piece_moves_rows[total_cut_moves] = row + 2;
              chosen_cut_piece_moves_cols[total_cut_moves] = col + 2;
              total_cut_moves += 1;
            }
          }
        }
      }
    }
  }
  if (color == 3) {
    if (0 < row) {
      if (0 < col) {
        if (startup[row - 1][col - 1] == 0) {
          chosen_piece_moves_rows[total_moves] = row - 1;
          chosen_piece_moves_cols[total_moves] = col - 1;
          total_moves += 1;
        } else if (startup[row - 1][col - 1] == 2 && (0 < row - 1)) {
          if (0 < col - 1) {
            if (startup[row - 2][col - 2] == 0) {
              chosen_cut_piece_moves_rows[total_cut_moves] = row - 2;
              chosen_cut_piece_moves_cols[total_cut_moves] = col - 2;
              total_cut_moves += 1;
            }
          }
        }
      }
      if (col < 7) {
        if (startup[row - 1][col + 1] == 0) {
          chosen_piece_moves_rows[total_moves] = row - 1;
          chosen_piece_moves_cols[total_moves] = col + 1;
          total_moves += 1;
        } else if (startup[row - 1][col + 1] == 2 && (0 < row - 1)) {
          if (col + 1 < 7) {
            if (startup[row - 2][col + 2] == 0) {
              chosen_cut_piece_moves_rows[total_cut_moves] = row - 2;
              chosen_cut_piece_moves_cols[total_cut_moves] = col + 2;
              total_cut_moves += 1;
            }
          }
        }
      }
    }
  }
  if(king_is_the>-1){
    if (color == 2){
      if (0 < row) {
        if (0 < col) {
          if (startup[row - 1][col - 1] == 0) {
            chosen_piece_moves_rows[total_moves] = row - 1;
            chosen_piece_moves_cols[total_moves] = col - 1;
            total_moves += 1;
          } else if (startup[row - 1][col - 1] == 3 && (0 < row - 1)) {
            if (0 < col - 1) {
              if (startup[row - 2][col - 2] == 0) {
                chosen_cut_piece_moves_rows[total_cut_moves] = row - 2;
                chosen_cut_piece_moves_cols[total_cut_moves] = col - 2;
                total_cut_moves += 1;
              }
            }
          }
        }
        if (col < 7) {
          if (startup[row - 1][col + 1] == 0) {
            chosen_piece_moves_rows[total_moves] = row - 1;
            chosen_piece_moves_cols[total_moves] = col + 1;
            total_moves += 1;
          } else if (startup[row - 1][col + 1] == 3 && (0 < row - 1)) {
            if (col + 1 < 7) {
              if (startup[row - 2][col + 2] == 0) {
                chosen_cut_piece_moves_rows[total_cut_moves] = row - 2;
                chosen_cut_piece_moves_cols[total_cut_moves] = col + 2;
                total_cut_moves += 1;
              }
            }
          }
        }
      }
    }
    if(color==3){
      if (7 > row) {
        if (0 < col) {
          if (startup[row + 1][col - 1] == 0) {
            chosen_piece_moves_rows[total_moves] = row + 1;
            chosen_piece_moves_cols[total_moves] = col - 1;
            total_moves += 1;
          } else if (startup[row + 1][col - 1] == 2 && (7 > row + 1)) {
            if (0 < col - 1) {
              if (startup[row + 2][col - 2] == 0){
                chosen_cut_piece_moves_rows[total_cut_moves] = row + 2;
                chosen_cut_piece_moves_cols[total_cut_moves] = col - 2;
                total_cut_moves += 1;
              }
            }
          }
        }
        if (col < 7) {
          if (startup[row + 1][col + 1] == 0) {
            chosen_piece_moves_rows[total_moves] = row + 1;
            chosen_piece_moves_cols[total_moves] = col + 1;
            total_moves += 1;
          } else if (startup[row + 1][col + 1] == 2 && (7 > row + 1)) {
            if (col + 1 < 7) {
              if (startup[row + 2][col + 2] == 0) {
                chosen_cut_piece_moves_rows[total_cut_moves] = row + 2;
                chosen_cut_piece_moves_cols[total_cut_moves] = col + 2;
                total_cut_moves += 1;
              }
            }
          }
        }
      }
    }
  }
}

// PRINT THE ROW COL OF PRESENT PIECES IN THE ARRAYS

void show_chosen_moves() {
  int led_no;
  if (total_cut_moves > 0) {
    for (int i = 0; i < total_cut_moves; i++) {
      led_no = LED_ref[chosen_cut_piece_moves_rows[i]][chosen_cut_piece_moves_cols[i]];
      // led_no = row_col_to_LED(chosen_cut_piece_moves_rows[i], chosen_cut_piece_moves_cols[i]);
      LED_Glow(led_no, 0, 255, 0);
      Serial.print(chosen_cut_piece_moves_rows[i]);
      Serial.print(" , ");
      Serial.println(chosen_cut_piece_moves_cols[i]);
      
    }
  } else {
    for (int i = 0; i < total_moves; i++) {
      led_no = LED_ref[chosen_piece_moves_rows[i]][chosen_piece_moves_cols[i]];
      // led_no = row_col_to_LED(chosen_piece_moves_rows[i], chosen_piece_moves_cols[i]);
      LED_Glow(led_no, 0, 255, 0);
      Serial.print(chosen_piece_moves_rows[i]);
      Serial.print(" , ");
      Serial.println(chosen_piece_moves_cols[i]);
    }
  }
}

void update_blacks_whites() {
  Serial.println("BLACKS, WHITES");
  int bc = 0, wc = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (startup[i][j] == 2) {
        blacks[bc][0] = i;
        blacks[bc][1] = j;
        blacks[bc][2] = 1;
        bc += 1;
      }
      if (startup[i][j] == 3) {
        whites[wc][0] = i;
        whites[wc][1] = j;
        whites[wc][2] = 1;
        wc += 1;
      }
    }
  }
  black_left = bc;
  white_left = wc;
}

void update_kings(int row, int col) {
  int i;
  for (i = 0; i < kings_made; i++) {
    if (kings[i][0] == row && kings[i][1] == col and startup[row][col] == kings[i][2]) {
      break;
    }
  }
  if (i != kings_made) {
    if (0 < i < kings_made - 1) {
      kings[i][0] = kings[kings_made - 1][0];
      kings[i][1] = kings[kings_made - 1][1];
      kings[i][2] = kings[kings_made - 1][2];
    }
    kings_made -= 1;
  }
}



void show_matrix() {
  int led_no;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      led_no = LED_ref[i][j];
      // led_no = row_col_to_LED(i,j);
      // if(startup[i][j]==2){
      //   leds[led_no] = CRGB(255, 0, 0);
      // }
      // if(startup[i][j]==3){
      //   leds[led_no] = CRGB(0, 0, 255);
      // }
      Serial.print(startup[i][j]);
      Serial.print(" ");
      //FastLED.show();
    }
    Serial.println();
  }
}

void show_state(){
  Serial.println("CURR");
  for(int i = 0;i<8; i++){
    for(int j = 0; j<8; j++){
      Serial.print(curr_state[i][j]);
      Serial.print(", ");
    }
    Serial.println();
  }

  Serial.println("Prev");
  for(int i = 0;i<8; i++){
    for(int j = 0; j<8; j++){
      Serial.print(prev_state[i][j]);
      Serial.print(", ");
    }
    Serial.println();
  }
}

/*
void move(int row, int col) {
  Serial.print("CURRENT PIECE - ");
  Serial.print(chosen_piece[0]);
  Serial.print(" ");
  Serial.println(chosen_piece[1]);

  if (chosen_piece[0] == -1)
    return;

  int i;
  if (total_cut_moves > 0) {
    for (i = 0; i < total_cut_moves; i++) {
      if (chosen_cut_piece_moves_rows[i] == row and chosen_cut_piece_moves_cols[i] == col) {
        break;
      }
    }
    if (i == total_cut_moves) {
      Serial.println("Wrong move");
      return;
    }
    startup[row][col] = startup[chosen_piece[0]][chosen_piece[1]];
    startup[chosen_piece[0]][chosen_piece[1]] = 0;
    int cut_row, cut_col;
    if (row - chosen_piece[0] == -2) {
      cut_row = row + 1;
    } else {
      cut_row = row - 1;
    }

    if (col - chosen_piece[1] == -2) {
      cut_col = col + 1;
    } else {
      cut_col = col - 1;
    }

    Serial.print("CUT PIECE COOR - ");
    Serial.print(cut_row);
    Serial.print(" ");
    Serial.println(cut_col);

    update_kings(cut_row, cut_col);

    startup[cut_row][cut_col] = 0;

    if ((startup[row][col] == 3 and row == 0) || (startup[row][col] == 2 and row == 7)) {
      kings[kings_made][0] = row;
      kings[kings_made][1] = col;
      kings[kings_made][2] = startup[row][col];
      kings_made += 1;
    }
    update_blacks_whites();

    chosen_piece[0] = row;
    chosen_piece[1] = col;
    find_piece_moves(chosen_piece[0], chosen_piece[1], startup[chosen_piece[0]][chosen_piece[1]]);
    if (total_moves == 0) {
      Serial.println("TOTAL MOVES ZERO");
      cur_move += 1;
      chosen_piece[0] = -1;
      chosen_piece[1] = -1;

    } else {
      show_chosen_moves();
      /*
      CODE FOR "MOVE" TO NEXT POSITION
      
      Serial.println("NEXT MOVES");
    }

  }

  else {

    for (i = 0; i < total_moves; i++) {
      if (chosen_piece_moves_rows[i] == row and chosen_piece_moves_cols[i] == col) {
        break;
      }
    }
    if (i == total_moves) {
      Serial.println("Wrong move");
      return;
    } else {
      startup[row][col] = startup[chosen_piece[0]][chosen_piece[1]];
      startup[chosen_piece[0]][chosen_piece[1]] = 0;
      Serial.println("No Pieces Cut");
      cur_move += 1;
      if ((startup[row][col] == 3 && row == 0) || (startup[row][col] == 2 && row == 7)) {
        kings[kings_made][0] = row;
        kings[kings_made][1] = col;
        kings[kings_made][2] = startup[row][col];
        kings_made += 1;
      }
      update_blacks_whites();
      /*
      if((row-chosen_piece[0]==1 || row-chosen_piece[0]==-1) && (col-chosen_piece[1]==1 || col-chosen_piece[1]==-1)){
        Serial.println("No Pieces Cut");
        cur_move+=1;
        if((startup[row][col] == 3 && row==0)||(startup[row][col] == 2 && row==7)){
          kings[kings_made][0] = row;
          kings[kings_made][1] = col;
          kings[kings_made][2] = startup[row][col];
          kings_made+=1;
        }
        update_blacks_whites(); 
    }
  }
}
*/

void validate_move(int row, int col){

  int king_ind = is_King(chosen_piece[0], chosen_piece[1], startup[chosen_piece[0]][chosen_piece[1]]);
  
  if (total_cut_moves > 0) {
    int cut_row, cut_col;
    for (int i = 0; i < total_cut_moves; i++) {
      int r = chosen_cut_piece_moves_rows[i];
      int c = chosen_cut_piece_moves_cols[i];
      if(curr_state[r][c]==1 && prev_state[r][c]==0 && row==r && col==c){
        prev_turn = cur_move;
        if(c>chosen_piece[1]){
          cut_col = chosen_piece[1]+1;
          if(r>chosen_piece[0]){
            cut_row = chosen_piece[0]+1;
          }
          else{
            cut_row = chosen_piece[0]-1;
          }
        }
        else{
          cut_col = chosen_piece[1]-1;
          if(r>chosen_piece[0]){
            cut_row = chosen_piece[0]+1;
          }
          else{
            cut_row = chosen_piece[0]-1;
          }
        }

        startup[r][c] = startup[chosen_piece[0]][chosen_piece[1]];
        startup[chosen_piece[0]][chosen_piece[1]] = 0;
        prev_state[chosen_piece[0]][chosen_piece[1]] = curr_state[chosen_piece[0]][chosen_piece[1]];
        prev_state[r][c] = curr_state[r][c];
        if (king_ind>-1){
          kings[king_ind][0] = row;
          kings[king_ind][1] = col;
        }
        

        update_kings(cut_row, cut_col);

        startup[cut_row][cut_col] = 0;
        int led_no = LED_ref[cut_row][cut_col];
        leds[led_no] = CRGB(120, 144, 255);
        FastLED.show();

        
        Serial.println("MATRIX CUT");
        show_matrix();
        prev_state[cut_row][cut_col] = curr_state[cut_row][cut_col];
        
        if (king_ind==-1 && ((startup[r][c] == 3 and r == 0) || (startup[r][c] == 2 and r == 7))) {
          kings[kings_made][0] = r;
          kings[kings_made][1] = c;
          kings[kings_made][2] = startup[r][c];
          kings_made += 1;
        }
        update_blacks_whites();
        chosen_piece[0] = r;
        chosen_piece[1] = c;
        find_piece_moves(chosen_piece[0], chosen_piece[1], startup[chosen_piece[0]][chosen_piece[1]]);
        if (total_cut_moves>0) {
          show_chosen_moves();
          /*
          CODE FOR "MOVE" TO NEXT POSITION
          */
          Serial.println("NEXT MOVES");

        } else{
          Serial.println("TOTAL CUT MOVES ZERO");
          cur_move += 1;
          chosen_piece[0] = -1;
          chosen_piece[1] = -1;
          
        }

    
        Serial.println("Correct move");
        return;
      }
    }
  }
  else{
    for (int i = 0; i < total_moves; i++) {
      int r = chosen_piece_moves_rows[i];
      int c = chosen_piece_moves_cols[i];
      if(row==r && col==c && chosen_piece[0]!=row && chosen_piece[1]!=col){
        prev_turn = cur_move;
        Serial.print("LOL");
        cur_move+=1;
        if (chosen_piece[0]!=-1 && chosen_piece[1]!=-1){
          startup[r][c] = startup[chosen_piece[0]][chosen_piece[1]];
          startup[chosen_piece[0]][chosen_piece[1]] = 0;
        }

        if (king_ind>-1){
          kings[king_ind][0] = row;
          kings[king_ind][1] = col;
        }
        
        chosen_piece[0] = -1;
        chosen_piece[1] = -1;
        if (king_ind==-1 && ((startup[r][c] == 3 && r == 0) || (startup[r][c] == 2 && r == 7))) {
          kings[kings_made][0] = r;
          kings[kings_made][1] = c;
          kings[kings_made][2] = startup[r][c];
          kings_made += 1;
        }
        update_blacks_whites();
        Serial.println("MATRIX");
        show_matrix();
        Serial.println("Correct move");
        Serial.println(cur_move);
        int led_no;
        for(int x = 0; x<8; x++){
          for(int y = 0; y<8; y++){
            if(curr_state[x][y]==1){
              led_no = LED_ref[x][y];
              if(cur_move%2==0){
                leds[led_no] = CRGB(255, 0, 0);
              }
              else{
                leds[led_no] = CRGB(0, 0, 255);      
              }
              FastLED.show();
            }
          }
        }
        return;
      }
    }
  }

  // wrong_move(row, col);
  chosen_piece[0] = -1;
  chosen_piece[1] = -1;
  Serial.println("WRONG MOVE");

}


void setup() {
  Serial.begin(9600);
  /* Initialize our digital pins...
    */
  pinMode(ploadPin, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);

  digitalWrite(clockPin, LOW);
  digitalWrite(ploadPin, HIGH);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);

  FastLED.clear();

  /* Read in and display the pin states at startup.
    
  pinValues = read_shift_regs();
  display_pin_values();
  oldPinValues = pinValues;
*/
  
  /*
  Serial.println("Start");
  show_matrix();

  //int row, col;
  int row, col, v;
  v = 31;
  int* rc = conv_pin_to_rnc(v);
  row = rc[0];
  col = rc[1];
  delete[] rc;
  Serial.print(v);
  Serial.print(" : ");
  Serial.print(row);
  Serial.print(",");
  Serial.println(col);

  set_pieces();
  //show_pieces();
  possible_choices();
  Serial.print("Possible CHoices");
  Serial.println((String)total_moves + " " + (String)total_cut_moves);
  show_chosen_moves();
  cur_row_col(5, 2);
  move(4, 3);
  show_matrix();
  //show_pieces();
  possible_choices();
  Serial.print("Possible CHoices");
  Serial.println((String)total_moves + " " + (String)total_cut_moves);
  show_chosen_moves();
  cur_row_col(2, 5);
  move(3, 4);
  show_matrix();
  possible_choices();
  Serial.print("Possible CHoices");
  Serial.println((String)total_moves + " " + (String)total_cut_moves);
  show_chosen_moves();
  show_pieces();
  cur_row_col(4, 2);
  move(2, 4);
  show_matrix();
  possible_choices();
  Serial.print("Possible CHoices");
  Serial.println((String)total_moves + " " + (String)total_cut_moves);
  show_chosen_moves();

  */
  /*cur_row_col(2, 5);
  cur_row_col(4, 3);*/
  show_matrix();
  set_pieces();  
  // possible_choices();
  display_pin_values();
//  detect_state_change();
  // put your setup code here, to run once:
}


int* compare_states(){
  int* rc = new int[2];
  rc[0] = -1;
  rc[1] = -1;
  for(int i=0; i<8; i++){
    for(int j=0;j<8; j++){
      if(curr_state[i][j]!=prev_state[i][j]){
        rc[0] = i;
        rc[1] = j;
        break;
      }
    }
  }
  return rc;
}

void loop() {
  // put your main code here, to run repeatedly:
  /* Read the state of all zones.
    */
  
  
  

  /* If there was a chage in state, display which ones changed.
    */
    /*
  if (pinValues != oldPinValues) {
    Serial.print("*Pin value change detected*\r\n");
    display_pin_values();
    oldPinValues = pinValues;
  }

  delay(POLL_DELAY_MSEC); */

  // Serial.println(pinValues);
  // Serial.println(oldPinValues);

  
  pinValues = read_shift_regs();

  // if (prev_turn != cur_move){
  //   possible_choices();
  //   prev_turn = cur_move;
  // }
  //Serial.println(pinValues);
  int* rc;

  if (pinValues != oldPinValues) {
    Serial.println(pinValues);
    Serial.println(oldPinValues);
    Serial.print("*Pin value change detected*\r\n");
    display_pin_values();
    // detect_state_change();
    rc = compare_states();
    show_state();
    show_matrix();
    if(rc[0]!=-1){
      Serial.println("UPPP");
      Serial.println(cur_move);
      if (prev_state[rc[0]][rc[1]]==1 && ( ( cur_move%2==1 && startup[rc[0]][rc[1]]==3) || ( cur_move%2==0 && startup[rc[0]][rc[1]]==2) )){
        Serial.println("Piece Chosen");
        cur_row_col(rc[0], rc[1], 1);
      }
      else{
        if( chosen_piece[0]!=rc[0] && chosen_piece[1]!=rc[1]){
          Serial.println("DOWNNN");
          validate_move(rc[0], rc[1]);
          Serial.println("Piece Placed");
          Serial.print("Current Turn - ");
          if(cur_move%2==0){
            Serial.println("Black");
          }
          else{
            Serial.println("White");
          }
        }
        else{
          chosen_piece[0] = -1;
          chosen_piece[1] = -1;
        }
      }
      Serial.print("Change - ");
      Serial.print(rc[0]);
      Serial.print(',');
      Serial.print(rc[1]);
      prev_state[rc[0]][rc[1]] = curr_state[rc[0]][rc[1]];
    }
    
    oldPinValues = pinValues;

  }
  // show_matrix();

  //Serial.println("St");
  
  //Serial.println("END");

  delay(POLL_DELAY_MSEC);
  // delete[] rc;

}

