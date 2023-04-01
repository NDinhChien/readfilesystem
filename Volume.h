#pragma once
#ifndef VOLUME_H
#define VOLUME_H
class Volume: public IFace {
protected:
	HANDLE device = NULL;
	char volume_letter; 
	uint nS, sC, sV;
	IFace* root = NULL;
public:
	Volume(){};
 	Volume(HANDLE device, char letter){
 		this->device = device;
 		this->volume_letter = letter;
	}
	~Volume(){
		CloseHandle(device);
		delete root;	
	};
	char getVolumeLetter() {
		return volume_letter;
	}
	void display() {
		system("cls");
		root->display();  
	} 
    virtual void readBootsector() {};
};

class FAT32 :public Volume { 
private:
	uint fcluster;
    uint sB, nF, sF;                   
    uint fSectorOfFAT, fSectorOfRDET;  
public:
	FAT32(HANDLE device, char letter) : Volume(device, letter) {};
	void status() {
		cout << "Volume hien tai: "<<  volume_letter << " (FAT32)\n";
	}
	bool isFAT32Volume() {return 1;}
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

	uint cToS(uint k) {
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
		n = clusters.size()*sC;  
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
	
	IFace** readFolderTree(int fcluster, IFace* parent) {  
    	int n; 
  	 	int* s = lookUpFAT(fcluster, n); // for RDET or SDET 
		IFace** list = new IFace* [52];     // tối đa 50 thư mục con 
		for (int i=0; i<52; i++) list[i] = NULL; 
		int index = 2;                       
		
		string name, ext;
		BYTE* det;
		int t;
		// nS = 512, sC=4;
		int nEntries = nS/32; //  16 entry (so entriest / mot bang det)
    	for (int k=0; k<n; k++) {
    		// nS = 512, sC=4;
    		det = read(device, s[k]*nS, nS); 
    		int i = 2;        // bo qua 64 byte dau   
    		//  RDET thi i=0; con lai i=2 
    		if (parent->getName()=="root" && parent->getParent()==NULL) i=0; 
			for (; i<nEntries; i++) { 
				t = i*32;                      // bat dau entry    
				if (det[t]==0xE5) continue;    // bỏ qua entry đã bị xóa            
				if (det[t+0xb]==0x10 || det[t+0xb]==0x20) { // file or sub folder  
					// xu ly entry chinh 
					name=ext="";
					for (int k=0; k<8; k++) {
					    name += det[t+k]; 
					}
					for (int k=0; k<3; k++) {
						ext += det[t+8+k];  
					}
					name += "." + ext;
					
					int j = t-32;                // entry truoc do, cot 0
					int jb = t-32 + 0xb;         // entry truoc do, cot b
					if (j>=0 && det[j]!=0xE5 && jb>=0 && det[jb]==0xf) {   // có it nhat mot entry phu
					    name = "";
					    do {
					    	for (int k=0; k<5; k++) name += det[j+1+2*k];   
					    	for (int k=0; k<6; k++) name += det[j+14+2*k];  
					    	for (int k=0; k<3; k++) name += det[j+28+2*k]; 
					    	jb -= 32; j -= 32;
						} while (j>=0 && det[j]!=0xE5 && jb>0 && det[jb]==0xf);
					} 
					//16h-2 giờ cập nhật
					string createdHour = toHour(copy(det, t+0x16, 2));
					//18h-2 ngày cập nhật
					string createdDate = toDate(copy(det, t+0x18, 2));
					//1Ah-2 cluster bắt đầu
					uint fclus =  convert(copy(det, t+0x1A, 2), 2); 
					//1Ch-4 kích thước tập tin
					uint size =  convert(copy(det, t+0x1C, 4), 4);
					if (det[t+0xb]==0x10) { // sub folder
						list[index] = new Folder(name, fclus, createdHour, createdDate, size, parent);
					}
					else {
						bool isTextFile = false;
						if (ext == "TXT") {
							isTextFile = true;
						}
						list[index] = new File(name, fclus, createdHour, createdDate, size, parent, isTextFile);
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
  	 	int* s = lookUpFAT(fcluster, n);
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
		//....
		system("pause");
	}
};

class NTFS: public Volume {
private:
	uint fSectorOfMFT;  
	uint nE;                   // 1024 bytes
	uint numberOfEntry;
	vector<IFace*> listOfAll; // all files and folder

public:
	NTFS():Volume() {}
	NTFS(HANDLE device, char letter) : Volume(device, letter) {};
	~NTFS() {
		freeMemory();
	}
	IFace* getComponent(uint index) {
		if (index >=0 && index < listOfAll.size()) return listOfAll[index];
		return NULL; 
	}
	bool isNTFSVolume() {return 1;}
	
	void status() {
		cout << "Volume hien tai: "<<  volume_letter << " (NTFS)\n";
	}
    void load() {
    	readBootsector();
    	readMFTEntry();
    	loadAllComponent(); 
    	root = new Folder("root", 0, 5, -1);  
    	root->load();
    	cout << "Load volume "<< volume_letter << " thanh cong!\n";
	    system("pause");
	}
	void readBootsector() {
		BYTE* bootsector = read(device, 0, 512);
		nS = convert(copy(bootsector, 0x0B, 2), 2);  
		sC = convert(copy(bootsector, 0x0D, 1), 1);  
		sV = convert(copy(bootsector, 0x28, 8), 8);  
		uint fclusterMFT = convert(copy(bootsector, 0x30, 8), 8);  
		fSectorOfMFT = fclusterMFT * sC; 
		uint tmp = convert(copy(bootsector, 0x40, 8), 8); 
		nE = pow(2,stoi(bitwise_complement(toBinary(tmp - 1)),0,2));  
		delete[] bootsector;
	}
	void properties() {
		system("cls");
		cout << "Dinh dang volume    : NTFS" << endl;
		cout << "Kich thuoc volume sV: "<< sV <<endl;
		cout << "So bytes/sector nS  : "<< nS <<endl;
		cout << "So sector/cluster sC: "<< sC <<endl;
		cout << "Sector bat dau cua MFT: "<< fSectorOfMFT<<endl;
		cout << "Kich thuoc MFT entry: "<< nE<< endl;
		cout << "So luong entry      : "<< numberOfEntry << endl;
	    system("pause");
	}
    void readMFTEntry() {
    	BYTE* mft = read(device, fSectorOfMFT*nS, nE);
		uint oSI = convert(copy(mft, 0x014, 2), 2);
    	uint lSI = convert(copy(mft, 0x048, 4), 4);
		uint lFN = convert(copy(mft, 0x09C, 4), 4);
	
		uint lDT, oDT = 0;
		uint tmp = convert(copy(mft, 0x108, 4), 4);
		if (tmp == 64) {
			oDT = oSI + lSI + lFN + convert(copy(mft, 0x10c, 4), 4);
			lDT = convert(copy(mft, 0x134, 4), 4);
		}
		else {
			oDT = oSI + lSI + lFN;
			lDT = convert(copy(mft, 0x10C, 4), 4);
		}
		tmp = sC/(nE/nS); //  so entry/cluster
		numberOfEntry = (convert(copy(mft, oDT + 24, 8), 8) + 1) * tmp; 
		delete[] mft;
	}
	void loadAllComponent() {         
		freeMemory();
		BYTE* entry; 
		uint ID;
		int parentID;
		uint jmp, size;
		uint lSI, lFN, lOI, lDT;
		string name, ext;
		bool isTextFile, isResident;
		int status, type;
		IFace* tmp;
		for (uint id=39; id<numberOfEntry; id++) {   
			entry = read(device, idToS(id)*nS, nE);
			if (convert(copy(entry, 0x00, 4), 4) != 1162627398) {
		 		delete[] entry;
				continue;
			}
			ID = convert(copy(entry, 0x02C, 4), 4); // == id
			if (ID > 38) {
				
				isTextFile = false;
				isResident = false;
				
				jmp = convert(copy(entry, 0x014, 2), 2); // 56 ?
				readSIAttr(entry, jmp, status, lSI);
				if (status != 0 && status != 32) {
					continue;
				}
				jmp = 56 + lSI;
				readFNAttr(entry, jmp, parentID, name, ext, isTextFile, lFN);
				
				jmp += lFN;
				if (convert(copy(entry, jmp, 4), 4)==64) {
					lOI = convert(copy(entry, jmp +4, 4), 4);
					jmp += lOI;
					readDTAttr(entry, jmp, size, type, lDT);
				}
				else if (status==32) {
					while(jmp+3< nE && convert(copy(entry, jmp, 4), 4) != 128) jmp+=4;
					if (jmp+3<nE) readDTAttr(entry, jmp, size, type, lDT);
					else {
						cout << "bat on!\n";
						size = 0;
						system("pause");
					}
				}
				
				if (type==0) isResident = true;
				delete[] entry;
				tmp = NULL;
				if (status == 0) tmp = new Folder(name, 0, ID, parentID);
				else if (status == 32) tmp =  new File(name, size, ID, parentID, isResident, jmp, isTextFile);
				if (tmp!=NULL) listOfAll.push_back(tmp);
			}
		}
	}
	uint idToS(uint id) { // id To Sector
		return fSectorOfMFT + id*2;
	}
	void readSIAttr(BYTE entry[], uint jmp, int &status, uint &lSI) {
		status = convert(copy(entry, jmp+56, 4), 4);
		lSI = convert(copy(entry, jmp+4, 4), 4);
	}
	void readFNAttr(BYTE entry[], uint jmp, int &parentID, string &name, string &ext, bool &isTextFile, uint &lFN) {
		lFN = convert(copy(entry, jmp + 4, 4), 4);
		parentID = convert(copy(entry, jmp+24, 6), 6);
		int lName = convert(copy(entry, jmp + 88, 1), 1);
		name = "";
		for (int i=0; i<lName; i++) {
			name += entry[jmp+90+i*2];
		}
		ext = "";
		int l=name.length()-1;
		while (name[l]!='.' && l>=0) ext = name[l--] + ext;
		if (l<0) {
			ext = "";
		}
		else {
			if (ext=="txt") isTextFile = true;
		}
	}
	void readDTAttr(BYTE* entry, uint jmp, uint &size, int &type, uint &lDT) {
		lDT = convert(copy(entry, jmp + 4, 4), 4);
		size = convert(copy(entry, jmp + 16, 4), 4);
		type = convert(copy(entry, jmp + 8, 1), 1);
	}
	void readFolderTree(int parentID, vector<int> &chIndex) {
		int l = listOfAll.size();
		for (int i=0; i<l; i++) {
			if (listOfAll[i]->getParentID()==parentID) chIndex.push_back(i); 
		}
	}
	void readFileContent(int ID, int start, int size) {
		BYTE* entry = read(device, idToS(ID)*nS, nE); 
		uint jmp = convert(copy(entry, start+20, 2), 2);
		pChar(copy(entry, start+jmp, size), size, size);
		delete[] entry;
	}
	void freeMemory() {
		int l = listOfAll.size();
		for (int i=0; i<l; i++) {
			delete listOfAll[i];
		}
		listOfAll.clear();
	}
	void freeMemory(int exceptID) {
		int l = listOfAll.size();
		for (int i=0; i<l; i++) {
			if (listOfAll[i]->getID()!=exceptID) delete listOfAll[i];
		}
		listOfAll.clear();
	}
	void test() {
		device = CreateFile("\\\\.\\F:",             // Drive to open  // volume_letter
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
    		cout << "Please run program with administrative permission!\n";
    		return;
		}
		else {
			cout << "Cannot load volume! error code #"<< GetLastError()<< endl;
			return;
		}	
		load();
		system("pause");
	}
};

#endif