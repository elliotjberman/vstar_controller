int findIndex(int x) {
  int row = (x / 4) - 9;
  int column = 3 - x % 4;
  return row * 4 + column;
}
