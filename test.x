
:int<-() main {
 :int a = a[:];
 :bool b = false;
 
 if a < 3 {
  :int i = 0;
  while i < 3 {
   i += 1;
   if i == 10 {
    break;
   }
  }
  a = 1+i;
 }

 :int<-(int a) square_integer {
  return a*a;
 }
}
