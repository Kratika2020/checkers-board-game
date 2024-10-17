using namespace std;

int black_left = 12;
int white_left = 12;
int kings_made = 0;
// 2= Black Pieces
// 3= White Piece
// 0= Black Squares
// -1= White Squares  
int startup[8][8] = {
  {-1 ,  2 , -1 ,  2 , -1 ,  2 , -1 ,  2},  //{(0,0) , (0,1) , (0,2) , (0,3) , (0,4) , (0,5) , (0,6) , (0,7)}
  { 2 , -1 ,  2 , -1 ,  2 , -1 ,  2 , -1},  //{(1,0) , (1,1) , (1,2) , (1,3) , (1,4) , (1,5) , (1,6) , (1,7)}
  {-1 ,  2 , -1 ,  2 , -1 ,  2 , -1 ,  2},  //{(2,0) , (2,1) , (2,2) , (2,3) , (2,4) , (2,5) , (2,6) , (2,7)}
  { 0 , -1 ,  0 , -1 ,  0 , -1 ,  0 , -1},  //{(3,0) , (3,1) , (3,2) , (3,3) , (3,4) , (3,5) , (3,6) , (3,7)}
  {-1 ,  0 , -1 ,  0 , -1 ,  0 , -1 ,  0},  //{(4,0) , (4,1) , (4,2) , (4,3) , (4,4) , (4,5) , (4,6) , (4,7)}
  { 3 , -1 ,  3 , -1 ,  3 , -1 ,  3 , -1},  //{(5,0) , (5,1) , (5,2) , (5,3) , (5,4) , (5,5) , (5,6) , (5,7)}
  {-1 ,  3 , -1 ,  3 , -1 ,  3 , -1 ,  3},  //{(6,0) , (6,1) , (6,2) , (6,3) , (6,4) , (6,5) , (6,6) , (6,7)}
  { 3 , -1 ,  3 , -1 ,  3 , -1 ,  3 , -1}   //{(7,0) , (7,1) , (7,2) , (7,3) , (7,4) , (7,5) , (7,6) , (7,7)}
};

int curr_state[8][8] = {
  {-1 ,  1 , -1 ,  1 , -1 ,  1 , -1 ,  1},  //{(0,0) , (0,1) , (0,2) , (0,3) , (0,4) , (0,5) , (0,6) , (0,7)}
  { 1 , -1 ,  1 , -1 ,  1 , -1 ,  1 , -1},  //{(1,0) , (1,1) , (1,2) , (1,3) , (1,4) , (1,5) , (1,6) , (1,7)}
  {-1 ,  1 , -1 ,  1 , -1 ,  1 , -1 ,  1},  //{(2,0) , (2,1) , (2,2) , (2,3) , (2,4) , (2,5) , (2,6) , (2,7)}
  { 0 , -1 ,  0 , -1 ,  0 , -1 ,  0 , -1},  //{(3,0) , (3,1) , (3,2) , (3,3) , (3,4) , (3,5) , (3,6) , (3,7)}
  {-1 ,  0 , -1 ,  0 , -1 ,  0 , -1 ,  0},  //{(4,0) , (4,1) , (4,2) , (4,3) , (4,4) , (4,5) , (4,6) , (4,7)}
  { 1 , -1 ,  1 , -1 ,  1 , -1 ,  1 , -1},  //{(5,0) , (5,1) , (5,2) , (5,3) , (5,4) , (5,5) , (5,6) , (5,7)}
  {-1 ,  1 , -1 ,  1 , -1 ,  1 , -1 ,  1},  //{(6,0) , (6,1) , (6,2) , (6,3) , (6,4) , (6,5) , (6,6) , (6,7)}
  { 1 , -1 ,  1 , -1 ,  1 , -1 ,  1 , -1}   //{(7,0) , (7,1) , (7,2) , (7,3) , (7,4) , (7,5) , (7,6) , (7,7)}
};

int prev_state[8][8] = {
  {-1 ,  1 , -1 ,  1 , -1 ,  1 , -1 ,  1},  //{(0,0) , (0,1) , (0,2) , (0,3) , (0,4) , (0,5) , (0,6) , (0,7)}
  { 1 , -1 ,  1 , -1 ,  1 , -1 ,  1 , -1},  //{(1,0) , (1,1) , (1,2) , (1,3) , (1,4) , (1,5) , (1,6) , (1,7)}
  {-1 ,  1 , -1 ,  1 , -1 ,  1 , -1 ,  1},  //{(2,0) , (2,1) , (2,2) , (2,3) , (2,4) , (2,5) , (2,6) , (2,7)}
  { 0 , -1 ,  0 , -1 ,  0 , -1 ,  0 , -1},  //{(3,0) , (3,1) , (3,2) , (3,3) , (3,4) , (3,5) , (3,6) , (3,7)}
  {-1 ,  0 , -1 ,  0 , -1 ,  0 , -1 ,  0},  //{(4,0) , (4,1) , (4,2) , (4,3) , (4,4) , (4,5) , (4,6) , (4,7)}
  { 1 , -1 ,  1 , -1 ,  1 , -1 ,  1 , -1},  //{(5,0) , (5,1) , (5,2) , (5,3) , (5,4) , (5,5) , (5,6) , (5,7)}
  {-1 ,  1 , -1 ,  1 , -1 ,  1 , -1 ,  1},  //{(6,0) , (6,1) , (6,2) , (6,3) , (6,4) , (6,5) , (6,6) , (6,7)}
  { 1 , -1 ,  1 , -1 ,  1 , -1 ,  1 , -1}   //{(7,0) , (7,1) , (7,2) , (7,3) , (7,4) , (7,5) , (7,6) , (7,7)}
};

int LED_ref[8][8] = {
  {-1 ,  31 , -1 ,  30 , -1 ,  29 , -1 ,  28},  //{(0,0) , (0,1) , (0,2) , (0,3) , (0,4) , (0,5) , (0,6) , (0,7)}
  { 24 , -1 ,  25 , -1 ,  26 , -1 ,  27 , -1},  //{(1,0) , (1,1) , (1,2) , (1,3) , (1,4) , (1,5) , (1,6) , (1,7)}
  {-1 ,  23 , -1 ,  22 , -1 ,  21 , -1 ,  20},  //{(2,0) , (2,1) , (2,2) , (2,3) , (2,4) , (2,5) , (2,6) , (2,7)}
  { 16 , -1 ,  17 , -1 ,  18 , -1 ,  19 , -1},  //{(3,0) , (3,1) , (3,2) , (3,3) , (3,4) , (3,5) , (3,6) , (3,7)}
  {-1 ,  15 , -1 ,  14 , -1 ,  13 , -1 ,  12},  //{(4,0) , (4,1) , (4,2) , (4,3) , (4,4) , (4,5) , (4,6) , (4,7)}
  { 8 , -1 ,  9 , -1 ,  10 , -1 ,  11 , -1},  //{(5,0) , (5,1) , (5,2) , (5,3) , (5,4) , (5,5) , (5,6) , (5,7)}
  {-1 ,  7 , -1 ,  6 , -1 ,  5 , -1 ,  4},  //{(6,0) , (6,1) , (6,2) , (6,3) , (6,4) , (6,5) , (6,6) , (6,7)}
  { 0 , -1 ,  1 , -1 ,  2 , -1 ,  3 , -1}   //{(7,0) , (7,1) , (7,2) , (7,3) , (7,4) , (7,5) , (7,6) , (7,7)}
};

