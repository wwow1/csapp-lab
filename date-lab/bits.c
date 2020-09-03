/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

//expr可以由以下元素构成:
//int常量只能使用0~255
//参数和局部变量
//！ ~
// & ^ | + << >>

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

//不能做的事:
//使用if,do,while,for,switch等控制语句
//定义和使用宏
//定义额外的函数
//调用函数
//使用其他运算符，例如&& || -,or ?
//使用强制类型转换
//使用除了int外的其他数据类型(例如数组，结构和联合)

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 //可以提供的前提条件:
 //int为32位且由补码表示
 //右移为算术右移
 //当移位数小于0或大于31时，会出现不可预测的情况

  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

// 浮点数规则:
//可以使用:
//循环和条件控制语句
//可以使用int和unsigned类型
//使用任意的int或unsigned常量
//可以使用基于int或unsigned的任何算术，逻辑，比较运算


FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

//不能使用:
//定义和使用宏
//定义和调用函数
//强制类型转换
//使用int和unsigned外的其他数据类型
//使用任何浮点数据类型，操作符和常量

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.

//dlc不仅会检查你的代码是否符合以上标准
//还会更进一步检查你使用的各种运算符的数量是否超出上限(=不会被计数，它可以被任意使用)
//运算符使用上限会在每个函数上方给出

NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  //tl将在x中为0,在y中为1的位置为1
  //tl将在x中为1,在x中为0的位置为1
  //~tl & ~tr 将所有在tl和tr中都为0的位置为1,可以将它看作~(tl|tr)
  int tl=~x&y;
  int tr=~y&x;
  int ans=~(~tl & ~tr);
  return ans;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  int ans=0x1<<31;
  return ans;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  //想了非常多的办法，但是仍然无法区分0x7FFFFFFF和0xFFFFFFFF(心累...
  //将等式分成两个部分 ~(x+x+1) 和 !(x+1)
  //要返回1  则有两种组合 ~(x+x+1)=-1,!(x+1)=1,或者是 ~(x+x+1)=!(x+1)=0
  //讨论第一种组合，要使！(x+1)=1 则x=0xFFFFFFFF,代入~(x+x+1)=0,所以不存在数x能够满足第一种组合
  //讨论第二种组合，要使~(x+x+1)=0,x=0xFFFFFFFF或0x7FFFFFFF, 而要同时满足!(x+1)=0则只有x=0x7FFFFFFF
  return !(~(x+x+1)+!(x+1));
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  //让x和0xAAAAAAAA想与后减去0xAAAAAAAA,若结果为0则正确，反之错误
  int tst=0xAA;
  int base=0xAA;
  tst=(tst<<8)+base;
  tst=(tst<<8)+base;
  tst=(tst<<8)+base;
  return !((x&tst)+(~tst+1));
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x+1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) { 
  //先拆分高位的3进行确认，然后查看低位
  //当低位0~7时，有lres=0,llf≠0
  //低位8~9时，lres≠0，llf=0  (ltmp=0--> lres=0)
  //低位A~F时，lres≠0,llf≠0,且lres&llf≠0,  这时lres=6,最终llf=2或4或6
  int high=x>>4;
  int low=x&0xF;
  int hres=high^3;   

  int ltmp=low&8;     
  int lres=(ltmp>>1)+(ltmp>>2);
  int llf=(low+(~8+1))&0xE;
  return !(hres | (lres&llf));
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {  
  //x=0-->return z  x≠0-->return y
  int ans=0;
  ans=ans+((~(!x+(~0)))&z);  //当x=0 ans+=z  当x≠0 ans+=0
  ans=ans+((!x+(~0))&y);    //当x=0 ans+=y  当x≠0 ans+=0
  return ans;
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {  
  //直接做减法会出现溢出导致错误
  //将异号的情况先单独讨论，再对同号的情况使用减法判断大小

  int ls=~((x&(~y))>>31)+1;  //x<0 y>=0 ls=1   否则ls=0
  int gt=!((y&(~x))>>31);   //x>=0 y<0 gt=0 否则gt=1
  //可以看到 当x,y异号时, ls==gt ,同号时ls^gt=1,这一特点会在最后一步使用到
  int Tmin=~(1<<31);
  int tmpx=x&Tmin;
  int tmpy=y&Tmin;
  int cmp=~((tmpy+(~tmpx+1))>>31);  
  //先除去符号位做减法比较大小,
  //然后根据符号位反转结果，只有在x,y同号时才考虑cmp，其他情况它会被忽略
  int ans=(ls&gt)+((ls^gt)&cmp);
  return ans;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {     
//除0外的数全部弄成负数然后右移31位,最后+1 
//利用-0=0的特点
  int ans=((x|(~x+1))>>31)+1;  //x=0 ans-->1  其他ans=0
  return ans;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4     0000 0005
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  //对负数取反，正数不变，此时最右侧的0的下标就是答案
  //先将x变为 000..001xx..xx的形式(x可以是0也可以是1)
  int top=(1<<31)&x; //取x最高位
  //将x变为000..00111..1的形式，
  int tmp;
  int cur=0;
  int ans=16;
  int tst=1<<15;
  int stm2;
  int stm1;
  x=x^(top>>31);

  tmp=x>>1; x=x|tmp;//2
  tmp=x>>2; x=x|tmp;  //4
  tmp=x>>4; x=x|tmp; //8
  tmp=x>>8; x=x|tmp; //16
  tmp=x>>16; x=x|tmp; //32

  //二分法找到最右侧的0的下标
  cur=tst&x;
  //根据cur决定左移还是右移
  stm1=(!cur)+(~1)+1;
  stm2=~stm1;
  tst=tst<<(stm1&8);  tst=tst>>(stm2&8); 
  ans=ans+(stm1&8); ans=ans+(stm2&(~8+1));

  cur=tst&x;
  stm1=(!cur)+(~1)+1;
  stm2=~stm1;
  tst=tst<<(stm1&4);  tst=tst>>(stm2&4); 
  ans=ans+(stm1&4);  ans=ans+(stm2&(~4+1));

  cur=tst&x;
  stm1=(!cur)+(~1)+1;
  stm2=~stm1;
  tst=tst<<(stm1&2);  tst=tst>>(stm2&2); 
  ans=ans+(stm1&2);  ans=ans+(stm2&(~2+1));

  cur=tst&x;
  stm1=(!cur)+(~1)+1;
  stm2=~stm1;
  tst=tst<<(stm1&1);  tst=tst>>(stm2&1); 
  ans=ans+(stm1&1);  ans=ans+(stm2&(~1+1));

  //注意最后一次不能漏,参考样例0x80000000就可以明白为什么需要加这一次
  cur=tst&x;
  stm1=(!cur)+(~1)+1;
  ans=ans+(stm1&1);
  return ans;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) { 
  //如果是NaN或无限大，直接返回参数
  unsigned tmp=uf&0x7F800000u;
  unsigned checkNaN=tmp^0x7F800000u;
  unsigned ans;
  unsigned norm=tmp;
  if(!checkNaN || !uf){
    return uf;
  }
  //非规格化数左移一位
  if(!norm){
    tmp=uf<<1;
    ans=(uf&0x80000000u)+(tmp&0x7FFFFFFFu);
    return ans;
  }
  else{
    //规格化数对阶码+1
    tmp=uf+0x00800000u;
    ans=(uf&0x807FFFFFu)+(tmp&0x7F800000u);
    return ans;
  }
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) { //127次方最多,-126次方最少
  //通过阶码先区分出越界的数
  unsigned tmp=uf&0x7F800000;
  unsigned checkNaN=tmp^0x7F800000u;
  int ans;
  int M;
  int mov;
  if(!checkNaN)   //无限大和NaN
    return 0x80000000u;
  ////阶码大于等于127+31的数溢出，实际上0xCF000000是不会溢出的，但是它的转换结果就是0x80000000
  if(tmp>=0x4F000000) 
    return 0x80000000u;
  else if(tmp<0x3F80000) //阶码小于127，则可以直接返回0，因为int无法表示那么小的数(非常接近0的小数) 
    return 0;
  else{
    ans=0;
    M=uf&0x007FFFFF;
    mov=tmp>>23;
    if(tmp<0x4B000000)   //当阶码小于(127+23),则需要将尾数M右移，反之左移
      ans=ans+((M+0x00800000)>>(150-mov));
    else                 //阶码大于等于(127+23),需要将位数左移
      ans=ans+((M+0x00800000)<<(mov-150));
    if(uf>>31)  //前面都是默认正数情况，如果符号位为1需要将结果转为负数
      ans=~ans+1;
    return ans;
  }
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) { 
    unsigned ans=0;
    if(!x)  return 0x3f800000;
    if(x>127)     //超出表达范围
      return 0x7F800000;
    else if(x<-149) //float能表达的最小正数是0x00000001(非规格化数)
      return 0;
    //2^x的浮点表示为非规格化数
    if(x<-126){
      x=~(x+126)+1;
      ans=0x00800000>>x;
      return ans;
    }
    else{ //2^x的浮点表示为规格化数
      ans=ans+(((x+127)&0x000000FF)<<23);
      return ans;
    }
}
