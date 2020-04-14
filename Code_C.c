#include <mega16.h>
#include <delay.h>

#define RS              PORTC.1
#define RW              PORTC.2
#define E               PORTC.3

#define motor_A         PORTA.0
#define motor_K         PORTA.1

#define K1              PINA.2
#define K2              PINA.3

#define LED1            PORTA.4
#define LED2            PORTA.5
#define LED3            PORTA.6

#define PORT_KEY        PORTD
#define DDR_KEY         DDRD
#define PIN_KEY         PIND

#define clr_LCD()       write_byte_LCD(0x01)
#define home_LCD()      write_byte_LCD(0x02)

#define NUM0            0xDFFF
#define NUM1            0xFFFE
#define NUM2            0xFFFD
#define NUM3            0xFFFB
#define NUM4            0xFFEF
#define NUM5            0xFFDF
#define NUM6            0xFFBF
#define NUM7            0xFEFF
#define NUM8            0xFDFF
#define NUM9            0xFBFF
#define OK              0xEFFF
#define UNLOCK          0xFFF7
#define LOCK            0xFF7F
#define CHANGE          0xF7FF
#define CLEAR           0x7FFF
#define BACKSPACE       0xBFFF

eeprom unsigned long password_eeprom;

unsigned int scan_key(){
    unsigned char i;   
    unsigned int result=0;
    DDRD=0x80;  
    PORTD=0x7F;      
    for(i=0;i<3;i++){  
        result|=PIN_KEY & 0x0F;
        result<<=4;
        PORTD=PORTD>>1|0x80;  
        DDRD>>=1;
    }  
    result|=PIN_KEY & 0x0F;  
    return result;
}

void wait_LCD(){
    unsigned char result;  
    PORTC=0xF0; 
    RS=0;
    RW=1;  
    while(1){    
        E=1; 
        DDRC=0x0F;   
        result=PINC;
        E=0;
        E=1;   
        E=0;
        if(!(result & 0x80))
            break;
        DDRC=0xFF;
    }
    DDRC=0xFF;  
    RW=0; 
}

void write_byte_LCD(unsigned char byte){
    PORTC&=0x0F;
    PORTC|=(byte)&0xF0;
    E=1;
    E=0;   
    PORTC&=0x0F; 
    PORTC|=(byte<<4)&0xF0;
    E=1;
    E=0; 
    wait_LCD();
}

void init_LCD(){
    E=0;
    RS=0;
    RW=0;
    PORTC|=0x20;
    E=1;
    E=0;    
    wait_LCD();
    write_byte_LCD(0x28);
    write_byte_LCD(0x0C);
    write_byte_LCD(0x06);
    clr_LCD();
}

void move_LCD(unsigned char x, unsigned char y){
    x=(x+y*64) | 0x80;  
    write_byte_LCD(x);
}

void print_LCD(unsigned char *string){
    unsigned char i=0;
    while(string[i]){ 
        RS=1;
        write_byte_LCD(string[i]);
        i++;
    }
    RS=0;
}

void print_num_LCD(unsigned int num){
    unsigned int temp=num, x10=1;
    while(temp>=10){
        x10*=10; 
        temp/=10;
    }    
    while(x10>=1){
        RS=1;
        write_byte_LCD(num/x10+48);
        num%=x10;     
        x10/=10;
    }
}

void open(){
    motor_A=0;
    motor_K=1;
    while(K1); 
    motor_K=0;
}

void close(){
    motor_K=0;
    motor_A=1;
    while(K2); 
    motor_A=0;
}

void nhay_led_1(unsigned char i){
    while(i--){
        LED1=0;
        LED2=1;
        delay_ms(100);
        LED1=1;
        LED2=0;
        delay_ms(100);  
    }   
    LED1=LED2=0;
}

void nhay_led_2(unsigned char i){
    while(i--){
        LED3=1;  
        delay_ms(500);
        LED3=0; 
        delay_ms(500);
    }
}

unsigned long enter_password(){
    unsigned char i=0, time;
    unsigned long password=0;  
    move_LCD(0, 1); 
    while(~scan_key());
    while(1){     
        start:
        switch(scan_key()){   
            default:
                goto start;
            case NUM0:
                print_LCD("*");
                i++;
                password*=10; 
                break;  
            case NUM1: 
                print_LCD("*");
                i++;
                password*=10;  
                password+=1;   
                break;
            case NUM2:    
                print_LCD("*");       
                i++;
                password*=10;  
                password+=2; 
                break;
            case NUM3: 
                print_LCD("*");
                i++;
                password*=10;  
                password+=3;              
                break;
            case NUM4:
                print_LCD("*");
                i++;  
                password*=10;  
                password+=4; 
                break;
            case NUM5:  
                print_LCD("*");
                i++;
                password*=10;  
                password+=5;  
                break;
            case NUM6:
                print_LCD("*");
                i++; 
                password*=10;  
                password+=6; 
                break;  
            case NUM7: 
                print_LCD("*");
                i++;
                password*=10;  
                password+=7; 
                break;
            case NUM8: 
                print_LCD("*");
                i++; 
                password*=10;  
                password+=8;  
                break;
            case NUM9:   
                print_LCD("*");
                i++;
                password*=10;  
                password+=9;   
                break;
            case OK:     
                if(i){
                    clr_LCD();   
                    return password%1000000 + i*100000000;    
                }
                break; 
            case CLEAR:    
                clr_LCD();       
                return 0;
            case BACKSPACE: 
                while(~(scan_key()|BACKSPACE))
                if(i>0){   
                    i--;
                    move_LCD(i, 1);
                    print_LCD(" ");
                    password/=10;   
                    move_LCD(i, 1); 
                    time=250;  
                    while(~(scan_key()|BACKSPACE) && time--)  
                        delay_ms(1);
                }
                break;  
        }
        while(~scan_key());   
    }
}

void unlock(){
    unsigned char password_false=0;
    unsigned long password;
    clr_LCD();
    while(1){  
        move_LCD(1, 0);    
        print_LCD("ENTER PASSWORD:");   
        password=enter_password();   
        if(password==0){
            move_LCD(2, 0);    
            print_LCD("MACH KHOA SO");   
            while(~scan_key()); 
            return;
        }
        else if(password/100000000==6 && password%1000000==password_eeprom){
            move_LCD(4, 0);
            print_LCD("WELLCOME"); 
            nhay_led_1(5);
            open();   
            while(~scan_key());    
            return;
        }
        else{
            move_LCD(2, 0); 
            password_false++;
            if(password_false==5){
                print_LCD("NHAP SAI QUA");
                move_LCD(5, 1);
                print_LCD("5 LAN!");   
                nhay_led_2(10);
                delay_ms(10000); 
            }
            else{
                print_LCD("SAI MAT KHAU!"); 
                delay_ms(1000);  
            }
        }
    }
}

void change_password(){
    unsigned char password_false=0;
    unsigned long password, password_new, password_confirm; 
    while(1){  
        clr_LCD();
        print_LCD("M.KHAU HIEN TAI:"); 
        password=enter_password();
        clr_LCD();  
        if(password==0)
            break;
        move_LCD(2, 0);
        print_LCD("MAT KHAU MOI:");  
        password_new=enter_password(); 
        clr_LCD(); 
        if(password_new==0)
            break;
        move_LCD(0, 0);
        print_LCD("XAC NHAN M.KHAU:");  
        password_confirm=enter_password(); 
        clr_LCD();
        if(password_confirm==0)
            break;     
        else if(password/100000000==6 && password%1000000==password_eeprom){
            if(password_new/100000000==6){
                if(password_new==password_confirm){     
                    move_LCD(2, 0);
                    print_LCD("THAY DOI MAT");    
                    move_LCD(0, 1);
                    print_LCD("KHAU THANH CONG");   
                    password_eeprom=password_new;  
                    nhay_led_1(5);
                    delay_ms(1200);        
                    break;
                }
                else{
                    move_LCD(2, 0);
                    print_LCD("XAC NHAN MAT");    
                    move_LCD(4, 1);
                    print_LCD("KHAU SAI!");    
                    delay_ms(1200);
                }   
            }
            else{
                move_LCD(2, 0);
                print_LCD("MAT KHAU PHAI");   
                move_LCD(4, 1);
                print_LCD("CO 6 SO"); 
                delay_ms(1200);
            }
        }
        else{
            move_LCD(2, 0);
            password_false++;
            if(password_false==5){
                print_LCD("NHAP SAI QUA");
                move_LCD(5, 1);
                print_LCD("5 LAN!");  
                nhay_led_2(10);
                delay_ms(10000); 
            }
            else{
                print_LCD("MAT KHAU SAI!");  
                delay_ms(1200);   
            }
        } 
    }             
    clr_LCD();
    move_LCD(2, 0); 
    print_LCD("MACH KHOA SO");    
    while(~scan_key());
}    

void main(){
    PORTA=0x0C;  
    DDRA=0x73;
    DDRC=0xFF;
    PORTD=0x0F;  
    init_LCD();   
    move_LCD(2, 0);      
    if(password_eeprom>999999)   
        password_eeprom=0;
    print_LCD("MACH KHOA SO");  
    while(1){
        switch(scan_key()){
            case UNLOCK:
                if(K1 && !K2)
                    unlock();
                break;
            case LOCK: 
                if(!K1 && K2)
                    close();
                break;  
            case CHANGE:
                change_password();
                break;
            default:
                break;
        }
    }  
}