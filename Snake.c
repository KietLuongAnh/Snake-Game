/*
	Luong Anh Kiet			19146347
	Ngo Van Huu Luan		19146352
	Nguyen Thanh Vinh		19146426
*/

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define spi_channel                     0
#define speed                           10000000
#define sample_rate                     25
#define config                          26
#define gyro_config                     27
#define acc_config                      28
#define interrupt_enable                56
#define power_management                107
#define acc_x                           59
#define acc_y                           61
#define acc_z                           63
#define decode_mode_register_address    0x09;
#define decode_value                    0x00;
#define intensity_address               0x0A;
#define intensity_value                 0x0F;
#define scan_limit                      0x0B;
#define scan_value                      0x07;
#define shutdown                        0x0C;
#define shutdown_mode                   0x01;
#define display_test                    0x0F;
#define display_value                   0x00;

unsigned char truc_y[8]={1,2,4,8,16,32,64,128};
unsigned char arr_tail[2];
unsigned char arr_body[2];
unsigned char arr_head[2];
unsigned char arr_ball[2];
unsigned char arr_add_1[2];
unsigned char arr_add_2[2];
unsigned char arr_add_3[2];
unsigned char arr_add_4[2];
int add_1 = 0;
int add_2 = 0;
int add_3 = 0;
int add_4 = 0;

int snake_len=3;
int mpu;
int head = 2;
int body = 1;
int tail = 0;
int ball = 0;
int all = 0;
int stt_x=1;
int stt_y = 0;
int stt = 1;
float pitch, roll;
int16_t read_acc(unsigned char sen)
{
    int16_t high, low, data;
    high = wiringPiI2CReadReg8(mpu,sen);
    low = wiringPiI2CReadReg8(mpu,sen + 1);
    data = (high<<8)|low;
    return data;
}
void read_mpu()
{
    float ax = (float)read_acc(acc_x)/4096.0;
    float ay = (float)read_acc(acc_y)/4096.0;
    float az = (float)read_acc(acc_z)/4096.0;

    pitch = atan2(ax,sqrt(pow(ay,2)+pow(az,2)))*180/M_PI;
    roll = atan2(ay,sqrt(pow(ax,2)+pow(az,2)))*180/M_PI;

    printf("Roll: %.2f \n", roll);
    printf("Pitch: %.2f \n", pitch);
    printf("\n");
}
uint16_t pos_x(int pos_value)
{
    return pos_value%8 + 1;
}
uint16_t pos_y(int pos_value)
{
    return truc_y[pos_value/8];
}
void init_6050()
{
    // register 25-28, 56,107
    // Sample rate 500Hz
    wiringPiI2CWriteReg8(mpu,sample_rate,15);
    // Khogn su dung nguon xung ngoai, tat DLFP 
    wiringPiI2CWriteReg8(mpu,config,0);
    // gyro FS: +- 500o/s
    wiringPiI2CWriteReg8(mpu,gyro_config,0x08);
    //acc FS: +-8g
    wiringPiI2CWriteReg8(mpu,acc_config,0x10);
    // mo interrupt cua data ready
    wiringPiI2CWriteReg8(mpu,interrupt_enable,1);
    // chon nguon xung Gyro X
    wiringPiI2CWriteReg8(mpu,power_management,0x01);
}

void init_max7219()
{
    unsigned char decode[2], inten[2], scan[2], shut[2], display[2];
    decode[0] = decode_mode_register_address;
    decode[1] = decode_value;

    inten[0] = intensity_address;
    inten[1] = intensity_value;

    scan[0] = scan_limit;
    scan[1] = scan_value;

    shut[0] = shutdown;
    shut[1] = shutdown_mode;

    display[0] = display_test;
    display[1] = display_value;
    wiringPiSPIDataRW(spi_channel,decode,2);
    wiringPiSPIDataRW(spi_channel,inten,2);
    wiringPiSPIDataRW(spi_channel,scan,2);
    wiringPiSPIDataRW(spi_channel,shut,2);
    wiringPiSPIDataRW(spi_channel,display,2);
}
void clear_matrix()
{    
    for (int i = 0; i <9; i++)
    {
        unsigned char buffer[2];
        buffer[0] = i;
        buffer[1] = 0x00;
        wiringPiSPIDataRW(spi_channel,buffer,2);
    }
}
int getRandom(int min, int max) // tao ball random
{
    return min + (int)(rand()*(max-min+1.0)/(1.0+RAND_MAX));
}
void doibit()
{
    add_4 = add_3;
    add_3 = add_2;
    add_2 = add_1;
    add_1 = tail;
    tail = body;
    body = head;
}
void create_snake_ball()
{
    unsigned char temp;
    unsigned char buffer[2];
    arr_head[0] = pos_x(head);
    arr_body[0] = pos_x(body);
    arr_tail[0] = pos_x(tail);
    arr_ball[0] = pos_x(ball);
    arr_add_1[0] = pos_x(add_1);
    arr_add_2[0] = pos_x(add_2);
    arr_add_3[0] = pos_x(add_3);
    arr_add_4[0] = pos_x(add_4);
    for(int x=1;x<9;x++)
	{
        if(arr_head[0]==x)
        {
            int temp = pos_y(head);
				all = all + temp;
            temp = 0;
        }
        if(arr_body[0]==x)
        {
            int temp = pos_y(body);
            all = all + temp;
            temp = 0;
        }
        if(arr_tail[0]==x)
        {
            int temp = pos_y(tail);
            all = all + temp;
            temp = 0;
        }
        if(arr_ball[0]==x && ball != head)
        {
            int temp = pos_y(ball);
            all = all + temp;
            temp = 0;            
        }
        if(arr_add_1[0]==x && snake_len >= 4)
        {
            int temp = pos_y(add_1);
            all = all + temp;
            temp = 0;            
        }
        if(arr_add_2[0]==x & snake_len >= 5)
        {
            int temp = pos_y(add_2);
            all = all + temp;
            temp = 0;            
        }
        if(arr_add_3[0]==x && snake_len >= 6)
        {
            int temp = pos_y(add_3);
            all = all + temp;
            temp = 0;            
        }
        if(arr_add_4[0]==x && snake_len == 7)
        {
            int temp = pos_y(add_4);
            all = all + temp;
            temp = 0;            
        }
        buffer[0] = 0x00 + x;
        buffer[1] = 0x00 + all;
        wiringPiSPIDataRW(spi_channel,buffer,2);
        all=0;
	}
}



void read_stt()
{
    read_mpu();
    if(roll > 20 && stt_x == 1)
    {
        stt_y = 1;
        stt_x = 0;
        stt   = 1;
    }
    if(roll < -20 && stt_x == 1)
    {
        stt_y = 1;
        stt_x = 0;
        stt   = 0;
    }
    if(pitch > 20 && stt_y == 1)
    {
        stt_x = 1;
        stt_y = 0;
        stt   = 1;
    }
    if(pitch < -20 && stt_y == 1)
    {
        stt_x = 1;
        stt_y = 0;
        stt   = 0;
    }
}

void check_stt()
{
    read_stt();
    printf("Trang thai x: %d \n", stt_x);
    printf( "Trang thai y: %d\n", stt_y);
    if(stt_x == 1 && stt ==1)
    {
        doibit();
        head++;
        if(head%8 == 0)
        {
            head = head -8;
        }
    }
    if(stt_x == 1 && stt ==0)
    {
        doibit();
        head--;
        if(head%8 == 7)
        {
            head = head + 8;
        }
    }
    if(stt_y == 1 && stt ==1)
    {
        doibit();
        head = head-8;
        if(head < 0)
        {
            head =64 + head;
        }
    }
    if(stt_y == 1 && stt ==0)
    {
        doibit();
        head = head + 8;
        if(head > 64)
        {
            head =head - 64;
        }
    }
    create_snake_ball();
}

void eatten()
{
    for(int x=0; x<10 ;x++)
    {
        if(head == ball)
        {
            snake_len++;
            x = 11;
            check_stt();
            printf("%d\n",snake_len);
        }
        else
        {
            printf("\n%d\n", x);
            check_stt();
            delay(500);
        }
    }
}
int main()
{
    // Setup for wiringPi
    wiringPiSetup();

    //Setup for mpu
    mpu = wiringPiI2CSetup(0x68);

    // Setup for max7219
    wiringPiSPISetup(spi_channel, speed);

    // THiet lap che do do mpu6050
    init_6050();
    
    //Config max7219
    init_max7219();

    while(1)
    {
        srand((unsigned int)time(NULL));
        ball = getRandom(1,63);
        while(ball == body || ball == tail)
        {
        srand((unsigned int)time(NULL));
        ball = getRandom(1,63);
        }
        // Doc gia tri do 
        clear_matrix();
        eatten();
    }
    return 0;
}
