
//include LCD and Wire library
#include <LiquidCrystal.h>
#include <Wire.h>
#include <DS3231.h>
#include <OneWire.h>  // odpowiada za komunikację za pomocą protokołu 1-Wire.
#include <DS18B20.h>  //biblioteka termometru

#define PIN_LIGHT 12  //definicja pinu dla oswietlenia
#define PIN_HEATING 13  //definicja pinu dla oswietlenia

#define ONEWIRE_PIN 10   //W definicji o nazwie ONEWIRE_PIN przechowywany jest numer pinu do którego podłączasz wyjście danych DQ z czujnika termometru

//stałe ustawiające czas pracy swiatla
#define lightStartHour 17  // tutaj podaj godzinę załączenia swiatla
#define lightStartMinute 00  // tutaj poadaj minute zalaczenia swiatla
#define lightStopHour 22      // tutaj podaj godzinę wyłączenia swiatla
#define lightStopMinute 00    // tutaj poadaj minute wylaczenia swiatla

//stałe ustawiające temperature i histereze grzalki
#define setTemperature 24  //tutaj ustaw temperature wylaczenia grzalki
#define setHisteresis 1    //ustaw temperature histerezy

//defiiuje adres zegara - nie nalezy zmieniac
#define DS3231_I2C_ADDRESS 0x68

//definiuje piny pod ktore wpiety jest wyswietlacz LCD
LiquidCrystal lcd( 4, 5, 6, 7, 8, 9);

//uruchamia znaczek migajacy miedzy godzina a minuta
boolean blinkOn = true; //visibility of ':' between hour and minutes

// Adres czujnika temperatury DS18B20 - kazdy czujnik ma unikalny aby go poznac nalezy go sczytać za pomocą specjalnego programu
byte address[8] = {0x28, 0x15, 0xFE, 0x8A, 0xA, 0x0, 0x0, 0x2};

OneWire onewire(ONEWIRE_PIN);     //Deklaruje obiekt onewire odpowiedzialny za komunikację za pomocą protokołu 1-Wire. W argumencie konstruktora podajesz numer pinu do którego podłaczone są urządzenia magistrali 1-Wire.
DS18B20 sensors(&onewire);        //Deklaruje obiekt sensors odpowiedzialny za obsługę czujników DS18B20. W argumecie konstruktora przekazywany jest wskaźnik do obiektu magistrali 1-Wire do której podłączony jest termometr.

byte znakStopniaCelsjusza[8] = {B11100, B10100, B11100, B00000, B00000, B00000, B00000, B00000};
        // definiujemy znak graficzny stopnia Celsjusza w celu wyświetlenia na lcd


//convert normal decimal numbers to binary coded decimals
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}
//convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}


/****************************************FUNKCJA SETUP INICJOWANA RAZ NA STARCIE PROGRAMU*****************************************************************************************/
void setup() {
  pinMode(PIN_LIGHT, OUTPUT); // ustawienie pinu odpowiedzialnego za obsluge swiatla
  pinMode(PIN_HEATING, OUTPUT); // ustawienie pinu odpowiedzialnego za obsluge swiatla

  Wire.begin();                 //nawiazanie lacznosci za pomoca magistrali z zegarem
  lcd.begin(16,2); //(col, rows) - inicjacja LCD

  //Uzyj ponizszej zakomentowanje funkcji setRTC tylko raz na poczatku uzywania sterownika w celu ustawienia zegara 
  //odkomentuj linijke setRTCTime tzn. usun dwa ukosnik zaczynajace wiersz i wpisz aktualna godzine i date wedlug ponizszgo schematu
  //schemat (sekundy (zostaw zero), min, godzina, dzien tygodnia, dzien miesiaca, miesiac, rok)
  //nastepnie skompiluj i wgraj program, po czym zakomentuj z powrotem i wgraj ponownie 
  //setRTCTime(0, 20, 14, 7, 18, 11, 18);

  sensors.begin(9);          //Metoda begin wyszukuje czujniki i ustawia we wszystkich rozdzielczość. Domyślnie BEZ ARGUMENTÓW jest to 12 bitów W tym przypadku jest to 10 bitów.
  sensors.request(address);   //Metoda request wysyła rozkaz do czujnika o adresie podanym w argumencie, by zaczął obliczać temperaturę.
    
lcd.createChar(1, znakStopniaCelsjusza); // przypisanie wcześniej zdefiniowanego znaku do 1


}




/***************************************************************************************************************************************************************************/
void checkTemperature()
{
   if (sensors.available())     //Metoda available sprawdza czy czujnik już obliczył temperaturę. Jeśli tak, to zwraca wartość true, jeśli nie to false. Metoda ta pozwala nie czekać na obliczenia czujnika, tylko wykonywać program dalej, a gdy wynik obliczeń się pojawi, to go odczytać.
  {
    float temperature = sensors.readTemperature(address);


//    Serial.print(temperature);
//    Serial.println(F(" 'C"));
    lcd.setCursor(8, 0);    // ustawienie kursora, w zadanej pozycji (kolumna, wiersz)
    lcd.print(temperature);    // wyświetlenie wartości zmiennej temp
    lcd.write(1);             // metoda wrtite() wyswietla zdefiniowany wyzej znak temperatury
    lcd.print("C");
//    Serial.print("test ");
sensors.request(address);
  }
}

//set the time and date to the RTC
void setRTCTime(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

//read the time and date from the RTC
void readRTCTime(byte *second, byte *minute, byte *hour, byte *dayOfWeek,byte *dayOfMonth, byte *month, byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

  void light()
{   
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    
    int currentTimeInMinute; 
    int starTimeInMinute, stopTimeInMinute;
  
  // odczyt czasu z RTC  z użyciem adresów wskaźników
  readRTCTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  currentTimeInMinute = hour * 60 + minute; //aktualny czas w minutach potrzebny do porównań  poniżej

  starTimeInMinute = lightStartHour * 60 + lightStartMinute;  //czas załączenia swiatla w minutach
  stopTimeInMinute = lightStopHour * 60 + lightStopMinute;    // czas wyłaczenia swiatla 
  
  if (currentTimeInMinute >= starTimeInMinute && currentTimeInMinute < stopTimeInMinute)
  {
   digitalWrite(PIN_LIGHT, HIGH);   // turn the LED on (HIGH is the voltage level)       
  }else{
    digitalWrite(PIN_LIGHT, LOW);   // turn the LED on (HIGH is the voltage level)       
  }
//  delay(100);
}

//easy and dirty way to clear the LCD
void clearLCD ()
{
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,0);
}



//reads the RTC time and prints it to the top of the LCD
void printTime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  
  // odczyt czasu z RTC  z użyciem adresów wskaźników
  readRTCTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  
  //print to lcd top
  lcd.setCursor(0,0);
  //lcd.print(" Godzina: ");
  if (hour<10)
  {
    lcd.print("0");
  }
  lcd.print(hour, DEC);
  if (blinkOn == true)
  {
    lcd.print(" ");
    blinkOn = false;
  }
  else if (blinkOn == false)
  {
    lcd.print(":");
    blinkOn = true;
  }
  if (minute<10)
  {
    lcd.print("0");
  }
  lcd.print(minute, DEC);

  delay(500);
}
void printLightTime()
{
  lcd.setCursor(0,1);
  if (lightStartHour < 10)
  {
    lcd.print("0");
  }
  lcd.print(lightStartHour, DEC);
  lcd.print(":");
    if (lightStartMinute < 10)
  {
    lcd.print("0");
  }
    lcd.print(lightStartMinute, DEC);
     lcd.print("-");
     
  if (lightStopHour < 10)
  {
    lcd.print("0");
  }
  lcd.print(lightStopHour, DEC);
  lcd.print(":");
    if (lightStopMinute < 10)
  {
    lcd.print("0");
  }
    lcd.print(lightStopMinute, DEC);   

}

void printSetTemp()
{
  lcd.setCursor(12,1);
  lcd.print(setTemperature, DEC);
  lcd.write(1);             // metoda write() wyswietla zdefiniowany wyzej znak temperatury
  lcd.print("C");
}


void onOfHeater()
{
  float aktualTemp = sensors.readTemperature(address);
  if (aktualTemp <= setTemperature)
  {
   digitalWrite(PIN_HEATING, HIGH);   // turn the LED on (HIGH is the voltage level)       
  }  
  if (aktualTemp >= setTemperature + setHisteresis)
  {
    digitalWrite(PIN_HEATING, LOW);   // turn the LED on (HIGH is the voltage level)       
  } 
  
}
/*****************************************FUNKCJA LOOP WYKONYWANA W PETLI CALY CZAS****************************************************************************************/
void loop() {
    checkTemperature();
    printTime();
    printLightTime();
    printSetTemp();
    onOfHeater();
}




  