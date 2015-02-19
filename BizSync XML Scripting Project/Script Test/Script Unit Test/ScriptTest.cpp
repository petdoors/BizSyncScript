#include "tinyxml2.h"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
using namespace tinyxml2;

void getSkus(std::vector<std::string>* SkuStorage) {
    std::ifstream myFile("skus.csv");
    std::string line;
    std::string token;

    if(myFile.is_open()) {
        while(getline(myFile, line)) {
            std::istringstream iss(line);
            getline(iss, token, ',');
            SkuStorage->push_back(token);
        }
    }
}

void addElement(XMLDocument* doc, XMLElement* Root, char* name, const char* contents) {

    XMLElement* pElement = doc->NewElement(name);
    pElement->SetText(contents);
    Root->InsertEndChild(pElement);
}

int main() {
    std::vector<std::string> sku_storage;
    std::string shipMethods[] = {"1GD", "FE2", "FC", "PM", "WC"};
    std::string address[] = {"201 Tank Farm", "POBox 12345"};
    std::string country[] = {"001", "123"};
    int order_num = 0;

    XMLDocument doc;
    XMLNode *pRoot = doc.NewElement("VFPData");
    doc.InsertFirstChild(pRoot);

    getSkus(&sku_storage);
    char temp[50];

    for(int i = 0; i < 1; i++) {
        for(int SM = 0; SM < 5; SM++) {
            for(int pn = 0; pn < 5; pn++) {
                for(int a = 0; a < 2; a++) {
                    for(int c = 0; c < 2; c++) {
                        XMLElement* root2 = doc.NewElement("import_ca");
                        pRoot->InsertEndChild(root2);
                        addElement(&doc, root2, "lastname", "Smith");
                        addElement(&doc, root2, "firstname", "John");
                        addElement(&doc, root2, "address", "123 Test Drive");
                        addElement(&doc, root2, "city", "San Luis");
                        addElement(&doc, root2, "state", "CA");
                        addElement(&doc, root2, "zipcode", "93401");
                        addElement(&doc, root2, "phone", "1234567891");
                        addElement(&doc, root2, "cardtype", "V");
                        addElement(&doc, root2, "cardnum", "4111111111111111");
                        addElement(&doc, root2, "expires", "06/16");
                        addElement(&doc, root2, "shipvia", shipMethods[SM].c_str());
                        addElement(&doc, root2, "order_date", "2015-1-07 12:12:12");
                        sprintf(temp, "%i", order_num);
                        addElement(&doc, root2, "odr_num", (const char*)temp);

                        addElement(&doc, root2, "product01", "04SF");
                        sprintf(temp, "%.4f", 1.000);
                        addElement(&doc, root2, "quantity01", (const char*)temp);
                        if(pn > 0) {
                            addElement(&doc, root2, "product02", sku_storage.at(1268).c_str());
                            sprintf(temp, "%.4f", 1.000);
                            addElement(&doc, root2, "quantity02", (const char*)temp);
                        }
                        else {
                            addElement(&doc, root2, "product02", "");
                            addElement(&doc, root2, "quantity02", "");
                        }
                        if(pn > 1) {
                            addElement(&doc, root2, "product03", sku_storage.at(1268).c_str());
                            sprintf(temp, "%.4f", 1.000);
                            addElement(&doc, root2, "quantity03", (const char*)temp);
                        }
                        else {
                            addElement(&doc, root2, "product03", "");
                            addElement(&doc, root2, "quantity03", "");
                        }
                        if(pn > 2) {
                            addElement(&doc, root2, "product04", sku_storage.at(1268).c_str());
                            sprintf(temp, "%.4f", 1.000);
                            addElement(&doc, root2, "quantity04", (const char*)temp);
                        }
                        else {
                            addElement(&doc, root2, "product04", "");
                            addElement(&doc, root2, "quantity04", "");
                        }
                        if(pn > 3) {
                            addElement(&doc, root2, "product05", sku_storage.at(1268).c_str());
                            sprintf(temp, "%.4f", 1.000);
                            addElement(&doc, root2, "quantity05", (const char*)temp);
                        }
                        else {
                            addElement(&doc, root2, "product05", "");
                            addElement(&doc, root2, "quantity05", "");
                        }

                        addElement(&doc, root2, "slastname", "Smith");
                        addElement(&doc, root2, "sfirstname", "John");
                        addElement(&doc, root2, "saddress1", address[a].c_str());
                        addElement(&doc, root2, "scity", "San Luis");
                        addElement(&doc, root2, "sstate", "CA");
                        addElement(&doc, root2, "szipcode", "93401");
                        addElement(&doc, root2, "holddate", "");
                        addElement(&doc, root2, "country", country[c].c_str());
                        addElement(&doc, root2, "scountry", country[c].c_str());

                        order_num++;
                    }
                }
            }
        }
    }

    doc.SaveFile("orders.xml", true);
    system("exit");
}
