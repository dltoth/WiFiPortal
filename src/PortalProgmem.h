
/**
 *   Template for finishConnect web page. Uses inline stlyes rather than a separate styles link as all other
 *   pages do. 
 */

#ifndef PORTALPROGMEM_H
#define PORTALPROGMEM_H

namespace lsc {
const char AP_NAME[]                    = "SleepingBear";
const char AP_PSK[]                     = "BigLakeMI"; 
const char AP_pskEntry[]        PROGMEM = "<form action=\"%s\">"                                                                                           // form submit path
                                             "<div align=\"center\">"
                                                 "<label for=\"psk\">PassKey &nbsp &nbsp &nbsp &nbsp</label>"
                                                 "<input type=\"text\" size=\"30\" placeholder=\" Enter PassKey for %s \" name=\"psk\" required><br><br>"  // SSID
                                                 "<button class=\"fmButton\" type=\"submit\">OK</button> &nbsp &nbsp"
                                                 "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>"     // form cancel path
                                                 "<input type=\"hidden\" name=\"ssid\" id=\"ssid\" value=\"%s\">"                                          // SSID
                                             "</div></form>";
const char AP_form[]            PROGMEM = "<form action=\"%s\">"                                                                                           // form submit path
                                             "<div align=\"center\">"
                                                 "<label for=\"psk\">PassKey &nbsp &nbsp &nbsp &nbsp</label>"
                                                 "<input type=\"text\" size=\"30\" placeholder=\" Enter PassKey for %s \" name=\"psk\" required><br><br>"  // SSID
                                                 "<label for=\"hostName\">Host Name &nbsp &nbsp</label>"
                                                 "<input type=\"text\" size=\"30\" placeholder=\" Enter Host Name (Optional) \" name=\"hostName\"><br><br>"
                                                 "<button class=\"fmButton\" type=\"submit\">Submit</button> &nbsp &nbsp"
                                                 "<button class=\"fmButton\" type=\"button\" onclick=\"window.location.href=\'%s\';\">Cancel</button>"   // form cancel path
                                                 "<input type=\"hidden\" name=\"ssid\" id=\"ssid\" value=\"%s\">"                                        // SSID
                                                 "<input type=\"hidden\" name=\"hash\" id=\"hash\" value=\"%s\">"                                        // Hash
                                                 "<input type=\"hidden\" name=\"salt\" id=\"salt\" value=\"%s\">"                                        // Salt
                                             "</div></form>";

/**
 *   Web pages for retry and finishConnect must include styles because the connection is reset after the page is sent.
 *   A client could not come back to fetch styles.css
 */
const char APfinish_connect[]   PROGMEM = "<!DOCTYPE html><html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
  "<head>"
  "<style type=\"text/css\">"
     ".apButton {"
     "background:linear-gradient(to bottom, #ededed 5%%, #bab1ba 100%%);" 
     "background-color:#ededed;"
     "border-radius:12px;"
     "border:1px solid #d6bcd6;" 
     "display:block;" 
     "cursor:pointer;" 
     "color:#3a8a9e;" 
     "font-family:Arial;" 
     "font-size: 1.2em;"   
     "padding: .5em;" 
     "width: 100%%;" 
     "text-decoration:none;" 
     "margin: 0px auto 3px auto;"                                                  
     "text-shadow:0px 1px 0px #e1e2ed;" 
     "text-align: center;" 
   "}" 
   ".apButton:hover {" 
     "background:linear-gradient(to bottom, #bab1ba 5%%, #ededed 100%%);" 
     "background-color:#bab1ba;" 
   "}" 
   ".apButton:active {" 
     "position:relative;" 
     "top:1px;" 
   "}"  
   "[class*=\"small\"]  {" 
     "width: 20%%;"
     "display: inline-block;" 
   "}"
   "@media only screen and (min-width: 768px) {" 
     ".small {width: 10%%;}"                       
   "}" 
"</style>"
"</head>" 
    "<body style=\"font-family: Arial\">" 
       "<H1 align=\"center\">Connect to %s?</H1><br><br>"
          "<div align=\"center\">"
             "<a href=\"%s\" class=\"small apButton\">Connect</a>&nbsp&nbsp" 
             "<a href=\"%s\" class=\"small apButton\">Cancel</a>"  
          "</div>"
    "</body>"
"</html>";

const char AP_retry[]   PROGMEM = "<!DOCTYPE html><html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
  "<head>"
  "<style type=\"text/css\">"
     ".apButton {"
     "background:linear-gradient(to bottom, #ededed 5%, #bab1ba 100%);" 
     "background-color:#ededed;"
     "border-radius:12px;"
     "border:1px solid #d6bcd6;" 
     "display:block;" 
     "cursor:pointer;" 
     "color:#3a8a9e;" 
     "font-family:Arial;" 
     "font-size: 1.2em;"   
     "padding: .5em;" 
     "width: 100%;" 
     "text-decoration:none;" 
     "margin: 0px auto 3px auto;"                                                  
     "text-shadow:0px 1px 0px #e1e2ed;" 
     "text-align: center;" 
   "}" 
   ".apButton:hover {" 
     "background:linear-gradient(to bottom, #bab1ba 5%, #ededed 100%);" 
     "background-color:#bab1ba;" 
   "}" 
   ".apButton:active {" 
     "position:relative;" 
     "top:1px;" 
   "}"  
   "[class*=\"small\"]  {" 
     "width: 20%;"
     "display: inline-block;" 
   "}"
   "@media only screen and (min-width: 768px) {" 
     ".small {width: 10%;}"                       
   "}" 
"</style>"
"</head>" 
    "<body style=\"font-family: Arial\">" 
       "<H1 align=\"center\">Connection Attempt FAILED!</H1><br><br>"
           "<div align=\"center\"><a href=\"/\" class=\"small apButton\">Retry</a></div>"
    "</body>"
"</html>";

} // End of namespace lsc

#endif
