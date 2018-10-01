#include <Adafruit_GPS.h> //Carga  la libreria GPS. Se encarga que sea instalado la libreria del sitio Adafruit
Adafruit_GPS GPS(&Serial1); //Crea un Objeto GPS.
String NMEA1;  //Se usara para mantener la primera oracion NMEA
String NMEA2;  //Se usara para mantener la segunda oracion NMEA
String url;    //Variable que contendra el HTTP GET request 
char c;        //Usado para leer los caracteres iniciales del modulo GPS
//Variables para ajustar los datos recibidos del modulo GPS
int hora , dia , latitudgrad , latitudmin , latsegaux , latseg ;
int latmiliseg , longitudgrad , longitudmin , lonsegaux , lonseg , lonmiliseg;
int altitud , mes , year;
float latitudseg;
float longitudseg;
//Variables para ajustar los datos recibidos del modulo GPS
// Pines auxiliares
int GPSLedPin = 13;
int GPRSLedPin = 12;
int sel = 11;
// Pines auxiliares
void setup()  
{
  Serial.begin(115200);  //Turn on the Serial Monitor
  GPS.begin(9600);       //Turn GPS on at baud rate of 9600
  Serial.println(F("Turn GPRS module On"));
  delay(10000);
  Serial.println(F("Begin setup"));
  pin_setup();          //Configuracion de los pines auxiliares
  gprsSetup();          //Configuracion del modulo GPRS
  gpsSetup();           //Configuracion del modulo GPS
  gps_fix();            //Obtencion del mejor ajuste de las coordenas GPS
  Serial.println(F("Setup OK"));
  delay(1000);  //Pausa de 1 segundo
}
void loop()             // Se ejecuta una y otra vez
{
  Serial.println(F("Begining Transmision"));
  gpsParse();

  Serial.print("velocidad: ");
  Serial.println(GPS.speed);

  Serial.print("Numero de satelites: ");
  Serial.println(GPS.satellites);
  if ( GPS.satellites != 0 ) // Si el GPS se encuentra en comunicacion de al menos 1 satelite
  {
    url_setup();       //Ejecuta la eleboracion de la solicitud HTTP GET 
    http_get();        //Ejecuta el envio de los datos
  } 
  else{
        gps_fix();
      }
  Serial.println(F("OK"));
  if(GPS.speed > 30){ //mayor a 55,56km/h
  delay(103000);   //actualizaciones cada 2 min
  }else{
    delay(23200); //cada 40,2 segundos
    }
}
void gps_fix() {
  digitalWrite(GPSLedPin, LOW);                   // Apaga el Led GPS cuando realiza un Fix(mejora de las coordenadas)
  Serial.println(F("Getting GPS Fix"));              // Se imprime por pantalla "Getting GPS Fix"
  int fixled;                                     // Almacena lo leido en el pin  FIX correspondiente al pin analogico A0
  int fix = 0;                                    // Contador
  while (fix < 12) {                              // Si fix > 12, El FIX led esta apagadopor mas de 1 segundo y se ha obtenido una mejora
    fixled = analogRead(A0);                      // Lectura analogica del 'fixled'
    if ( fixled > 100 ) {                         // Chequea si 'fixled' esta por encima de 100 (por debajo estaria a 0.5 V). Si lo es, eta encendido.
      fix = 0;                                    // Restaura 'fix' a 0 para continuar el loop
    }
    else {                                        // Si FIX led esta apagado 
      fix++;                                      // Incrementa 'fix'
    }
    delay(100);                                   // Espera 0.1 segundos
  }
  Serial.println(F("Fix obtained"));                 // Fix obtenido
  digitalWrite(GPSLedPin, HIGH);                  // Enciende el GPS LED cuando el fix GPS sea exitoso
}
void toSerial() 
{
  Serial1.flush();                                // Vacia el puerto Serial1(Pines 0_RX y 1_TX) 
  Serial.flush();                                 // Vacia el puerto Serial(Comunicacion serial a traves del puerto USB Client)
  while(Serial1.available()!=0)                   // Revisa si Serial1 tiene informacion disponible y no nula 
  {                 
    Serial.write(Serial1.read());                 // Imprime la informacion disponible del Serial1 en el puero Serial
  }
}
void pin_setup() 
{
  pinMode(GPSLedPin, OUTPUT);                       // Inicializa GPS LED
  pinMode(GPRSLedPin, OUTPUT);                      // Inicializa GPRS LED
  pinMode(sel, OUTPUT);                             // Inicializa pin 'sel' como pin de salida 

  Serial.println(F("Pins set up")); 
  delay(1000);                                      // Espera 1 segundo
}
void readGPS(){                                     //Esta funcion leera y recordara las dos oraciones del modulo GPS
  clearGPS();                                       //El puerto Serial probablemente tendra data vieja y corrupta, entonces se comienza por limpiarlo por completo
  while(!GPS.newNMEAreceived()) {                   //Se mantiene leyendo caracteres en el loop mientras unsa buena oracion NMEA es recibida
  c = GPS.read();                                     //lee un caracter del GPS
  }
  GPS.parse(GPS.lastNMEA());                        //Una vez conseguido una buena NMEA, se analiza
  NMEA1 = GPS.lastNMEA();                             //Una vez analizada, se salva la oracion NMEA en la variable NMEA1
  while(!GPS.newNMEAreceived()) {                   //SE sale y recive la segunda oracion NMEA, debe ser de diferente tipo que la primera leida arriba
  c = GPS.read();
  }
  GPS.parse(GPS.lastNMEA());                        //Una vez conseguido una buena NMEA, se analiza
  NMEA2 = GPS.lastNMEA();                             //Una vez analizada, se salva la oracion NMEA en la variable NMEA1
  Serial.println(NMEA1);                            //Muestra en pantalla la primera oracion NMEA
  Serial.println(NMEA2);                            //Muestra en pantalla la segunda oracion NMEA
  Serial.println(F(""));
}
void clearGPS() {                                   //Mientras que se lee el GPS, todavia tenemos data transmitiendose a la tarjeta Galileo, 
  while(!GPS.newNMEAreceived()) {                     //necesitamos limpiar las data vieja leyendo unas pocas oraciones y descartandolas
    c=GPS.read();
    }
  GPS.parse(GPS.lastNMEA());
  while(!GPS.newNMEAreceived()) {
    c=GPS.read();
    }
  GPS.parse(GPS.lastNMEA());
   
}
void gpsSetup()
{
  Serial.println(F("Initializing GPS"));             //Se imprime por monitor            
  digitalWrite(sel, HIGH);                        //Se enciende el modo GPS   
//  Serial.print(F("Selector: "));                     //Se muestra el estado del selector
//  selector();

  GPS.sendCommand(PMTK_SET_BAUD_9600);              
  GPS.sendCommand(PGCMD_NOANTENNA);              // Se apaga la antena de actualizacion GPS(Antena eterna que no se posee)
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);   // Se le dice al modulo GPS que queremos solo las oraciones NMEA $GPRMC y $GPGGA 
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_100_MILLIHERTZ);     // la tasa de actualizacion se configura a 10 Hz
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_100_MILLIHERTZ);
  

  Serial.println(F("GPS set"));
}
void gpsParse()
{   
  Serial.println(F("Initializing parse"));           // Se notifica por pantalla el procedimiento
  digitalWrite(sel, HIGH);                        // Se coloca 'sel' en HIGH (operacion GPS)


  readGPS();                                      // Esta funcion que se define lee las dos oraciones NMEA tomadas del modulo GPS
  readGPS();                                                                          
  
  if(GPS.hour >= 0 && GPS.hour < 4){              // A continuacion, se utilizand las variables globales antes definidas para adecuar la hora a Venezuela.
                                dia = GPS.day - 1;    // en este rango de 4 horas la hora esta adelantada en 1 dia
                                hora = 20 + GPS.hour; // Ademas, de estar en formato de 24 horas, se reinicia a 0 cuando es media noche
                                mes = GPS.month;      // El mes no se adelanta 
                                year = GPS.year;      // El año no se adelanta

                                  // A partir de aca se verifica que la informacion del dia a enviar sea la correcta, 
                                  // debido a que cuando se finaliza el mes tambien se adelantaria en una unidad,
                                  // al igual que en el fin del año. Tambien, se toma el caso de febrero donde 
                                  // es biciesto y cuando no, suministrando la informacion correcta.
                                  if ((GPS.month == 12 || GPS.month == 3 || GPS.month == 5 || GPS.month == 7 || 
                                  GPS.month == 8 || GPS.month == 10) && (GPS.day == 31) )
                                  {
                                    dia = 30 + GPS.day;
                                    mes = GPS.month - 1;
                                  }
                                  
                                  if ((GPS.month == 4 || GPS.month == 6 || GPS.month == 9 || GPS.month == 11)
                                  && (GPS.day == 30) )
                                  {
                                    dia = 29 + GPS.day;
                                    mes = GPS.month - 1;
                                  }
                                  if(GPS.month == 2)
                                  {
                                    if((2000 + GPS.year) % 4 == 0 && (GPS.year + 2000) % 100 == 0 && (GPS.year + 2000) % 400 == 0)
                                    {
                                      dia = GPS.day + 27;
                                    }
                                     if(GPS.day == 29 )
                                    {
                                      dia=GPS.day+28;
                                    }
                                    mes = GPS.month - 1;
                                  } 
                                  if(GPS.month == 1 && GPS.day == 1)
                                  {
                                    dia = GPS.day + 30;
                                    mes = GPS.month + 11;
                                    year = GPS.year - 1;
                                  } 
                                
                                }             
  else{                                               //de no estar en ese rango de horas y casos especiales, solo se restan 4 horas a la hora
        dia = GPS.day;
        hora = GPS.hour-4;
        mes = GPS.month;
        year = GPS.year;
      }
  // se manipula los datos de latitud y longitud para darle el formato correcto a ser enviado y guardado en el servidor
  latitudgrad = GPS.latitude / 100;
  latitudmin = GPS.latitude - latitudgrad * 100;
  latitudseg = ( GPS.latitude * 10000 - ( latitudgrad * 1000000 + latitudmin * 10000 ) ) * 60;
  latsegaux = latitudseg;
  latseg = latsegaux/10000;
  latmiliseg = latsegaux - (latseg * 10000);
  longitudgrad= GPS.longitude / 100;
  longitudmin= GPS.longitude - longitudgrad * 100;
  longitudseg = (  GPS.longitude * 10000  - ( longitudgrad * 1000000 + longitudmin * 10000 )   ) * 60;
  lonsegaux = longitudseg;
  lonseg = lonsegaux/10000;
  lonmiliseg = lonsegaux - (lonseg * 10000);
  altitud= GPS.altitude;
  
  Serial.print(F("Latitude in degrees: "));
  Serial.print(latitudgrad);
  Serial.print(F(" "));
  Serial.print(latitudmin);
  Serial.print(F("' "));
  Serial.print(latseg);
  Serial.print(F("'' "));
  Serial.print(latmiliseg);
  Serial.print(F(" mseg, "));
  Serial.print(F(" "));
  Serial.println(GPS.lat);
  Serial.print(F("Longitude in degrees: "));
  Serial.print(longitudgrad);
  Serial.print(F(" "));
  Serial.print(longitudmin);
  Serial.print(F("' "));
  Serial.print(lonseg);
  Serial.print(F("'' "));
  Serial.print(lonmiliseg);
  Serial.print(F(" mseg, "));  
  Serial.print(F(" "));
  Serial.println(GPS.lon);
  Serial.print(F("Altitude: "));
  Serial.print(GPS.altitude);
  Serial.println(F(" M"));
  Serial.print(F("Hour: "));
  Serial.print(hora);
  Serial.print(F(":"));
  Serial.print(GPS.minute);
  Serial.print(F(":"));
  Serial.println(GPS.seconds);
  Serial.print(F("Date: "));
  Serial.print(dia);
  Serial.print(F("/"));
  Serial.print(mes);
  Serial.print(F("/"));
  Serial.println(year);

  Serial.println(F("Parse finished"));
  Serial.println(F(""));
}
void gprsSetup()
{
  Serial.println(F("Initializing GPRS module"));
  digitalWrite(sel, LOW);                           // Se coloca 'sel' en LOW (operacion GPRS)
  Serial1.flush();
  Serial.flush();
  
//  Serial.print(F("Selector: "));
//  selector();
  
  Serial1.println("AT+CGATT?");                    // Se consulta el vinculo al servicio GPRS
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo

  Serial1.println("AT+CGDCONT?");                  // Consulta el contexto PDP (Packet Data Protocol)  
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo

  Serial1.println("AT+CSCLK=2\r\n");               // Configura el reloj lento, cuando el modulo no reciba nada entonces entrara en modo SLEEP, cuando reciba despertara
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo
  
  Serial1.print("AT+CGDCONT=1,\"IP\",\"internet.movistar.ve\",\"\",0,0\r\n"); //Configura el Contexto PDP
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo

  Serial1.println("AT+IPR=9600");                  // Configura la tasa de transmision del modulo a 9600 baudios 
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo 

  Serial1.println("AT+CGATT?");                    // Se consulta el vinculo al servicio GPRS 
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo
  
  Serial1.println("AT+CFUN=1\r\n");                // Configura la funcionalidad del modulo GPRS como completa. 
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo

  Serial1.println("AT+CPIN?\r\n");                 // Pregunta el estado del PIN, como la tarjeta sim esta desbloqueada no pregunta por ninguna y contesta con READY
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo
  
  Serial1.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n"); // configuracion de la portadora(Bearer)
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo
  
  Serial1.println("AT+SAPBR=3,1,\"APN\",\"internet.movistar.ve\"\r\n"); // Configuracion del Access Point Network (APN)
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo
  
  Serial1.println("AT+SAPBR=1,1\r\n");             // Establecimiento de la conexion a la configuracion de portadora para aplicaciones basadas en IP
  delay(3000);                                     // Espera de 3 segundos
  toSerial();                                      // Enviar data al monitor Serial

  Serial1.println("AT+CSQ\r\n");                   // Pregunta por la calidad de la señal. Se recibe el rssi y el RXQUAL referente a la norma GSM 05.08 tabla 8.2.4
  delay(1000);                                     // Espera de 1 segundo
  toSerial();                                      // Enviar data al monitor Serial
  delay(1000);                                     // Espera de 1 segundo
  
  digitalWrite(GPRSLedPin, HIGH);                  // Enciende el LED GPRS cuando la configuracion esta lista
  Serial.println(F("GPRS set"));
}
void url_setup() {
  //Solicitud GET
  url = "AT+HTTPPARA=\"URL\",\"http://luis-ucab.herokuapp.com/transporte/AA113BB/?lat=";
  // Concatenacion de la data GPS                      
  url.concat(latitudgrad);
  url.concat("g");
  url.concat(latitudmin);
  url.concat("m");
  url.concat(latseg);
  url.concat("s");
  url.concat(latmiliseg);
  url.concat("mili");
  url.concat("&lon=");
  url.concat(longitudgrad);
  url.concat("g");
  url.concat(longitudmin);
  url.concat("m");
  url.concat(lonseg);
  url.concat("s");
  url.concat(lonmiliseg);
  url.concat("mili");
  url.concat("&hour=");
  url.concat(hora);
  url.concat("h");
  url.concat(GPS.minute);
  url.concat("m");
  url.concat(GPS.seconds);
  url.concat("s");
  url.concat("&date=");
  url.concat(dia);
  url.concat("Dd"); 
  url.concat(GPS.month);
  url.concat("Mm"); 
  url.concat(GPS.year);
  url.concat("Yy");
  url.concat("\"\r\n");
  Serial.println(F(""));
  Serial.println(url);
  Serial.println(F(""));
}
void http_get() {
  digitalWrite(sel, LOW);                   // Coloca el pin 'sel' en LOW (operacion GPRS)
 

  Serial1.flush();
  Serial.flush();
  
//  Serial.print(F("Selector: "));
//  selector();

  delay(1000);                              // Espera de 1 segundo
  Serial1.println("AT+HTTPINIT\r\n");       // Inicializa servicio HTTP
  delay(2000);                              // Espera de 2 segundo
  toSerial();                               // Enviar data al monitor Serial
  
  Serial1.println("AT+HTTPPARA=\"CID\",1\r\n"); // Configura parametros CID
  delay(2000);                              // Espera de 2 segundos
  toSerial();                               // Enviar data al monitor Serial                         
  
  Serial1.println(url);                     // Configura URL
  delay(2000);                              // Espera de 2 segundos
  toSerial();                               // Enviar data al monitor Serial

  Serial1.println("AT+HTTPACTION=0\r\n");   // Configura el tipo de accion HTTP
  delay(6000);                              // Espera de 6 segundos
  toSerial();                               // Enviar data al monitor Serial
  Serial1.println("");

  Serial1.println("AT+HTTPREAD\r\n");       // Lee la respuesta del servidor WEB
  delay(2000);                              // Espera de 2 segundo
  toSerial();                               // Enviar data al monitor Serial
  Serial1.println("");
  
  Serial1.println("AT+HTTPTERM\r\n");       // Finaliza el servicio HTTP
  toSerial();                               // Enviar data al monitor Serial
  delay(300);                               // Espera de 0.3 segundo
  Serial1.println(F(""));
  delay(1000);                              // Espera de 1 segundo
}
//void selector(){
//  if (sel==11){
//    Serial.println("GPS");}
//    else{
//          Serial.println("GPRS");
//        }
//  }
