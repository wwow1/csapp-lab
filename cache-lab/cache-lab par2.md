## 32x32

```c
    int i,j;
    int tmp1,tmp2,tmp3,tmp4;
    int tmp5,tmp6,tmp7,tmp8;
    for(j=0;j<M;j+=8){
        for(i=0;i<N;i++){
            tmp1=A[i][j];  tmp2=A[i][j+1];  tmp3=A[i][j+2];  tmp4=A[i][j+3];
            tmp5=A[i][j+4];  tmp6=A[i][j+5];  tmp7=A[i][j+6];  tmp8=A[i][j+7];
            B[j][i]=tmp1;  B[j+1][i]=tmp2;  B[j+2][i]=tmp3;  B[j+3][i]=tmp4;
            B[j+4][i]=tmp5;  B[j+5][i]=tmp6; B[j+6][i]=tmp7;   B[j+7][i]=tmp8;
        }
    }
//外循环按列，内循环按行，每一次在A中横着连续读8个int
```

```
func 0 (Transpose submission): hits:1766, misses:287, evictions:255
```



## 64x64

```c
    int i,j,k;
    int tmp1,tmp2,tmp3,tmp4;
    int tmp5,tmp6,tmp7,tmp8;
    for(k=0;k<N-8;k+=8){
        for(j=0;j<M;j+=8){
            for(i=k;i<k+8;i++){
                tmp1=A[i][j];  tmp2=A[i][j+1];  tmp3=A[i][j+2];  tmp4=A[i][j+3];
                tmp5=A[i][j+4]; tmp6=A[i][j+5]; tmp7=A[i][j+6]; tmp8=A[i][j+7];
                B[j][i]=tmp1;  B[j+1][i]=tmp2;  B[j+2][i]=tmp3;  B[j+3][i]=tmp4;
                
                B[j][i+8]=tmp5;  B[j+1][i+8]=tmp6;  B[j+2][i+8]=tmp7;  B[j+3][i+8]=tmp8; 
                //暂存在B中的空闲位
            }
            for(i=k;i<k+8;i++){  //将暂存在B中空闲位置的数据转置到正确的位置
                tmp1=B[j][i+8]; tmp2=B[j+1][i+8]; tmp3=B[j+2][i+8]; tmp4=B[j+3][i+8];
                B[j+4][i]=tmp1; B[j+5][i]=tmp2;  B[j+6][i]=tmp3; B[j+7][i]=tmp4;
            }
        }
    }
    for(j=0;j<M;j+=4){ //转置到最后一个块时，B的其他位置已经全部转置完毕，没有额外的空间暂存
        //只能退化为4x4的方式(一次只读4个元素)
        for(i=N-8;i<N;i++){
            tmp1=A[i][j];  tmp2=A[i][j+1];  tmp3=A[i][j+2];  tmp4=A[i][j+3];
            B[j][i]=tmp1;  B[j+1][i]=tmp2;  B[j+2][i]=tmp3;  B[j+3][i]=tmp4;
        }
    }
```

```
func 0 (Transpose submission): hits:10354, misses:1427, evictions:1395
```

上述代码未通过64x64测试



64x64 通过测试版本：

```c
        for(k=0;k<N;k+=8){
            for(j=0;j<M;j+=8){
                for(i=k;i<k+4;i++){   //前四行,前四列正常转置，后四列翻转后暂存在B的非对应位中
                    tmp1=A[i][j];  tmp2=A[i][j+1];  tmp3=A[i][j+2];  tmp4=A[i][j+3];
                    tmp5=A[i][j+4]; tmp6=A[i][j+5]; tmp7=A[i][j+6]; tmp8=A[i][j+7];
                    B[j][i]=tmp1;  B[j+1][i]=tmp2;  B[j+2][i]=tmp3;  B[j+3][i]=tmp4;
                    B[j][i+4]=tmp5; B[j+1][i+4]=tmp6; B[j+2][i+4]=tmp7; B[j+3][i+4]=tmp8;
                }
                for(d=j;d<j+4;d++){  //后四行前四列(corner case)
                    tmp1=A[k+4][d]; tmp2=A[k+5][d]; tmp3=A[k+6][d]; tmp4=A[k+7][d];
                    tmp5=B[d][k+4]; tmp6=B[d][k+5]; tmp7=B[d][k+6]; tmp8=B[d][k+7];

                    B[d][k+4]=tmp1;  B[d][k+5]=tmp2; B[d][k+6]=tmp3; B[d][k+7]=tmp4;
                    //这两行的顺序很重要，不能颠倒，否则会导致大量的冲突不命中
                    B[4+d][k]=tmp5; B[4+d][k+1]=tmp6; B[4+d][k+2]=tmp7; B[4+d][k+3]=tmp8;
                }
                for(i=k+4;i<k+8;i++){  //后四行后四列，按4x4块的模式转置
                    tmp1=A[i][j+4]; tmp2=A[i][j+5]; tmp3=A[i][j+6];  tmp4=A[i][j+7];
                    B[j+4][i]=tmp1; B[j+5][i]=tmp2; B[j+6][i]=tmp3;  B[j+7][i]=tmp4;
                }
            }
        }
```

```
func 0 (Transpose submission): hits:9066, misses:1179, evictions:1147
```





## 61x67

```c
for(j=0;j<M;j+=8){
    for(i=0;i<N;i++){
      if(j==56){
         tmp1=A[i][j];  tmp2=A[i][j+1];  tmp3=A[i][j+2];  
         tmp4=A[i][j+3];  tmp5=A[i][j+4];
         B[j][i]=tmp1;  B[j+1][i]=tmp2;  B[j+2][i]=tmp3;  
         B[j+3][i]=tmp4; B[j+4][i]=tmp5;
      }
      tmp1=A[i][j];  tmp2=A[i][j+1];  tmp3=A[i][j+2];  tmp4=A[i][j+3];
      tmp5=A[i][j+4];  tmp6=A[i][j+5];  tmp7=A[i][j+6];  tmp8=A[i][j+7];
      B[j][i]=tmp1;  B[j+1][i]=tmp2;  B[j+2][i]=tmp3;  B[j+3][i]=tmp4;
      B[j+4][i]=tmp5;  B[j+5][i]=tmp6; B[j+6][i]=tmp7;   B[j+7][i]=tmp8;
	}
}
```

```
Summary for official submission (func 0): correctness=1 misses=1852
```

