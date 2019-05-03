int findIndex(int x) {
  int row = 12 - (x / 4);
  int column = x % 4;
  return row * 4 + column;
}
