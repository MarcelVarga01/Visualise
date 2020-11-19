//Implementation of the visualise task
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

enum {eCHAR,eUCHAR,eINT,eUINT,eLONG};
// Sizes of types are saved in nibbles, for later usage
// Position in arrays correspond to the enums above e.g. sizes[eCHAR] is 2
const int sizes[] = {2,2,8,8,16};
// Type limits
const long long upperlimit[] = {CHAR_MAX, UCHAR_MAX, INT_MAX, UINT_MAX, LLONG_MAX};
const long long lowerlimit[] = {CHAR_MIN,         0, INT_MIN,        0, LLONG_MIN};

// each element holds the type of the value we entered and 
// index, which represents the starting index in *args[] of the value the element represents
// e.g. for input: char 0000 0000 type is eCHAR and index is 2; Due to knowing the size of Char in nibbles, we can go up to args[3] easily
struct queueelem {
    int type, index;
    struct queueelem *next;
};
typedef struct queueelem Elem;
struct thequeue {
    Elem *front;
    Elem *back;
};
typedef struct thequeue Queue;
    
void InitialiseQueue(Queue *s);
void Push(Queue *q, int type, int index);
int Pop(Queue *q, int *type, int *index);    
bool QueueEmpty(Queue *q);
void printbinary(char *s);
void error();

// Converts string to a decimal integer
// "valid" becomes equal to 0 if we encounter an input error
long long ConvertDecStr(char *s, bool *valid, int type, bool testing){
    *valid = true;
    long long value = atoll(s);

    //Check if by converting value back to a string it is equal to the initial string, thus the input being valid
    char test[80];
    sprintf(test,"%lld",value);
    if (strcmp(s,test)) *valid = 0;
    if (value>upperlimit[type] || value<lowerlimit[type]) *valid = 0;

    // Parameter "testing" tells the function if we're testing, so it doesn't terminate the program prematurely
    if (!(*valid) && !testing) error();
    return value;
}

// Takes decimal value and type and puts its binary representation in the string
void ConvertDec(long long value, int type, char *binary){
    // Length of the representation is equal to the number of nibbles for each type times 4
    int length = sizes[type]*4;
    bool negative = false;

    // Initialize array
    for (int i = 0; i < length; i++)
        binary[i] = '0';

    // Check if value is negative
    // Adding 1 from the initial value then multiplying it by (-1) assures us that by inverting the bits(excluding the MSB) of the new value
    // we will get the initial one in binary
    // e.g if value == -72 =>  value becomes 71. By converting 71 to binary then inverting the bits(apart from the MSB) we will get the representation of -72
    if (value < 0) {
        negative = true;
        value++;
        value *= -1;
    }

    int i=length-1; // Put values in string from right to left
    while(value){
        binary[i] = value%2 + '0';
        i--;
        value = value >> 1;
    }
    if (negative){ //invert bits if the initial value was negative
    for (int i = 1; i < length; i++)
        if(binary[i]=='0') binary[i] = '1';
                      else binary[i] = '0';
    }
    if (negative) binary[0] = '1';
    binary[length] = '\0';

}

//Converts binary to decimal
long long ConvertBin(int n, char *s[n], int index, int type){
    long long value = 0,p = 0;
    bool unsign = 0;
    if (type == eUCHAR || type == eUINT) unsign = 1;
    // Initialize the value holds 2^n, weight of each bit
    p = upperlimit[type]>>1;
    p++;

    if (s[index][0] == '1' && !unsign) value = lowerlimit[type]; //we have a negative value
    // convert the most significant nibble first to exclude the first bit if the value is signed
    for (int j = 0; j < 4; j++)
        if (j!=0 || unsign){
        value += p * (s[index][j] - '0'); 
        p >>= 1;
        }

    for (int i = index+1; i < index+sizes[type]; i++){
        for (int j = 0; j < 4; j++){
            value += p * (s[i][j] - '0');
            p >>= 1;
        }
    }
    return value;
}

// Checks input type
// Returns -1 if input type is invalid
int InputType(char *s1, char *s2){
    if (!strcmp(s1,"unsigned")){
        if (!strcmp(s2,"char")) return eUCHAR;
        if (!strcmp(s2,"int")) return eUINT;
        return -1;
    }
    if (!strcmp(s1,"char")) return eCHAR;
    if (!strcmp(s1,"int")) return eINT;
    if (!strcmp(s1,"long")) return eLONG;
    if (!strcmp(s1,"unsignedchar")) return eUCHAR;
    if (!strcmp(s1,"unsignedint")) return eUINT;
    return -1;
}

// Checks whether input is in binary or decimal form
// Returns 1 if the input is binary, 0 otherwise
bool IsBinary(int n, char *s[n]){
    // variable index holds the index in s at which the first decimal/binary value might be entered
    int index = 0;
    if (s[1][0] == '{'){
        for (int i = 1; i < n; i++){
            int len = strlen(s[i]);
            if (s[i][len-1] == '}') index = i+1;
            
        }
    }
    else{
        int x = InputType(s[1], s[2]);
        if (x == -1) return 0;
        if (x == eUCHAR || x == eUINT) index = 3;
        else index = 2;
    }
    if (index >= n) return 0; // return 0 in case the index at which we start looking is out of scope

    // Check if there are only 1s and 0s entered
    for (int i = index; i < n; i++){
        int x = strlen(s[i]);
        if (x != 4) return 0;
        for (int j = 0; j < x; j++){
        if ((s[i][j] != '0') && (s[i][j] != '1')) return 0;
        }
    }
    // Check number of arguments
    // This captures the case in which for example we have to convert int 1111 from decimal to binary
    // Due to the smallest type being 2 nibbles, n must be bigger at least by 2 than index to allow checking both nibbles of a char
    if(s[1][0] != '{') if (index == n-1) return 0; 
    if(s[1][0] == '{'){
        int values = 1;
        for (int i = 1; i < index; i++){
            int x = strlen(s[i]);
            for (int j = 0; j < x; j++)
            if(s[i][j] == ';') values++; //when ';' is found that means there's a new type that has been entered
        }
        //n-index should be at least the (number of values) * 2 for the entered value to be binary, since the smallest type, char, needs 2 nibbles
        if ((n-index) < values * 2) return 0; 
    }
    return 1;
}

// Formats input and puts all values that need to be converted in a queue
// Captures Input Errors regarding input type and number of types/values(or nibbles for binary input)
void FormatInput(int n, char *s[n], bool binary, Queue *q){
    bool structured = 0; //tells us if the input is a structured one, with multiple values, or not
    if(s[1][0] == '{') structured = 1;
    if (!structured){
        int type,index;// variable index holds the index in s (main equivalent would be args) at which values that have to be converted start appearing 
        type = InputType(s[1],s[2]); // Note that we are sure s[2] exists, since n > 2 (this function is only called in main if n > 2)
        
        if(type == -1) error(); //input type is wrong
        if(type == eCHAR || type == eINT || type == eLONG) index = 2;
        else index = 3;

        if(!binary){ // "binary" tells us if the input of the program is binary or not
            if (n != index+1) error(); //too many inputs
            Push(q, index, type);
        }
        else{
            if (n != index + sizes[type]) error(); //too many/little nibble inputs
            Push(q, index, type);
        }
    }
    if (structured){
        // Create one string "c" that will contain all types that are in the input
        char digits[] = "-0123456789";
        char *c = malloc(200*sizeof(char));
        strcpy(c,s[1]);

        int i = 2;
        while(!(strchr(digits,s[i][0])) && i < n){
            strcat(c,s[i]);
            i++;
        }

        int index = i, type = 0; // Save the index at which numerical values will start appearing
        char *token;
        // We will break the types string into a series of tokens, containing only the input types, one by one
        token = strtok(c, "{};");
        while(token){
            if (index > n) error(); // Too many input types/too little values entered (index would be out of scope of args[][])
            type = InputType(token,"");
            if (type == -1) error(); //Input Error 
            Push(q, index, type);

            //update index for next value
            if(!binary) index++;
                   else index += sizes[type];

            token = strtok(NULL, "{};"); //update token
        }
        //check that there are no values left that have no type corresponding to them i.e. too many values have been entered
        if (index != n)  error();
        free(c);
        }      
    }

// Empties the queue and calls the right functions for the input
void Solve(int n, char *s[n], bool binary, Queue *q){
    while(!QueueEmpty(q)){
        int index = 0,type = 0;

        Pop(q,&index,&type);
        if(!binary){
            bool valid = 1;
            char ANS[100];
            ConvertDec(ConvertDecStr(s[index],&valid,type,0),type,ANS);
            printbinary(ANS);
        }
        else{
            long long value = 0;
            value = ConvertBin(n, s, index, type);
            printf("%lld ",value);
        }
    }
    printf("\n");
}
// -------------------------------------------------------------------------------------------
// Queue Functionality

void InitialiseQueue(Queue *q) {
    q->front = NULL;
    q->back = NULL;
}

// Adds new element to the queue
void Push(Queue *q, int index, int type) {
    Elem *e;
    e = (Elem *) malloc(sizeof(Elem));
    if (QueueEmpty(q)){//if the queue is empty both the front and the back should be the new element
        q->back = e;
        q->front = e;
    }
    else{ //add the new element at the back of the queue
    q->back->next = e;
    e->next = NULL;
    q->back = e;
    }

    e->next = NULL;
    e->type = type;
    e->index = index;

}

// Saves type and index of the front of the queue then deletes the first element of the queue
int Pop(Queue *q, int *index, int *type){
    Elem *curr;
    if(QueueEmpty(q)) return -1; //report value in case we try to pop an empty queue

    *type = q->front->type;
    *index = q->front->index;

    curr = q->front;
    q->front=q->front->next;
    free(curr);
    return 0;
}

// Returns 1 if the queue is empty
bool QueueEmpty(Queue *q){
    if(q->front == NULL) return 1;
    return 0;
}
// -------------------------------------------------------------------------------------------
//  User interface
void error(){
    printf("Input error.\n");
    exit(1);
}

// A replacement for the library assert function.
void assert(int line, int b) {
    if (b) return;
    printf("The test on line %d fails.\n", line);
    exit(1);
}

// Prints binary value grouped in nibbles
void printbinary(char *s){
    int n=strlen(s);
    for (int i = 0; i < n; i++){
        printf("%c",s[i]);
        if(i%4 == 3) printf(" ");
    }
}
// -------------------------------------------------------------------------------------------
// Testing
// Tests 1 to 20
// Checks conversion from string to decimal and the validity of the input
void testConvertDecStr(){
    bool valid=false;
    assert(__LINE__, ConvertDecStr("1",&valid,eCHAR,1) == 1 && valid==1);
    assert(__LINE__, ConvertDecStr("12345678",&valid,eINT,1) == 12345678 && valid==1);
    assert(__LINE__, ConvertDecStr("2147483647",&valid,eINT,1) == 2147483647 && valid==1);
    assert(__LINE__, ConvertDecStr("2147483648",&valid,eINT,1) == 2147483648 && valid==0);
    assert(__LINE__, ConvertDecStr("2147483649",&valid,eINT,1) == 2147483649 && valid==0);
    assert(__LINE__, ConvertDecStr("-2147483648",&valid,eINT,1) == -2147483648 && valid==1);
    assert(__LINE__, ConvertDecStr("-2147483649",&valid,eINT,1) == -2147483649 && valid==0);
    assert(__LINE__, ConvertDecStr("255",&valid,eCHAR,1) == 255 && valid==0);
    assert(__LINE__, ConvertDecStr("-128",&valid,eCHAR,1) == -128 && valid==1);
    assert(__LINE__, ConvertDecStr("-129",&valid,eCHAR,1) == -129 && valid==0);
    assert(__LINE__, ConvertDecStr("255",&valid,eUCHAR,1) == 255 && valid==1);
    assert(__LINE__, ConvertDecStr("-1",&valid,eUCHAR,1) == -1 && valid==0);
    assert(__LINE__, ConvertDecStr("-1",&valid,eUINT,1) == -1 && valid==0);
    assert(__LINE__, ConvertDecStr("08",&valid,eCHAR,1) == 8 && valid==0);
    assert(__LINE__, ConvertDecStr("-x0",&valid,eCHAR,1) == 0 && valid==0);
    assert(__LINE__, ConvertDecStr("-x",&valid,eCHAR,1) == 0 && valid==0);
    assert(__LINE__, ConvertDecStr("4y",&valid,eCHAR,1) == 4 && valid==0);
    assert(__LINE__, ConvertDecStr("9223372036854775807",&valid,eCHAR,1) == 9223372036854775807  && valid==0);
    assert(__LINE__, ConvertDecStr("9223372036854775807",&valid,eLONG,1) == 9223372036854775807  && valid==1);
    assert(__LINE__, ConvertDecStr("9223372036854775808",&valid,eLONG,1) && valid==0);
}

// Tests 21 to 33
// Tests conversion from decimal to binary
void testConvertDec(){ 
    char *binary=malloc(100*sizeof(char));
    ConvertDec(-128,eCHAR,binary);assert(__LINE__,!strcmp(binary,"10000000"));
    ConvertDec(0,eCHAR,binary);assert(__LINE__,!strcmp(binary,"00000000"));
    ConvertDec(127,eCHAR,binary);assert(__LINE__,!strcmp(binary,"01111111"));
    ConvertDec(255,eCHAR,binary);assert(__LINE__,!strcmp(binary,"11111111"));
    ConvertDec(-72,eCHAR,binary);assert(__LINE__,!strcmp(binary,"10111000"));
    ConvertDec(72,eCHAR,binary);assert(__LINE__,!strcmp(binary,"01001000"));
    ConvertDec(INT_MAX,eINT,binary);assert(__LINE__,!strcmp(binary,"01111111111111111111111111111111"));
    ConvertDec(INT_MIN,eINT,binary);assert(__LINE__,!strcmp(binary,"10000000000000000000000000000000"));
    ConvertDec(INT_MIN+1,eINT,binary);assert(__LINE__,!strcmp(binary,"10000000000000000000000000000001"));
    ConvertDec(UINT_MAX,eUINT,binary);assert(__LINE__,!strcmp(binary,"11111111111111111111111111111111"));
    ConvertDec(0,eUINT,binary);assert(__LINE__,!strcmp(binary,"00000000000000000000000000000000"));
    ConvertDec(LLONG_MAX,eLONG,binary);assert(__LINE__,!strcmp(binary,"0111111111111111111111111111111111111111111111111111111111111111"));
    ConvertDec(LLONG_MIN,eLONG,binary);assert(__LINE__,!strcmp(binary,"1000000000000000000000000000000000000000000000000000000000000000"));
    free (binary);
}

// Tests 34 to 45
// Tests conversion from binary to decimal
void testConvertBin(){
    assert(__LINE__,ConvertBin(4,(char *[]) { "", "char", "0010","1100" },2,eCHAR) == 44); 
    assert(__LINE__,ConvertBin(6,(char *[]) { "", "{char;char}", "0010","1100", "0011","1111" },4,eCHAR) == 63); 
    assert(__LINE__,ConvertBin(7,(char *[]) {"","{unsigned", "char;char}", "0001", "0000", "1000", "0110"},3,eCHAR) == 16); 
    assert(__LINE__,ConvertBin(7,(char *[]) {"","{unsigned", "char;char}", "0001", "0000", "1000", "0000"},5,eCHAR) == -128); 
    assert(__LINE__,ConvertBin(18,(char *[]) {"","long", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111"},2,eLONG) == -1);
    assert(__LINE__,ConvertBin(20,(char *[]) {"","{char;long}","1111","0111", "1000", "0000", "0000", "0000", "0000", "0000", "0000", "0000", "0000", "0000", "0000", "0000", "0000", "0000", "0000", "0000"},4,eLONG) == LLONG_MIN);
    assert(__LINE__,ConvertBin(18,(char *[]) {"","long", "0111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111", "1111"},2,eLONG) == LLONG_MAX);
    assert(__LINE__,ConvertBin(4,(char *[]) { "", "char", "0000","0000" },2,eCHAR) == 0); 
    assert(__LINE__,ConvertBin(4,(char *[]) { "", "char", "1111","1111" },2,eCHAR) == -1); 
    assert(__LINE__,ConvertBin(4,(char *[]) { "", "char", "1000","0000" },2,eCHAR) == CHAR_MIN); 
    assert(__LINE__,ConvertBin(4,(char *[]) { "", "char", "0000","0000" },2,eCHAR) == 0);    
}

// Tests 46 to 54
// Tests if input type is interpreted correctly
void testInputType(){
    assert(__LINE__,InputType("char","")==eCHAR);
    assert(__LINE__,InputType("int","")==eINT);
    assert(__LINE__,InputType("long","")==eLONG);
    assert(__LINE__,InputType("unsigned","char")==eUCHAR);
    assert(__LINE__,InputType("unsigned","int")==eUINT);
    assert(__LINE__,InputType("unsigned","")==-1);
    assert(__LINE__,InputType("","")==-1);
    assert(__LINE__,InputType("unsignedchar","")==eUCHAR);
    assert(__LINE__,InputType("unsignedint","")==eUINT);
}

// Tests 55 to 70
// Tests if the function determines correctly whether the input is in a binary form or not
void testIsBinary(){
    assert(__LINE__,IsBinary(3,(char *[]) { "", "int", "0010" }) == 0);
    assert(__LINE__,IsBinary(3,(char *[]) { "", "char", "0010" }) == 0);
    assert(__LINE__,IsBinary(4,(char *[]) { "", "char", "0010", "0000" }) == 1);
    assert(__LINE__,IsBinary(3,(char *[]) { "", "int", "int" }) == 0);
    assert(__LINE__,IsBinary(3,(char *[]) { "", "unsigned", "0010" }) == 0);
    assert(__LINE__,IsBinary(5,(char *[]) { "", "unsigned", "char", "0000", "0000" }) == 1);
    assert(__LINE__,IsBinary(5,(char *[]) { "", "unsigned", "char", "000", "1000" }) == 0);
    assert(__LINE__,IsBinary(5,(char *[]) { "", "unsigned", "char", "0000", "1120" }) == 0);
    assert(__LINE__,IsBinary(6,(char *[]) { "", "{char;char}", "0000", "0000", "1111", "1111" }) == 1);
    assert(__LINE__,IsBinary(5,(char *[]) { "", "{char;char}", "0000", "0000", "1111" }) == 0);
    assert(__LINE__,IsBinary(6,(char *[]) { "", "{char;char}", "0010", "0010", "0010", "1111" }) == 1);
    assert(__LINE__,IsBinary(6,(char *[]) { "", "{char;char}", "0010", "0010", "02010", "1111" }) == 0);
    assert(__LINE__,IsBinary(6,(char *[]) { "", "{char;char}", "0010", "0010", "010", "1111" }) == 0);
    assert(__LINE__,IsBinary(6,(char *[]) { "", "{char;char}", "0010", "0010", "010", "1111" }) == 0);
    assert(__LINE__,IsBinary(8,(char *[]) { "", "{unsigned", "char;unsigned", "char}", "0000", "0000", "1111", "1111"}) == 1);
    assert(__LINE__,IsBinary(5,(char *[]) { "", "{unsigned", "int;int}", "1111", "1010"}) == 0);

}

// Tests 71 to 85
// Test if input is formatted correctly into a queue
void testFormatInput(){
    int index=0,type=0;
    Queue q;
    InitialiseQueue(&q);
    FormatInput(3,(char *[]) {"","int","5"},0,&q);  Pop(&q,&index,&type);  assert(__LINE__,index == 2 && type == eINT);
    FormatInput(3,(char *[]) {"","char","5"},0,&q);  Pop(&q,&index,&type);  assert(__LINE__,index == 2 && type == eCHAR);
    FormatInput(3,(char *[]) {"","long","5"},0,&q);  Pop(&q,&index,&type);  assert(__LINE__,index == 2 && type == eLONG);
    FormatInput(4,(char *[]) {"","unsigned", "char","5"},0,&q);  Pop(&q,&index,&type);  assert(__LINE__,index == 3 && type == eUCHAR);
    FormatInput(4,(char *[]) {"","unsigned", "int","5"},0,&q);  Pop(&q,&index,&type);  assert(__LINE__,index == 3 && type == eUINT);
    FormatInput(4,(char *[]) {"","char","0010","0100"},1,&q);  Pop(&q,&index,&type);  assert(__LINE__,index == 2 && type == eCHAR);
    FormatInput(5,(char *[]) {"","{unsigned", "int;char}", "5", "33"},0,&q);    Pop(&q,&index,&type);   assert(__LINE__,index == 3 && type == eUINT);
                                                                                Pop(&q,&index,&type);   assert(__LINE__,index == 4 && type == eCHAR);
    FormatInput(4,(char *[]) {"","{int;char}", "5", "33"},0,&q);Pop(&q,&index,&type);   assert(__LINE__,index == 2 && type == eINT);
                                                                Pop(&q,&index,&type);   assert(__LINE__,index == 3 && type == eCHAR);
    FormatInput(7,(char *[]) {"","{unsigned", "char;char}", "0001", "0000", "1000", "0110"},1,&q);
                                                                Pop(&q,&index,&type);   assert(__LINE__,index == 3 && type == eUCHAR);
                                                                Pop(&q,&index,&type);   assert(__LINE__,index == 5 && type == eCHAR);
    FormatInput(9,(char *[]) {"","{unsigned", "char;char;char}", "0001", "0000", "1000", "0110", "0000", "0000"},1,&q);
                                                                Pop(&q,&index,&type);   assert(__LINE__,index == 3 && type == eUCHAR);
                                                                Pop(&q,&index,&type);   assert(__LINE__,index == 5 && type == eCHAR);
                                                                Pop(&q,&index,&type);   assert(__LINE__,index == 7 && type == eCHAR);
    
}

//Tests 86 to 91
// Tests queue functionalities
void testQueue(){
    Queue q;
    int x,y;
    InitialiseQueue(&q);
    assert(__LINE__,(QueueEmpty(&q) == 1));
    Push(&q,1,1);
    Push(&q,4,3);
    assert(__LINE__,(QueueEmpty(&q) == 0));
    assert(__LINE__,(Pop(&q,&x,&y)==0 && x==1 && y==1));
    assert(__LINE__,(QueueEmpty(&q) == 0));
    assert(__LINE__,(Pop(&q,&x,&y)==0 && x==4 && y==3));
    assert(__LINE__,(QueueEmpty(&q) == 1));
}

// Run the tests
void test(){
    testConvertDecStr();
    testConvertDec();
    testConvertBin();
    testInputType();
    testIsBinary();
    testQueue();
    testFormatInput();
    printf("All tests pass.\n");
    exit(1);
}

// Run the program or, if there are no arguments, test it
int main(int n, char *args[n]) {
    if(n == 1) test();
    if(n == 2) error(); //not enough arguments
    else{
        bool b = IsBinary(n,args); //checks if input is in binary form or not
        Queue q; //Queue which will hold data that will be manipulated
        InitialiseQueue(&q);
        FormatInput(n, args, b, &q);
        Solve(n, args, b, &q);
    } 
    return 0;
}
