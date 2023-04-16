//
// Created by peter on 4/2/2023.
//
#include "print.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>



/**
 * This reverse the strings from given offset
 * @param str       : String
 * @param end       : End point (exclusive)
 * @param offset    : Offset
 */
static void reverseStr(char *str,uint8_t end,uint32_t offset){
    if(end<=1)
        return;
    uint32_t i=offset, j=end-1;
    char temp;
    while (i<j){
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}


/****************************INTEGER CONVERSION*****************************************/

static const char ch_start = 'a';
static const char ch_end = 'z';

/**
 * This converts unsigned 32bit integer to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static uint32_t uint32ToStr(uint32_t val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint32_t i=offset;
    uint8_t temp;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=(uint32_t)base;
    }
    while(i < (width+offset)){
        str[i++]=fill;
    }

    reverseStr(str,i,offset);
    return i;
}

/**
 * This converts signed 32bit integer to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static uint32_t int32ToStr(int32_t val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint8_t is_negative = val < 0;
    uint32_t i=offset;
    uint8_t temp;
    if(is_negative)
        val=-val;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=base;
    }
    if(is_negative){
        if(fill == ' ')
            str[i++] = '-';
        else
            width -= 1;
    }
    while (i < (width+offset)) {
        str[i++] = fill;
    }
    if(is_negative && fill!=' ')
        str[i++] = '-';

    reverseStr(str,i,offset);
    return i;
}

/**
 * This converts unsigned 64bit integer to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static uint32_t uint64ToStr(uint64_t val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint32_t i=offset;
    uint8_t temp;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=base;
    }

    while(i < (width+offset)){
        str[i++]=fill;
    }

    reverseStr(str,i,offset);
    return i;
}

/**
 * This converts signed 64bit integer to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static uint32_t int64ToStr(int64_t val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint8_t is_negative = val < 0;
    uint32_t i=offset;
    uint8_t temp;
    if(is_negative)
        val=-val;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=base;
    }
    if(is_negative){
        if(fill == ' ')
            str[i++] = '-';
        else
            width -= 1;
    }
    while (i < (width+offset)) {
        str[i++] = fill;
    }
    if(is_negative && fill!=' ')
        str[i++] = '-';
    reverseStr(str,i,offset);
    return i;
}

/**
 * This converts signed integer to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static uint32_t intToStr(int val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint8_t is_negative = val < 0;
    uint32_t i=offset;
    uint8_t temp;
    if(is_negative)
        val=-val;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=base;
    }
    if(is_negative){
        if(fill == ' ')
            str[i++] = '-';
        else
            width -= 1;
    }
    while (i < (width+offset)) {
        str[i++] = fill;
    }
    if(is_negative && fill!=' ')
        str[i++] = '-';
    reverseStr(str,i,offset);
    return i;
}

/**
 * This converts signed long to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static uint32_t longToStr(long val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint8_t is_negative = val < 0;
    uint32_t i=offset;
    uint8_t temp;
    if(is_negative)
        val=-val;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=base;
    }
    if(is_negative){
        if(fill == ' ')
            str[i++] = '-';
        else
            width -= 1;
    }
    while (i < (width+offset)) {
        str[i++] = fill;
    }
    if(is_negative && fill!=' ')
        str[i++] = '-';

    reverseStr(str,i,offset);
    return i;
}

/**
 * This converts signed long long to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static uint32_t longLongToStr(long long val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint8_t is_negative = val < 0;
    uint32_t i=offset;
    uint8_t temp;
    if(is_negative)
        val=-val;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=base;
    }
    if(is_negative){
        if(fill == ' ')
            str[i++] = '-';
        else
            width -= 1;
    }
    while (i < (width+offset)) {
        str[i++] = fill;
    }
    if(is_negative && fill!=' ')
        str[i++] = '-';

    reverseStr(str,i,offset);
    return i;
}

/**
 * This converts unsigned integer to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static uint32_t unsignedToStr(unsigned val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint32_t i=offset;
    uint8_t temp;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=base;
    }

    while(i < (width+offset)){
        str[i++]=fill;
    }

    reverseStr(str,i,offset);
    return i;
}

/**
 * This converts unsigned integer to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static __attribute__((unused)) uint32_t unsignedIntToStr(unsigned int val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint32_t i=offset;
    uint8_t temp;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=base;
    }

    while(i < (width+offset)){
        str[i++]=fill;
    }

    reverseStr(str,i,offset);
    return i;
}

/**
 * This converts unsigned long to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static uint32_t unsignedLongToStr(unsigned long val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint32_t i=offset;
    uint8_t temp;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=base;
    }

    while(i < (width+offset)){
        str[i++]=fill;
    }

    reverseStr(str,i,offset);
    return i;
}

/**
 * This converts unsigned long long to string
 * @param val       : Integer value
 * @param str       : String buffer
 * @param base      : Base supports from 2 to 36
 * @param width     : Width of string
 * @param offset    : Offset
 * @param fill      : Filling character if width is empty
 * @return          : Last index + 1 of string
 */
static uint32_t unsignedLongLongToStr(unsigned long long val,char *str,uint8_t base,uint8_t width,uint32_t offset,char fill){
    uint32_t i=offset;
    uint8_t temp;
    if(val==0)
        str[i++] = '0';
    while(val){
        temp = val % base;
        if(temp>=0 && temp<=9)
            str[i++] = (char)(temp + (int)'0');
        else if(temp>=10 && temp <= (10+(ch_end-ch_start)))
            str[i++] = (char)(temp - 10 + ch_start);
        else
            str[i++] = (char)temp;
        val/=base;
    }

    while(i < (width+offset)){
        str[i++]=fill;
    }

    reverseStr(str,i,offset);
    return i;
}

/****************************FLOAT CONVERSION*****************************************/

/**
 * This converts float to string array
 * @param val           : Float value
 * @param str           : String
 * @param after_point   : Precision after decimal
 * @param width         : Width
 * @param offset        : Offset
 * @param fill          : Filling character if width is empty
 * @return              : Last index + 1 of string
 */
static __attribute__((unused)) uint32_t floatToStr(float val,char *str,uint8_t after_point,uint8_t width,uint32_t offset,char fill){
    uint8_t is_negative = val < 0.0f;
    if(is_negative)
        val=-val;

    int32_t integer_part = (int32_t)val;
    uint32_t decimal_part = (uint32_t)((val-(float) integer_part) * powf(10,after_point));

    if(after_point<1)
        after_point = 1;

    if(width>(after_point+1))
        width = width-after_point-1;
    else
        width = 1;

    if(integer_part==0 && is_negative && width<2)
        width = 2;
    else if(is_negative)
        integer_part = -integer_part;
    uint32_t i = offset;
    uint32_t temp = int32ToStr(integer_part,str,10,width,i,fill);
    if(integer_part==0 && is_negative) {
        if(fill==' ')
            str[temp - 2] = '-';
        else
            str[i] = '-';
    }
    i = temp;
    str[i++] = '.';
    i = uint32ToStr(decimal_part,str,10,after_point,i,'0');
    return i;
}

/**
 * This converts double to string array
 * @param val           : Float value
 * @param str           : String
 * @param after_point   : Precision after decimal
 * @param width         : Width
 * @param offset        : Offset
 * @param fill          : Filling character if width is empty
 * @return              : Last index + 1 of string
 */
static uint32_t doubleToStr(double val,char *str,uint8_t after_point,uint8_t width,uint32_t offset,char fill){
    uint8_t is_negative = val < 0.0f;
    if(is_negative)
        val=-val;

    if(after_point<1)
        after_point = 1;

    int64_t integer_part = (int64_t)val;
    uint64_t decimal_part = (uint64_t)((val-(double ) integer_part) * powf(10,after_point));

    if(width>(after_point+1))
        width = width-after_point-1;
    else
        width = 1;

    if(integer_part==0 && is_negative && width<2)
        width = 2;
    else if(is_negative)
        integer_part = -integer_part;
    uint32_t i = offset;
    uint32_t temp = int64ToStr(integer_part,str,10,width,i,fill);
    if(integer_part==0 && is_negative) {
        if(fill==' ')
            str[temp - 2] = '-';
        else
            str[i] = '-';
    }
    i = temp;
    str[i++] = '.';
    i = uint64ToStr(decimal_part,str,10,after_point,i,'0');
    return i;
}

/****************************Printf*************************************************/
__attribute__((weak)) void printChar(char ch){
//    putchar(ch);
}

__attribute__((weak)) void printString(char*str,uint32_t len){
//    for(uint32_t i=0;i<len;i++)
//        printChar(str[i]);
}

static int strToInt(const char* str,int len){
    if(len==0)
        return 0;
    int x = 0;
    int ten = 1;
    for(int i=len-1;i>=0;i--){
        x += (int)(str[i]-'0')*ten;
        ten*=10;
    }
    return x;
}

uint32_t print(const char * format, ...){
    uint32_t count = 0;
    va_list vl;

    uint32_t i = 0, len;
    char temp_buff[30]={0};
    char * str_arg;

    va_start(vl, format );
    while (format[i]!='\0'){
        if(format[i] == '%'){
            char filler = ' ';
            char a_buff[3]="";
            char b_buff[3]="";
            int a_len=0,b_len=0;

            i++;
            if(format[i] == '0')
                filler = format[i++];
            uint8_t point_encounter = 0;
            while(1){
                if(format[i] == 'c'){
                    printChar((char)va_arg( vl, int ));
                    count++;
                    break;
                }else if(format[i] == 'd'){
                    len = intToStr(va_arg( vl, int ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'u'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'd'){
                    i++;
                    len = longToStr(va_arg( vl, long ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'u'){
                    i++;
                    len = unsignedLongToStr(va_arg( vl, unsigned long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'd'){
                    i+=2;
                    len = longLongToStr(va_arg( vl, long long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'u'){
                    i+=2;
                    len = unsignedLongLongToStr(va_arg( vl,unsigned long long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'x'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 16, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'x'){
                    i++;
                    len = unsignedLongToStr(va_arg( vl, unsigned long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'x'){
                    i+=2;
                    len = unsignedLongLongToStr(va_arg( vl,unsigned long long), temp_buff, 16, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'p'){
                    if(sizeof(void*)<=4)
                        len = uint32ToStr(va_arg( vl, uint32_t), temp_buff, 16, 8, 0, '0');
                    else if(sizeof(void*)<=8)
                        len = uint64ToStr(va_arg( vl, uint64_t), temp_buff, 16, 16, 0, '0');
                    printString("0x",2);
                    count+=len+2;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'o'){
                    len = intToStr(va_arg( vl, int ), temp_buff, 8, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'f'){
                    len = doubleToStr(va_arg( vl, double ), temp_buff, b_len==0?6:strToInt(b_buff, b_len), strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'b'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 2, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 's'){
                    str_arg = va_arg( vl, char* );
                    len = strlen(str_arg);
                    count+=len;
                    printString(va_arg( vl, char* ), len);
                    break;
                }else if(format[i] == '.'){
                    point_encounter = 1;
                    i++;
                }else{
                    if(!point_encounter)
                        a_buff[a_len++] = format[i];
                    else
                        b_buff[b_len++] = format[i];
                    i++;
                }
            }
        } else {
            printChar(format[i]);
            count++;
        }
        i++;
    }
    va_end(vl);
    return count;
}

uint32_t vprint(const char * format, va_list vl){
    uint32_t count = 0;
    uint32_t i = 0, len;
    char temp_buff[30]={0};
    char * str_arg;

    while (format[i]!='\0'){
        if(format[i] == '%'){
            char filler = ' ';
            char a_buff[3]="";
            char b_buff[3]="";
            int a_len=0,b_len=0;

            i++;
            if(format[i] == '0')
                filler = format[i++];
            uint8_t point_encounter = 0;
            while(1){
                if(format[i] == 'c'){
                    printChar((char)va_arg( vl, int ));
                    count++;
                    break;
                }else if(format[i] == 'd'){
                    len = intToStr(va_arg( vl, int ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'u'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'd'){
                    i++;
                    len = longToStr(va_arg( vl, long ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'u'){
                    i++;
                    len = unsignedLongToStr(va_arg( vl, unsigned long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'd'){
                    i+=2;
                    len = longLongToStr(va_arg( vl, long long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'u'){
                    i+=2;
                    len = unsignedLongLongToStr(va_arg( vl,unsigned long long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'x'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 16, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'x'){
                    i++;
                    len = unsignedLongToStr(va_arg( vl, unsigned long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'x'){
                    i+=2;
                    len = unsignedLongLongToStr(va_arg( vl,unsigned long long), temp_buff, 16, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'p'){
                    if(sizeof(void*)<=4)
                        len = uint32ToStr(va_arg( vl, uint32_t), temp_buff, 16, 8, 0, '0');
                    else if(sizeof(void*)<=8)
                        len = uint64ToStr(va_arg( vl, uint64_t), temp_buff, 16, 16, 0, '0');
                    printString("0x",2);
                    count+=len+2;
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'o'){
                    len = intToStr(va_arg( vl, int ), temp_buff, 8, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'f'){
                    len = doubleToStr(va_arg( vl, double ), temp_buff, b_len==0?6:strToInt(b_buff, b_len), strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 'b'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 2, strToInt(a_buff, a_len), 0, filler);
                    count+=len;
                    printString(temp_buff, len);
                    break;
                }else if(format[i] == 's'){
                    str_arg = va_arg( vl, char* );
                    len = strlen(str_arg);
                    count+=len;
                    printString(va_arg( vl, char* ), len);
                    break;
                }else if(format[i] == '.'){
                    point_encounter = 1;
                    i++;
                }else{
                    if(!point_encounter)
                        a_buff[a_len++] = format[i];
                    else
                        b_buff[b_len++] = format[i];
                    i++;
                }
            }
        } else {
            printChar(format[i]);
            count++;
        }
        i++;
    }
    return count;
}

uint32_t sprint(char *buff ,const char * format, ...){
    uint32_t count = 0;
    va_list vl;

    uint32_t i = 0, len;
    char temp_buff[30]={0};
    char * str_arg;

    va_start(vl, format );
    while (format[i]!='\0'){
        if(format[i] == '%'){
            char filler = ' ';
            char a_buff[3]="";
            char b_buff[3]="";
            int a_len=0,b_len=0;

            i++;
            if(format[i] == '0')
                filler = format[i++];
            uint8_t point_encounter = 0;
            while(1){
                if(format[i] == 'c'){
                    buff[count++] = (char)va_arg( vl, int );
                    break;
                }else if(format[i] == 'd'){
                    len = intToStr(va_arg( vl, int ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'u'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'd'){
                    i++;
                    len = longToStr(va_arg( vl, long ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'u'){
                    i++;
                    len = unsignedLongToStr(va_arg( vl, unsigned long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'd'){
                    i+=2;
                    len = longLongToStr(va_arg( vl, long long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'u'){
                    i+=2;
                    len = unsignedLongLongToStr(va_arg( vl,unsigned long long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'x'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 16, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'x'){
                    i++;
                    len = unsignedLongToStr(va_arg( vl, unsigned long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'x'){
                    i+=2;
                    len = unsignedLongLongToStr(va_arg( vl,unsigned long long), temp_buff, 16, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'p'){
                    if(sizeof(void*)<=4)
                        len = uint32ToStr(va_arg( vl, uint32_t), temp_buff, 16, 8, 0, '0');
                    else if(sizeof(void*)<=8)
                        len = uint64ToStr(va_arg( vl, uint64_t), temp_buff, 16, 16, 0, '0');
                    printString("0x",2);
                    count+=len+2;
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'o'){
                    len = intToStr(va_arg( vl, int ), temp_buff, 8, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'f'){
                    len = doubleToStr(va_arg( vl, double ), temp_buff, b_len==0?6:strToInt(b_buff, b_len), strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'b'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 2, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 's'){
                    str_arg = va_arg( vl, char* );
                    len = strlen(str_arg);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = str_arg[k];
                    break;
                }else if(format[i] == '.'){
                    point_encounter = 1;
                    i++;
                }else{
                    if(!point_encounter)
                        a_buff[a_len++] = format[i];
                    else
                        b_buff[b_len++] = format[i];
                    i++;
                }
            }
        } else {
            buff[count++] = format[i];
        }
        i++;
    }
    va_end(vl);
    buff[count]='\0';
    return count;
}

uint32_t vsprint(char *buff ,const char * format, va_list vl){
    uint32_t count = 0;
    uint32_t i = 0, len;
    char temp_buff[30]={0};
    char * str_arg;

    while (format[i]!='\0'){
        if(format[i] == '%'){
            char filler = ' ';
            char a_buff[3]="";
            char b_buff[3]="";
            int a_len=0,b_len=0;

            i++;
            if(format[i] == '0')
                filler = format[i++];
            uint8_t point_encounter = 0;
            while(1){
                if(format[i] == 'c'){
                    buff[count++] = (char)va_arg( vl, int );
                    break;
                }else if(format[i] == 'd'){
                    len = intToStr(va_arg( vl, int ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'u'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'd'){
                    i++;
                    len = longToStr(va_arg( vl, long ), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'u'){
                    i++;
                    len = unsignedLongToStr(va_arg( vl, unsigned long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'd'){
                    i+=2;
                    len = longLongToStr(va_arg( vl, long long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'u'){
                    i+=2;
                    len = unsignedLongLongToStr(va_arg( vl,unsigned long long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'x'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 16, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'x'){
                    i++;
                    len = unsignedLongToStr(va_arg( vl, unsigned long), temp_buff, 10, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'l' && format[i + 1] == 'l' && format[i + 2] == 'x'){
                    i+=2;
                    len = unsignedLongLongToStr(va_arg( vl,unsigned long long), temp_buff, 16, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'p'){
                    if(sizeof(void*)<=4)
                        len = uint32ToStr(va_arg( vl, uint32_t), temp_buff, 16, 8, 0, '0');
                    else if(sizeof(void*)<=8)
                        len = uint64ToStr(va_arg( vl, uint64_t), temp_buff, 16, 16, 0, '0');
                    printString("0x",2);
                    count+=len+2;
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'o'){
                    len = intToStr(va_arg( vl, int ), temp_buff, 8, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'f'){
                    len = doubleToStr(va_arg( vl, double ), temp_buff, b_len==0?6:strToInt(b_buff, b_len), strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 'b'){
                    len = unsignedToStr(va_arg( vl, unsigned ), temp_buff, 2, strToInt(a_buff, a_len), 0, filler);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = temp_buff[k];
                    break;
                }else if(format[i] == 's'){
                    str_arg = va_arg( vl, char* );
                    len = strlen(str_arg);
                    for(uint32_t k=0;k<len;k++)
                        buff[count++] = str_arg[k];
                    break;
                }else if(format[i] == '.'){
                    point_encounter = 1;
                    i++;
                }else{
                    if(!point_encounter)
                        a_buff[a_len++] = format[i];
                    else
                        b_buff[b_len++] = format[i];
                    i++;
                }
            }
        } else {
            buff[count++] = format[i];
        }
        i++;
    }
    buff[count]='\0';
    return count;
}

