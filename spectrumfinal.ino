
//This is not my code. I found it and used it as a base since the code for the instructables is bullshit and doesnt work. Ill make others lifes easier. 
//I havent messed with the bluetooth yet, will keep posted as I go. I just do this when i have free time, so dont expect fast speedy work. 
//And lastly, I have managed to use the same wireing diagram as the instructables so, just follow the guide. 


// 16 Band led spectrum analyzer
// neopixel led strip 16 * 16 + hc-06 + simple mic amp
// code by worynim@gmail.com

// 현재 문제점 메모리 부족으로 256 개를 모두 켜기 어려움
// neo pixel 켜는 부분을 직접 하드 코딩 하는 것이 좋을 듯
// 주파수를 나누는 부분도 다시 신경을 써서.. 해볼 것.
// 정 안된다면.. 메가를 이용


unsigned char brightness = 50;
unsigned char line_color = 0;    // 0 : rainbow 
unsigned char dot_dir = 1 ;      // 1:up,    0:down
unsigned char dot_color = 0  ;   // rainbow, This is chosen further down the code, feel free to change it to what you like 
unsigned char line_dir = 1 ;     // 1: up    0: down  
unsigned char random_flag=1;


#include <Adafruit_NeoPixel.h>
#define PIN 3
Adafruit_NeoPixel strip = Adafruit_NeoPixel(100, PIN, NEO_GRB + NEO_KHZ800);

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <math.h>
#include <FFT.h> // include the library

signed int sel_freq[11] = {0,2,4,6,8,10,12,14,16,18,20};

signed int freq_offset[10] = { 
15,15,15,15,15,15,15,15,15,15   //Edit this if your shit gets weird false lights coming on. i recoment starting at all 15s and increrase if lights dont go off when source is turned all the way down. 
};      
unsigned char freq_div[10] = { 
        6,6,6,6,6,6,6,6,6,6     //if it keeps peaking the top led, then add 1 to the specific row. and if its not responsive, remove 1. 
};

unsigned char band[10] = { 
        0 };  // 0~15
unsigned char display_band[10]= {       //havent messed with this yet
        0 }; 
unsigned char dot_band[10]={
        0};
        
        
void setup() {
 //       Serial.begin(9600); // use the serial port
        TIMSK0 = 0; // turn off timer0 for lower jitter
        ADCSRA = 0xe5; // set the adc to free running mode
        ADMUX = 0b01000111; // use adc0
        DIDR0 = 0x01; // turn off the digital input for adc0

        strip.begin();
        strip.show(); // Initialize all pixels to 'off'

        strip.setBrightness(brightness);
}

void loop() {
        
       // while(1) { // reduces jitter
                cli();  // UDRE interrupt slows this way down on arduino1.0
                for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
                        while(!(ADCSRA & 0x10)); // wait for adc to be ready
                        ADCSRA = 0xf5; // restart adc
                        byte m = ADCL; // fetch adc data
                        byte j = ADCH;
                        int k = (j << 8) | m; // form into an int
                        k -= 0x0200; // form into a signed int
                        k <<= 6; // form into a 16b signed int
                        fft_input[i] = k; // put real data into even bins
                        fft_input[i+1] = 0; // set odd bins to 0
                }
                fft_window(); // window the data for better frequency response
                fft_reorder(); // reorder the data before doing the fft
                fft_run(); // process the data in the fft
                fft_mag_log(); // take the output of the fft
                sei();

                volatile static unsigned char line_delay_count=0;
                volatile static signed char max_dot_count=0;

                if(++line_delay_count>=2) {    ///  2
                        line_delay_count=0; 
                        for(int ii=0; ii<11; ii++)if( display_band[ii] > 0 )  display_band[ii] -=1; 
                }

                              //  line_delay_count=0; 
                              // for(int ii=0; ii<10; ii++)if( display_band[ii] > 0 )  display_band[ii] -=1;    //These three lines allow you to have a custom refresh speed so, only mess with the line delay. I just keep it out. 

                              // for(int ii=0; ii<10; ii++) display_band[ii] =0; 


                if(++max_dot_count>=8 ) { ////8
                        max_dot_count=0; 
                        for(int ii=0; ii<10; ii++)
                        {
                                if(dot_dir){  
                                        if( dot_band[ii] > 0 )dot_band[ii] -=1; 
                                }        /// up 
                                else {  
                                        if( dot_band[ii] > 0 )dot_band[ii] -=1;  
                                }       /// down
                        }
                }

                static unsigned char color_offset=0;
                color_offset++;



                for(int ii=0; ii<10; ii++)   //
                {
                        int fft_value=0;
                        long fft_sum=0;
   // 라인 부드럽게 하는 코드
                        unsigned char sum_count = sel_freq[ii+1] - sel_freq[ii];
                        for(int z =sum_count-1 ;  z > 0; z--) //  2,4,7
                        {
                                fft_sum += fft_log_out[ sel_freq[ii] + z ];
                        }
                        fft_value = fft_sum / sum_count - freq_offset[ii];

                        ////fft_value = fft_log_out[ sel_freq[ii] ] - freq_offset[ii];     // freq select

                        if( fft_value < 0 ) fft_value = 0;

                        band[ii] = fft_value / freq_div[ii];                  // current value update  range 0~15
                        if(band[ii] > 10) band[ii]=0;

                        if( display_band[ii] < band[ii] )  display_band[ii]= band[ii];     // line update

                        for(int jj=0; jj < 10; jj++)
                        {
                                int address =0;
                            // line direction LED 16ea
                                if( (ii % 2) == (line_dir)) address = 0+jj + (10*ii);
                                else address = jj + (10*ii); 

                                // dot color line color 
                                if(dot_band[ii] == jj) 
                                {
                                        if(dot_color == 0)strip.setPixelColor(address, Wheel(9+jj + (10*ii) + color_offset)  );  // dot color
                                        else if(dot_color == 1)strip.setPixelColor(address, 0xFF0000);  // dot color
                                        else if(dot_color == 2)strip.setPixelColor(address, 0xFFFF00);  // dot color
                                        else if(dot_color == 3)strip.setPixelColor(address, 0x00FF00);  // dot color    Looks scary, but google the weird number after address and google will show you the color. 
                                        else if(dot_color == 4)strip.setPixelColor(address, 0x00FFFF);  // dot color
                                        else if(dot_color == 5)strip.setPixelColor(address, 0x0000FF);  // dot color
                                        else if(dot_color == 6)strip.setPixelColor(address, 0xFF00FF);  // dot color
                                        else if(dot_color == 7)strip.setPixelColor(address, 0xFFFFFF);  // dot color
                                        else if(dot_color == 8)strip.setPixelColor(address, Wheel(15-jj + (16*ii)));  // dot color
                                        else ;
                                }
                                else if(display_band[ii] > jj) 
                                {
                                        if(line_color == 0)strip.setPixelColor(address, Wheel(9-jj + (10*ii) + color_offset )   );  // line color : rainbow
                                        else if(line_color == 1)strip.setPixelColor(address, 0xFF0000);  // line color : rainbow
                                        else if(line_color == 2)strip.setPixelColor(address, 0xFFFF00 );  // line color : rainbow
                                        else if(line_color == 3)strip.setPixelColor(address, 0x00FF00 );  // line color : rainbow
                                        else if(line_color == 4)strip.setPixelColor(address, 0x00FFFF );  // line color : rainbow
                                        else if(line_color == 5)strip.setPixelColor(address, 0x0000FF );  // line color : rainbow
                                        else if(line_color == 6)strip.setPixelColor(address, 0xFF00FF );  // line color : rainbow
                                        else if(line_color == 7)strip.setPixelColor(address, 0xFFFFFF );  // line color : rainbow 
                                        else if(line_color == 8)strip.setPixelColor(address, Wheel(15-jj + (16*ii))   );  // line color : rainbow
                                        else ;
                                }
                                else   strip.setPixelColor(address, 0  ); 
                        }
                }
                strip.show();
/*
                if( Serial.available() > 0 )
                {
                        static char rx_data=0;
                        rx_data = Serial.read();
                        if( rx_data == '1' ){ 
                                line_color++;  
                                if(line_color > 8)line_color = 0;   
                                Serial.print("L color: ");
                                Serial.write(line_color+'0');  
                                Serial.println(); 

                        }
                        else if( rx_data == '2' ){ 
                                dot_color++; 
                                if(dot_color > 8)dot_color = 0;  
                                Serial.print("D color: ");
                                Serial.write(dot_color+'0');  
                                Serial.println(); 
                        }
                        else if( rx_data == 'd') 
                        {
                                dot_dir ^= 1;
                                Serial.print("D dir: ");
                                Serial.write(dot_dir+'0');  
                                Serial.println(); 

                        }
                        else if( rx_data == 'l') 
                        {
                                line_dir ^= 1;
                                Serial.print("L dir: ");
                                Serial.write(line_dir+'0');  
                                Serial.println(); 
                        }
                        else if( rx_data == 'b'){ 
                                brightness += 51; 
                                strip.setBrightness(brightness);  
                                if( brightness < 51)brightness = 51; 
                                Serial.print("brightness: ");
                                Serial.println(brightness); 
                        }
                        else if( rx_data == 'r')
                        {
                                random_flag ^= 1;
                                if(random_flag) Serial.print("R ON");
                                else Serial.print("R OFF");
                        }
                }
                
                if( random_flag )
                {
                        static unsigned int random_count=0;
                        random_count++;
                        if(random_count > 1000)
                        {
                                random_count=0;
                                dot_dir = random(2);
                                line_dir = random(2);
                                line_color = random(9);
                                dot_color = random(9);
                        }
                }
                */
        }
//}

uint32_t Wheel(byte WheelPos) {
        if(WheelPos < 85) {
                return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        } 
        else if(WheelPos < 170) {
                WheelPos -= 85;
                return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
        } 
        else {
                WheelPos -= 170;
                return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
}
