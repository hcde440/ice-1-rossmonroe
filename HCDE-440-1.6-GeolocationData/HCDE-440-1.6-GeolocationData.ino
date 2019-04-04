/*ICE #1A
   Using HCDE_IoT_4_GeoLocation.ino as a starting point, augment the sketch to include
   a new data type (a struct) called MetData designed to contain name:value pairs from
   http://openweathermap.org/. You will need to get an API key to use this service, the
   key is free for low volume development use. Get your key at http://openweathermap.org/api
   by subscribing to the Current Weather Data api. Examples of the API call are provided in
   the API documnentation.
   Wrap your HTTP request in a function called getMet(), which will be similar to getIP()
   and getGeo(), and print out the data as a message to the Serial Monitor using Imperial
   units. You should report temperature, humidity, windspeed, wind direction, and cloud
   conditions. Call the API by city name.
   
   One possible solution is provided below, though you should try to find your own solution
   first based upon previous examples.
*/

/*ICE #1B
   To complete this ICE, demonstrate your understanding of this sketch by running
   the code and providing complete inline commentary, describing how each line of
   code functions in detail. The completed project will comment every line. Some of
   this has already been done for you in earlier progressions of this project.
   This sketch . . . (briefly relate what this sketch does here)
   
   If your code differs from this solution, fine. Just comment that code thoroughly.
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Ross";
const char* pass = "bullshit";
const char* key = "99519d3788a270f03d0a9a3ad5070921";
String weatherKey = "98825c9c60142b038f2e0d111694a5fe";

typedef struct {
  String ip;
  String cc;
  String cn;
  String rc;
  String rn;
  String cy;
  String tz;
  String ln;
  String lt;
} GeoData;

typedef struct {
  String tp;
  String pr;
  String hd;
  String ws;
  String wd;
  String cd;
} MetData;

GeoData location;
MetData conditions;

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.print("Connecting to "); Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(); Serial.println("WiFi connected"); Serial.println();
  Serial.print("Your ESP has been assigned the internal IP address ");
  Serial.println(WiFi.localIP());

  String ipAddress = getIP();
  getGeo(ipAddress);

  Serial.println();

  Serial.println("Your external IP address is " + location.ip);
  Serial.print("Your ESP is currently in " + location.cn + " (" + location.cc + "),");
  Serial.println(" in or near " + location.cy + ", " + location.rc + ".");
  Serial.print("You are in the " + location.tz + " timezone ");
  Serial.println("and located at (roughly) ");
  Serial.println(location.lt + " latitude by " + location.ln + " longitude.");

  getMet(location.cy);

  Serial.println();
  Serial.println("With " + conditions.cd + ", the temperature in " + location.cy + ", " + location.rc);
  Serial.println("is " + conditions.tp + "F, with a humidity of " + conditions.hd + "%. The winds are blowing");
  Serial.println(conditions.wd + " at " + conditions.ws + " miles per hour, and the ");
  Serial.println("barometric pressure is at " + conditions.pr + " millibars.");
}

void loop() {
}

String getIP() {
  HTTPClient theClient;
  String ipAddress;

  theClient.begin("http://api.ipify.org/?format=json");
  int httpCode = theClient.GET();

  if (httpCode > 0) {
    if (httpCode == 200) {

      DynamicJsonBuffer jsonBuffer;

      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);
      ipAddress = root["ip"].as<String>();

    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
      return "error";
    }
  }
  return ipAddress;
}


void getGeo(String ipAdress) {
  HTTPClient theClient;
  Serial.println("Making HTTP request");
  theClient.begin("http://api.ipstack.com/" + ipAdress + "?access_key=" + key); //return IP as .json object
  int httpCode = theClient.GET();

  if (httpCode > 0) {
    if (httpCode == 200) {
      Serial.println("Received HTTP payload.");
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      Serial.println("Parsing...");
      JsonObject& root = jsonBuffer.parse(payload);

      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return;
      }

      //Some debugging lines below:
      //      Serial.println(payload);
      //      root.printTo(Serial);

      //Using .dot syntax, we refer to the variable "location" which is of
      //type GeoData, and place our data into the data structure.

      location.ip = root["ip"].as<String>();            //we cast the values as Strings b/c
      location.cc = root["country_code"].as<String>();  //the 'slots' in GeoData are Strings
      location.cn = root["country_name"].as<String>();
      location.rc = root["region_code"].as<String>();
      location.rn = root["region_name"].as<String>();
      location.cy = root["city"].as<String>();
      location.lt = root["latitude"].as<String>();
      location.ln = root["longitude"].as<String>();

    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
}

void getMet(String city) {  //City variable is being passed into the function
  HTTPClient theClient;
  String apiCall = "http://api.openweathermap.org/data/2.5/weather?q=" + city;
  apiCall += "&units=imperial&appid=";
  apiCall += weatherKey;
  theClient.begin(apiCall);
  int httpCode = theClient.GET();
  if (httpCode > 0) {

    if (httpCode == HTTP_CODE_OK) {
      String payload = theClient.getString();
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (!root.success()) {
        Serial.println("parseObject() failed in getMet().");
        return;
      }
      conditions.tp = root["main"]["temp"].as<String>(); //casting the conditions as strings so they can be recieved by the struct
      conditions.pr = root["main"]["pressure"].as<String>();
      conditions.hd = root["main"]["humidity"].as<String>();
      conditions.cd = root["weather"][0]["description"].as<String>();
      conditions.ws = root["wind"]["speed"].as<String>();
      int deg = root["wind"]["deg"].as<int>();
      conditions.wd = getNSEW(deg);
    }
  }
  else {
    Serial.printf("Something went wrong with connecting to the endpoint in getMet().");
  }
}

String getNSEW(int d) {
  String direct;

  //Conversion based upon http://climate.umn.edu/snow_fence/Components/winddirectionanddegreeswithouttable3.htm
  if (d > 348.75 && d < 360 || d >= 0  && d < 11.25) {
    direct = "north";
  };
  if (d > 11.25 && d < 33.75) {
    direct = "north northeast";
  };
  if (d > 33.75 && d < 56.25) {
    direct = "northeast";
  };
  if (d > 56.25 && d < 78.75) {
    direct = "east northeast";
  };
  if (d < 78.75 && d < 101.25) {
    direct = "east";
  };
  if (d < 101.25 && d < 123.75) {
    direct = "east southeast";
  };
  if (d < 123.75 && d < 146.25) {
    direct = "southeast";
  };
  if (d < 146.25 && d < 168.75) {
    direct = "south southeast";
  };
  if (d < 168.75 && d < 191.25) {
    direct = "south";
  };
  if (d < 191.25 && d < 213.75) {
    direct = "south southwest";
  };
  if (d < 213.25 && d < 236.25) {
    direct = "southwest";
  };
  if (d < 236.25 && d < 258.75) {
    direct = "west southwest";
  };
  if (d < 258.75 && d < 281.25) {
    direct = "west";
  };
  if (d < 281.25 && d < 303.75) {
    direct = "west northwest";
  };
  if (d < 303.75 && d < 326.25) {
    direct = "south southeast";
  };
  if (d < 326.25 && d < 348.75) {
    direct = "north northwest";
  };
  return direct;
}
