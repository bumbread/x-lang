{

  :int x = 2;
  :int y = 3;
  :int z = x + y*123;
  print z / 2;

  if z < 256 {
    x = x + 3;
    while true {
      y = y - 1;
      if y > 2 { break; }
    }
  }

  :int<-(int, int, int) b;
  :int$ a;

  a = $x;
  @a = 3;
  a[2][3] = 3[w];
  $@a[2] = 3;

  {
    :float celcius = 50;
    :float farenheit;
	farenheit = celcius + 2 * $a[40];
  }
  
  a;
  b;
  c;
}
