
:int<-(int x) factorial {
  if x == 0 { return 1; }
  return x*factorial(x-1);
}

:int<-() main {
  :int x = 3;
  print factorial(10);
  return 0;
}