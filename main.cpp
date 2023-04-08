#include <iostream>
#include <stdio.h>
#include <bitset>
#include <windows.h>
#include <string.h>
#include <string>
#include <vector>
#include <cmath>


using namespace std;
typedef unsigned int uint;
typedef unsigned long long ull;

#include "Helper.h"
class IFace {
public:
  virtual void status(){};
  virtual void load(){};     
  virtual void display(){};  
  virtual void properties(){};
  
  // for file and folder
  virtual int isFile() {return 0;};
  virtual int isFolder() {return 0;};
  virtual string getName() {return "";};
  virtual uint getSize() {return 0;};
  virtual IFace* getParent() {return NULL;};
  // ntfs
  virtual uint getID() {return 0;};
  virtual int getParentID(){ return 0;};
  virtual IFace* getComponent(uint ID) { return NULL;};

  // for volume
  virtual char getVolumeLetter() {return ' ';};
  virtual bool isFAT32Volume() {return 0;};
  virtual bool isNTFSVolume() {return 0;};
  // fat32
  virtual IFace** readFolderTree(int fcluster, IFace* parent){return NULL;};
  virtual void readFileContent(int fcluster, int size){};
  // ntfs
  virtual void readFolderTree(int parentID, vector<int> &chIndex) {};
  virtual void readFileContent(int ID, int start, int size) {};
  virtual void loadAllComponent(){};
  virtual void freeMemory(int exceptID) {};
  // for testing
  virtual void test() {};    	
};

IFace* v = NULL;  

#include "Component.h"   
#include "Volume.h"
 
 
// menu
class Menu {
private:
	int n = 4;
    const char* list[4] = {"Exit", "Load volume", "Volume property", "Root folder"};;
public: 
	Menu() {};
	void introduce() {  
		system("cls");
		cout << "-----Introduce-----\n";
    	cout << "He Dieu Hanh Thay Long!\n";
    	cout << "1. Nguyen Dinh CHien\n";
    	cout << "2. Nguyen Duy Anh\n";
    	cout << "3. Ngoc Duy\n";
    	cout << "4. Pham Phuoc Binh\n";
		cout << "5. Nguyen Minh Phat\n";
		cout << "Tham khao: https://github.com/DAMHONGDUC/FAT32_NTFS\n\n";
	}
	void run() {
    	int myoption;
   	 	while(1) { 
   	 		system("cls");
   	 		introduce(); 
   	 		status();
   	 		menu();
			myoption = getOption(0, n-1, 1, 0);
			switch(myoption) {
				case 1:
					load();
					break;
				case 2:
					properties();
					break;
				case 3:
					display();
					break;
				default:
					cout << "Good bye!\n";
					return;
			}
    	}
	}
	void properties() {
		if (v==NULL) {
			system("cls");
			cout << "Ban chua load volume nao!\n";
			system("pause");
		}
		else {
			v->properties();
		}
		
	}
	void display() {
		if (v==NULL) {
			system("cls");
			cout << "Ban chua load volume nao!\n";
			system("pause");
		}
		else {
			v->display();
		}
	}
	void menu() {
		cout << "\n-----Menu-----\n";
   		for (int i=0; i<n; i++) {
   	 		cout << i <<". "<< list[i] <<endl;	
		}
	}
	void status() {
		cout << "-----Status-----\n";
		if (v==NULL) {
			cout << "Ban chua load volume nao!\n";
		}
		else {
			v->status();
		}
	}

	void load() { 
		if (v!=NULL) {  
			system("cls");
			cout << "Ban da load volume "<< v->getVolumeLetter() << " roi, ban co thuc su muon load volume khac!\n";
			cout << "0. Quay lai\n";
			cout << "1. Load volume khac\n";
    		if (getOption(0,1,1,0)==0) return;
    		delete v;	
			v=NULL;
		}
		system("cls");
		HANDLE device = NULL;
		string disk_name;
	  	string letter;
	    do {
			cout << "Nhap volume letter (E, F): ";
			cin >> letter;
            disk_name = "\\\\.\\" + letter + ":";
			device = CreateFileA(disk_name.c_str(),             // Drive to open  // volume_letter
		                        GENERIC_READ,           // Access mode
		                        FILE_SHARE_READ|FILE_SHARE_WRITE,        // Share Mode
		                        NULL,                   // Security Descriptor
		                        OPEN_EXISTING,          // How to create
		                        0,                      // File attributes
		                        NULL);                  // Handle to template
		    	
		    if (device != INVALID_HANDLE_VALUE && device != NULL) {
		    	break;
			}
	    	if (GetLastError()==5) {
	    		cout << "Please run program with administrative permission\n";
			}
			else {
				cout << "Cannot load volume! error code #"<< GetLastError()<< endl;
				continue;
			}	
		} while (1);  
		
		int status = checkVolumeFormat(device);
		if (status == -1) {
			cout << "Sorry! we can not read this kind of volume format\n";
			system("pause");
		}
		else {
			if (status == 1) v = new FAT32(device, letter[0]);
			else if (status == 0) v = new NTFS(device, letter[0]);
			v->load();
		}
    }

    int checkVolumeFormat(HANDLE device) {
    	BYTE* buffer = read(device, 0, 512);
    	char format[15];
    	// fat32
		memcpy(format, copy(buffer, 0x52, 6), 6);
    	format[5]='\0';  
		if (strcmp(format, "FAT32")==0) return 1; 
		// ntfs
		memcpy(format, copy(buffer, 0x03, 4), 4);  
		format[4]='\0';
		if (strcmp(format, "NTFS")==0) return 0;  
    	delete[] buffer; 
		return -1; // bad                         
	}
	
};

Menu* menu = new Menu;  

int main() {
  menu->run();

  //v = new NTFS;
  //v->test();
  return 0;
}







