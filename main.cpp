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

#include "Helper.h"

class Component { 
protected:
  string name; 
  int fcluster;    
  string updatedHour; // created
  string updatedDate;  
  int size;  
  Component* parent;
public:
  Component() {};
  ~Component() {};
  Component(string name, int fcluster, Component* parent) {  // for root folder
  	this->name = name;
    this->fcluster = fcluster;
    this->parent = parent;
  }
  Component(string name, int fcluster, string updatedHour, string updatedDate, int size, Component* parent) { // for file or sub folder
    this->name = name;
    this->fcluster = fcluster;
    this->updatedHour = updatedHour;  
  	this->updatedDate = updatedDate; 
  	this->size = size;
  	this->parent = parent;
  }
  virtual int isFile() {return 0;};
  virtual int isFolder() {return 0;};
  virtual string getName() {return name;};
  virtual int getSize() {return size;};
  virtual Component* getParent() {return parent;}
  
  
  virtual void load() {};     
  virtual void display(){};  
  virtual void properties() {  
  	cout << name << endl;
  	cout << "First cluster: "<< fcluster<<endl;
  	cout << "Size         : "<< getSize() <<endl;
  	if (name=="root" && parent==NULL) return; // root folder don't have these following informations
	cout << "Updated hour : " << updatedHour<<endl;
  	cout << "Updated date : " << updatedDate<<endl;
  }
  // volume methods
  // loaded volume or not 
  virtual void status() {};
  // convert cluster index to sector index
  virtual int cToS(int k){return 0;};
  // look up fat table, return a list of sectors
  virtual int* lookUpFAT(int fcluster, int &n){return NULL;};
  // read folder tree, return a list of files and subfolders
  virtual Component** readFolderTree(int fcluster, Component* parent){return NULL;};
  virtual void readFileContent(int fcluster, int size){};
  virtual char getVolumeLetter() {return ' ';};

  // test area
  virtual void test() {};    
}; 

Component* v = NULL;  

#include "FileAndFolder.h"   

class Volume: public Component {
protected:
	HANDLE device = NULL;
	char volume_letter; 
	uint nS, sC, sV;
	 
public:
	Volume():Component() {};
 	Volume(HANDLE device, char letter):Component() {
 		this->device = device;
 		this->volume_letter = letter;
	}
	char getVolumeLetter() {
		return volume_letter;
	}
    virtual void readBootsector() {};
	
	~Volume(){CloseHandle(device);};


    // read FAT table contains the fcluster(th) cluster
    virtual BYTE* readFAT(int fcluster, uint &d, uint &r) {return NULL;};
};

class FAT32 :public Volume { 
private:
    uint sB, nF, sF;                  // sB=6646, nF=2, sF=773, sV=403456; 
    uint fSectorOfFAT, fSectorOfRDET; // fSectorOfFAT = 6646,  fSectorOfRDET = 8192;
    Component* root = NULL;  
public:
	FAT32(HANDLE device, char letter) : Volume(device, letter) {};
	void status() {
		cout << "Volume hien tai: "<<  volume_letter << " (FAT32)\n";
	}
	void display() {
		system("cls");

		root->display();
	} 
	
	void load() {
    	readBootsector();
    	this->root = new Folder("root", fcluster, NULL);  
		cout << "Load volume "<< volume_letter << " thanh cong!\n";
	    system("pause");
	}
	
	void readBootsector() {
	  	BYTE* bootsector = read(device, 0, 512);
	  	sV = convert(copy(bootsector, 0x20, 4), 4); // (tổng số sector của volume)
	  	sB = convert(copy(bootsector, 0xe, 2), 2);  // (sB)
	  	nS = convert(copy(bootsector, 0xb, 2), 2);  // (số bytes/sector), 
		sC = convert(copy(bootsector, 0xd, 1), 1);  // (số sector/cluster)
		nF = convert(copy(bootsector, 0x10, 1), 1); // (số bảng FAT)
		sF = convert(copy(bootsector, 0x24, 4), 4); // (số sectors/FAT)
		fcluster = convert(copy(bootsector, 0x2C, 4), 4);
		fSectorOfFAT = sB;                          // bootsector, FAT, FAT2, Data Area(RDET, ...)
		fSectorOfRDET = cToS(fcluster); 
	    delete[] bootsector; 
	}
	void properties() {
		system("cls");

		cout << "Dinh dang volume    : FAT32" << endl;
		cout << "Kich thuoc volume sV: "<< sV <<endl;
		cout << "Kich thuoc bootsector sB: "<< sB <<endl;
		cout << "So bytes/sector nS  : "<< nS <<endl;
		cout << "So sector/cluster sC: "<< sC <<endl;
		cout << "So bang FAT: "<< nF <<endl;
		cout << "Kich thuoc bang FAT sF: "<< sF <<endl;
	
		cout << "Sector bat dau cua FAT: "<< fSectorOfFAT<<endl;
		cout << "Sector bat dau cua RDET (Data Area): "<< fSectorOfRDET <<" (cluster bat dau "<< fcluster<<")\n";
	    system("pause");
	}

	int cToS(int k) {
    	return sB + nF*sF + (k-2)*sC;  
    }
	
	BYTE* readFAT(int fcluster, uint &d, uint &r) {	
		d = fcluster/(nS/4);        
		r = fcluster%(nS/4);        
		return read(device, (fSectorOfFAT+d)*nS, nS);  // FAT đánh số từ 0, 128 cluster, 256 KB 
	}
	int* lookUpFAT(int fcluster, int &n) {   
	    vector<uint> clusters;
	    uint t = fcluster; // the actual cluster
	    uint d, r;         // tương ứng bảng fat thứ d, cluster thứ r   
		BYTE* fat = readFAT(fcluster, d, r);
		while(d!=0 || r>=2) {      // d==0 && r<2 (không đọc 2 cluster đầu tiên)
			clusters.push_back(t); // oke
			t = convert(copy(fat, r*4, 4), 4); // bảng fat thứ d, cluster thứ r  
			if (t >= 0x0FFFFFF8) break;        // 0x0FFFFFFF, 0x0FFFFFF8 // end
			if (t/(nS/4)!=d) {                 // có cần đọc lại bảng fat (!= d hiện tại)
				delete[] fat;
				fat = readFAT(t, d, r);        // đọc lại bảng fat, d, r được cập nhật (ứng với đoạn fat mới)
			}
			else {
				r = t%(nS/4);
			}
		};
		n = clusters.size()*sC; // sC=4 
		int* s = new int [n]; 
		for (int i=0; i<clusters.size(); i++) {
			t = cToS(clusters[i]);
			for (int j=0; j<sC; j++) {
				s[i*sC + j] = t+j; 
			}
		}
		delete[] fat;
		return s;	
	}
	
	Component** readFolderTree(int fcluster, Component* parent) {  
    	int n; 
  	 	int* s = v->lookUpFAT(fcluster, n);      // for RDET or SDET 
  	
		Component** list = new Component* [52];  // tối đa 50 thư mục con 
		for (int i=0; i<52; i++) list[i] = NULL; 
		int index = 2;                       
		
		string name, ext;
		BYTE* det;
		int t;
		// nS = 512, sC=4;
		int nEntries = nS/32; // nS/32 = 16 entry (so entries tren mot bang det)
    	for (int k=0; k<n; k++) {
    		// nS = 512, sC=4;
    		det = read(device, s[k]*nS, nS); // mot lan doc nS*k (k>=1)
    		int i = 2;             	 // bo qua 64 by dau   
    		// doc RDET thi i=0; con lai i=2 
    		if (parent->getName()=="root" && parent->getParent()==NULL) i=0; 
			for (; i<nEntries; i++) { 
				t = i*32;                                   // bat dau entry    
				if (det[t]==0xE5) continue;    // bỏ qua entry đã bị xóa         // || det[t]==0               
				if (det[t+0xb]==0x10 || det[t+0xb]==0x20) { // file or sub folder  
					// xử lý entry chinh // lay ten // 11 byte đầu
					name=ext="";
					for (int k=0; k<8; k++) {
					    name += det[t+k]; 
					}
					for (int k=0; k<3; k++) {
						ext += det[t+8+k];  
					}
					name += "." + ext;
					//cout << "full name: "<< name<<endl;
					int j = t-32;                // entry truoc do, cot 0
					int jb = t-32 + 0xb;         // entry truoc do, cot b  j, jb trên một hàng
					if (j>=0 && det[j]!=0xE5 && jb>=0 && det[jb]==0xf) { // có it nhat mot entry phu
					    name = "";
					    do {
					    	for (int k=0; k<5; k++) name += det[j+1+2*k];   // 1
					    	for (int k=0; k<6; k++) name += det[j+14+2*k];  // e
					    	for (int k=0; k<3; k++) name += det[j+28+2*k];  // 1c 
					    	jb -= 32; j -= 32;
						} while (j>=0 && det[j]!=0xE5 && jb>0 && det[jb]==0xf); //  && det[j]!=0
					} 
					//16h-2 giờ cập nhật
					string updatedHour = toHour(copy(det, t+0x16, 2));
					//18h-2 ngày cập nhật
					string updatedDate = toDate(copy(det, t+0x18, 2));
					//1Ah-2 cluster bắt đầu
					uint fclus =  convert(copy(det, t+0x1A, 2), 2); 
					//1Ch-4 kích thước tập tin
					uint size =  convert(copy(det, t+0x1C, 4), 4);
					if (det[t+0xb]==0x10) { // sub folder
						list[index] = new Folder(name, fclus, updatedHour, updatedDate, size, parent);
					}
					else {
						bool isTextFile = false;
						if (ext == "TXT") {
							isTextFile = true;
						}
						list[index] = new File(name, fclus, updatedHour, updatedDate, size, parent, isTextFile);
					}
					index++;			
				} 			
			}
			delete[] det;
		}
		return list;
	}
	
	void readFileContent(int fcluster, int size) {
  	    int n;
  	 	int* s = v->lookUpFAT(fcluster, n);
		for (int i=0; i<n-1; i++) {
			pChar(read(device, s[i]*nS, nS),nS, nS);
		}
		int t = size%nS;
		if (t==0) pChar(read(device, s[n-1]*nS, nS),nS, nS);
		else pChar(read(device, s[n-1]*nS, nS),t, t);
	}
	
	void test() {

 		device = CreateFile("\\\\.\\E:",             // Drive to open  // volume_letter
		                        GENERIC_READ,           // Access mode
		                        FILE_SHARE_READ|FILE_SHARE_WRITE,        // Share Mode
		                        NULL,                   // Security Descriptor
		                        OPEN_EXISTING,          // How to create
		                        0,                      // File attributes
		                        NULL);                  // Handle to template
		    	
	    if (device != INVALID_HANDLE_VALUE && device != NULL) {
	    	cout << "ready to read!\n";
		}
        else if (GetLastError()==5) {
    		cout << "Please run program with administrative permission\n";
			return;
		}
		else {
			cout << "Cannot load volume! error code #"<< GetLastError()<< endl;
				return;
		}	
		// ...
	
		system("pause");
	}

};
class NTFS: public Volume {
private:
	uint fSectorOfMFT;  
	uint nEntry;     // 1024 bytes
public:
	NTFS():Volume() {
		
	}
	NTFS(HANDLE device, char letter) : Volume(device, letter) {};
	~NTFS() {};
	void status() {
		cout << "Volume hien tai: "<<  volume_letter << " (NTFS)\n";
	}
    void load() {
    	readBootsector();
    	properties();
    	cout << "Load volume "<< volume_letter << " thanh cong!\n";
	    system("pause");
	}
	void readBootsector() {
		BYTE* bootsector = read(device, 0, 512);
		nS = convert(copy(bootsector, 0x0B, 2), 2);  
		sC = convert(copy(bootsector, 0x0D, 1), 1);  
		sV = convert(copy(bootsector, 0x28, 8), 8); // Total Sectors
		uint fclusterMFT = convert(copy(bootsector, 0x30, 8), 8);  
		fSectorOfMFT = fclusterMFT * sC; 
		uint tmp = convert(copy(bootsector, 0x40, 8), 8); 
		nEntry = pow(2,stoi(bitwise_complement(toBinary(tmp - 1)),0,2)); // transform binary string to bitwise_complement, then convert to uint, 2^
		delete[] bootsector;
	}
	void properties() {
		system("cls");
		cout << "Dinh dang volume    : NTFS" << endl;
		cout << "Kich thuoc volume sV: "<< sV <<endl;
		cout << "So bytes/sector nS  : "<< nS <<endl;
		cout << "So sector/cluster sC: "<< sC <<endl;
		cout << "Sector bat dau cua MFT: "<< fSectorOfMFT<<endl;
		cout << "Kich thuoc MFT entry: "<< nEntry<< endl;
	    system("pause");
	}
	void test() {

		device = CreateFile("\\\\.\\C:",             // Drive to open  // volume_letter
		                        GENERIC_READ,           // Access mode
		                        FILE_SHARE_READ|FILE_SHARE_WRITE,        // Share Mode
		                        NULL,                   // Security Descriptor
		                        OPEN_EXISTING,          // How to create
		                        0,                      // File attributes
		                        NULL);                  // Handle to template
		    	
	    if (device != INVALID_HANDLE_VALUE && device != NULL) {
	    	cout << "ready to read!\n";
		}
        else if (GetLastError()==5) {
    		cout << "Please run program with administrative permission\n";
    		return;
		}
		else {
			cout << "Cannot load volume! error code #"<< GetLastError()<< endl;
			return;
		}	
		//
		 
		load();
	}
	
};
 
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
					load(); // and create volume v
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
	  	char my_volume[7] = "\\\\.\\";
	  	char letter;
	    do {
			cout << "Nhap volume letter (E, C, D): ";
			cin >>  letter;
			my_volume[4] = letter;
			my_volume[5] = ':';
			my_volume[6] = '\0';

			device = CreateFile(my_volume,             // Drive to open  // volume_letter
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
			}	
		} while (1);  
		
		int check = checkVolumeFormat(device);
		if (check == -1) {
			cout << "Sorry! we can not read this kind of volume format\n"; // v==NULL 
			system("pause");
		}
		else {  // v != NULL, created, has device and letter
			if (check == 1) v = new FAT32(device, letter);
			else if (check == 0) v = new NTFS(device, letter);
			v->load(); //
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
    	// bad
		return -1;                                
	}
	
};

Menu* menu = new Menu;  

int main() {
  menu->run();
  return 0;
}










