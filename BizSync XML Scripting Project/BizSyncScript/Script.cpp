/**
   --------------------------------------------------------------
   | Copyright 2015 Patio Pacific. All rights reserved.         |
   | Use of this source code is governed by a BSD-style license |
   | that can be found in the LICENSE file.                     |
   --------------------------------------------------------------

Before you dive into this script, you should probably review two things:
1. The SOP on the BizSync script.
2. Go look at one of the order.xml files (you can find hundreds in the backup folder on the remote server)
The source code will make more sense if you do (maybe)

*/


#include "tinyxml2.h"
#include <string>
#include <cstring>
#include <cctype>
#include <ctime>
#include <stdlib.h>
#include <windows.h>
#include <fstream>
using namespace std;

#define NUM_PRODUCTS_TO_CHECK_FOR 12
const char* badProductID[] = {"12BW", "fg", "01HS", "01PP", "01RI", "06HS", "02SS", "02DD", "19PS01", "19HSIFC", "04HS90", "02CW"};



//Virtual Functions
void setHold(tinyxml2::XMLNode *Node);

/** Function that returns true if string base starts
 *  with string sequence.
 *  Example:
 *  Base = "This is a test"
 *  Sequence = "Thi"
 */
bool startsWith(const char* base, char* sequence) {
    if(strlen(sequence) > strlen(base)) {
        return false;
    }

    for(int i = 0; i < strlen(sequence); i++) {
        if(base[i] != sequence[i]) {
            return false;
        }
    }
    return true;
}

/** Returns the time in a nice format.
 */

char* getTime(const struct tm *timeptr) {
  static char result[26];
  sprintf(result, "%.2d:%.2d",
    timeptr->tm_hour, timeptr->tm_min);
  return result;
}

/** A function I wrote to help name the backup files.
 *  Same premise as above.
 */

char* myAsctime(const struct tm *timeptr) {
  static const char mon_name[][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  static char result[26];
  sprintf(result, "%.3s%2d-%.2d.%.2d-%d",
    mon_name[timeptr->tm_mon],
    timeptr->tm_mday, timeptr->tm_hour,
    timeptr->tm_min,
    1900 + timeptr->tm_year);
  return result;
}

/** This function removes all the white space from a string */

char* trim(const char *input) {
    int i, j;
    char *output = (char*)malloc(strlen(input));
    for (i = 0, j = 0; i < strlen(input); i++,j++) {
        if (!isspace(input[i])) {
            output[j] = input[i];
        }
        else {
            j--;
        }
    }
    output[j] = '\0';
    return output;
}

/** This function converts a string input into all uppercase characters */

char* toUpperString(const char *input) {
    int i = 0;
    char *output = (char*)malloc(strlen(input));


    while(i < strlen(input)) {
        output[i] = toupper(input[i]);
        i++;
    }
    output[i-1] = '\0';
    return output;
}

bool check_hold(tinyxml2::XMLNode *Node) {
    bool multiFlag = false;
    tinyxml2::XMLNode *tempNode;
    const char* tempString;
    /** Select the first product from the order*/
    tempNode = Node->FirstChildElement("product01");

    /** This loop goes over each of the product numbers to
    make sure that there is not a product complication or
    sku that needs to be reviewed before being shipped out. */

    for(int i = 0; i < 5; i++) {
        tempString = tempNode->ToElement()->GetText();

        if(!tempString) {
            break;
        }

        /** This check is to see if someone buys a wall-mount with any other
        product. We place these orders on hold and review them to make sure that
        the customer is getting the right products together */

        if(startsWith(tempString, "10") || strstr(tempString, "04SW") ||
        strstr(tempString, "04SF") || strstr(tempString, "04AM")) {
            multiFlag = true;
        }

        if(multiFlag && i > 0) {
            setHold(Node);
            printf("Multiple Product Confliction.\n");
            return true;
        }

        /** This the skus in the loop can easily be modified to
        acomidate additional skus. There are definitions at the
        top of this document */


        for(int a = 0; a < NUM_PRODUCTS_TO_CHECK_FOR; a++) {
            const char* prodID = badProductID[a];
            if(tempString && strstr(toUpperString(tempString), prodID)) {
                setHold(Node);
                printf("Flagged Sku.\n");
                return true;
            }
        }

        /** The following two lines skip skip the quantity part of the xml file
        and go to the next product. */

        tempNode = tempNode->NextSibling();
        tempNode = tempNode->NextSibling();
    }

    /** After checking over each of the product, it's time to move onto other reasons we'd like to place
    all the orders on hold. If there is no "last name" in the order, that means that someone ordered more
    than 5 products, and we should just ignore searching for shipping. */

    if((Node->FirstChildElement("lastname"))) {
        bool ship_conflic = false;


        if((char*)Node->FirstChildElement("shipvia")->GetText()) {
            bool isInternational = strcmp(Node->FirstChildElement("scountry")->GetText(), "001") == 0 ? false : true;
            char* temp_data;

            temp_data = (char*)Node->FirstChildElement("saddress1")->GetText();
            char* address = (char*)malloc(strlen(temp_data));
            memcpy(address, temp_data, strlen(temp_data));
            address = toUpperString(trim((const char*) address));

            temp_data = (char*)Node->FirstChildElement("shipvia")->GetText();
            char* shipMethod = (char*)malloc(strlen(temp_data));
            memcpy(shipMethod, temp_data, strlen(temp_data));


            if((isInternational && strcmp(shipMethod, "PM") != 0) ||
            (strstr(address, "POBOX") && (strcmp(shipMethod, "1GD") == 0 ||
            strcmp(shipMethod, "FES") == 0 || strcmp(shipMethod, "FE2") == 0))) {
                ship_conflic = true;
            }
            free(address);
            free(shipMethod);
        }
        else {
            ship_conflic = true;
        }

        if(ship_conflic) {
            setHold(Node);
            printf("Shipping Confliction. \n");
            return true;
        }
    }
    return false;
}

void setHold(tinyxml2::XMLNode *Node){
    int year, month, day;
    char holdDate[20];
    char* workString;

    //get the order date element and get the text.
    tinyxml2::XMLNode *tempNode = Node->FirstChildElement("order_date");
    const char* tempString = tempNode->ToElement()->GetText();

    //transfer it to a non-const string
    workString = (char*)memcpy(holdDate, tempString, strlen(tempString));

    //get the necessary date
    year = atoi((const char*)strtok(workString, "- ")) + 1;
    month = atoi((const char*)strtok(NULL, "- "));
    day = 1;

    //transfer that date to another string.
    sprintf(holdDate, "%d-%d-%d\n", year, month, day);

    //Save it to the file.
    tempNode = Node->FirstChildElement("holddate");
    tempNode->ToElement()->SetText((const char*)holdDate);

}

void scanFile(tinyxml2::XMLDocument *myDoc) {
    tinyxml2::XMLNode *myNode = myDoc->FirstChild()->FirstChild();

    while(myNode!= NULL) {
        if(check_hold(myNode)) {
            printf("Hold placed on order %s.\n", myNode->FirstChildElement("odr_num")->GetText());
        }
        myNode = myNode->NextSibling();
    }

}

bool fexists(const char *filename)
{
  ifstream ifile(filename);
  return ifile;
}

int main() {
    time_t rawtime;
    struct tm* timeinfo;
    tinyxml2::XMLDocument myDoc;
    char filename[200];

    while(true) {
        Sleep(1000);
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        if((timeinfo->tm_min) % 20 == 0) {
            printf("Script Starting at %s\n\n", getTime(timeinfo));

            printf("Running Download....\n");
            system("bizsyncxlc.exe.lnk");

            if(fexists("orders.xml")) {
                myDoc.LoadFile("orders.xml");

                sprintf(filename, "BackupOrders\\%s.xml", myAsctime(timeinfo));
                printf("Backing up order....\n");
                myDoc.SaveFile(filename);

                printf("\nScanning for orders to put on hold....\n");
                scanFile(&myDoc);
                myDoc.SaveFile("Imports\\orders.xml", true);

                system("del orders.xml");
            }
            else
                printf("No new orders.");

            time(&rawtime);
            printf("\nScript finished successfully at %s\n", getTime(localtime(&rawtime)));
            printf("---------------------------------------\n");
            Sleep(60000);
        }
    }

    system("pause");
}
