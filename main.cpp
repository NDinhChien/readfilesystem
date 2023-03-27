#include <iostream>
#include <stdio.h>
#include <bitset>
#include <windows.h>
#include <string.h>
#include <string>
#include <vector>


using namespace std;
typedef unsigned int uint;

// một số hàm hỗ trợ
BYTE* copy(BYTE* src, int k, int n) {
  BYTE* dest = new BYTE[n];
  for(int i=0; i<n; i++) {
    dest[i] = src[k+i];
  }
  return dest;
} 
uint e256[4] = {1, 256, 256*256, 256*256*256};

// litte edian, normal indexing
uint convert(BYTE number[], int n) {  // 4 BYTE trở xuống
  uint t = 0;
  for (int i=0; i<n; i++) {
  	t += number[i] * e256[i];
  }
  return t;
}

string toDate(BYTE date[]) { 
	bitset<7> y;  // 6...0
	bitset<4> m;  // 3...0
	bitset<5> d;  // 4...0
	bitset<8> r(date[0]); // date: 0...7      8...15
	bitset<8> l(date[1]); //    l: 7...0   r: 7...0 
	
	for (int i=6; i>=0; i--) y[i] = l[i+1]; // 7...1
	m[3] = l[0];
	for (int i=2; i>=0; i--) m[i] = r[i+5]; // 7...5
	for (int i=4; i>=0; i--) d[i] = r[i]; // 4...0
	string str(to_string(y.to_ulong()+1980) + "-"+ to_string(m.to_ulong())+ "-"+ to_string(d.to_ulong()));
	return str;

}
string toHour(BYTE hour[]) {
    bitset<5> h;  // 4...0
	bitset<6> m;  // 5...0
	bitset<5> s;  // 4...0
	bitset<8> r(hour[0]); // date: 0...7      8...15
	bitset<8> l(hour[1]); //    l: 7...0   r: 7...0 
	
	for (int i=4; i>=0; i--) h[i] = l[i+3]; // 7...3
	m[5] = l[2]; m[4]= l[1]; m[3]= l[0];
	for (int i=2; i>=0; i--) m[i] = r[i+5]; // 7...5
	for (int i=4; i>=0; i--) s[i] = r[i]; // 4...0
	string str(to_string(h.to_ulong()) + ":"+ to_string(m.to_ulong())+ ":"+ to_string(s.to_ulong()));
	return str;
} 
void pChar(BYTE* str, int n, int ncol) {
	int d = n/ncol;
	int r = n%ncol;
	for (int i=0; i<d; i++) {
		for (int j=0; j<ncol; j++) {
			cout << str[i*ncol+j];
		}
		cout << endl;
	}
	for (int j=0; j<r; j++) {
		cout << str[d*ncol+j];
	}
	if (r>0) cout << endl;	
}
void pHexa(BYTE* str, int n, int ncol) {
	int t;
	int d = n/ncol;
	int r = n%ncol;
	for (int i=0; i<d; i++) {
		for (int j=0; j<ncol; j++) {
			t = str[i*ncol+j];
			if (t<16) cout << "0";
			cout << hex << t << " ";
		}
		cout << endl;
	}
	for (int j=0; j<r; j++) {
		t = str[d*ncol+j];
		if (t<16) cout << "0";
		cout << hex << t << " ";
	}
	if (r>0) cout << endl;	
}
bool isLetterChar(char chr) {
	if (('a'<=chr && chr <='z') || ('A'<=chr && chr <='Z')) return 1;
	return 0;
}
bool isNumberChar(char chr) {
	if ('0'<=chr && chr <='9') return 1;
	return 0;
}
int getOption(int start, int end, int step, bool isletterAccepted) { // isletterAccepted 1, them mot so chu cai
	string rs;
	int tmp, l;
	bool isNumber;
	int plus=isletterAccepted*2;  
	// isletterAccepted=0, plus=0 // normal 
	// isletterAccepted=1, plus=2; 
	// b/B, r/R 0, 1, 2, 3
	// 0,    1, 2  3, 4, 5
	while(1) {  
		cout <<">> ";
		cin >> rs;
		l = rs.length();
		if (isletterAccepted==1 && l==1 && isLetterChar(rs[0])) { // nhap 1 chu cai
			if (rs[0]=='r' || rs[0]=='R') return 1;
			if (rs[0]=='b' || rs[0]=='B') return 0;
			continue;
		}
		// so 1,2,... chu so
		isNumber=true;
		for (int i=0; i<l; i++) {
			if (!isNumberChar(rs[i])) {
				isNumber=false;
				break;
			}
		}
		if (isNumber) {
			tmp = stoi(rs); // 0-9
			if (tmp>=start && tmp<=end && (tmp-start)%step==0) return tmp+plus;
		} 
	}
} 

class Component { 
protected:
  string name; 
  int fcluster;    
  string updatedHour; 
  string updatedDate;  
  int size;  
public:
  Component() {};
  ~Component() {};
  Component(string name, int fcluster) {  // for root folder
  	this->name = name;
    this->fcluster = fcluster;
  }
  Component(string name, int fcluster, string updatedHour, string updatedDate, int size) { // for file or sub folder
    this->name = name;
    this->fcluster = fcluster;
    this->updatedHour = updatedHour;  
  	this->updatedDate = updatedDate; 
  	this->size = size;
  }

  // virtual so it can allow overriding
  virtual int isFile() {return 0;};
  virtual int isFolder() {return 0;};
  virtual string getName() {return name;};
  virtual int getSize() {return size;};
  virtual Component* getParent() {return NULL;}
  virtual void load() {};    // updated content or list
  virtual void display(){};  // properties + content or list
  
  virtual void properties() {  
  	cout << name << endl;
  	cout << "First cluster: "<< fcluster<<endl;
  	cout << "Size         : "<< getSize() <<endl;
  	if (name=="root") return;
	cout << "Updated hour : " << updatedHour<<endl;
  	cout << "Updated date : " << updatedDate<<endl;
  }
  
  // can be able to call methods of volume v
  virtual int cToS(int k){return 0;};
  virtual BYTE* read(int vt, int k){return NULL;};
  virtual int* lookUpFAT(int fcluster, int &n){return NULL;};
  virtual Component** readFolderTree(int fcluster, Component* parent){return NULL;};
  virtual void readFileContent(int fcluster, int size){};
  virtual void status() {};
  // for testing
  virtual void test() {};    
}; 

Component* v = NULL;  

class File :public Component{
private:
  bool isTextFile;
public:
  File():Component() {};
  ~File() {};
  File(string name, int fcluster, string updatedHour, string updatedDate, int size, bool isTextFile) 
  	: Component(name, fcluster, updatedHour, updatedDate, size) {
  		this->isTextFile = isTextFile;
  };
  void display() {  
  	system("cls");
  	properties();                          // of this file, only update when refresh its parent (folder chứa nó)
  	cout << "\n-----Content-----\n";       // content
  	
	if (isTextFile) {
		if (size==0) {
			cout << "<empty>\n";
		}
		else v->readFileContent(fcluster, size); // reload content 
	}
	else {
		cout << "Xin loi, hien tai he thong chi co the doc noi dung file TXT !";
	}
	cout << endl;
    system("pause");
    system("cls");
  }
  void properties() {  
    cout <<"----Properties----\n";
    cout << "File name    : ";
    Component::properties();
  }
  //void load() {};
  int isFile() {return 1;};
};


class Folder :public Component{
private:
  Component** list=NULL;  
  Component* parent;    // name=="root" &* parent==NULL // 
public:
  Folder():Component() {};
  Folder(string name, int fcluster,  Component* parent) 
    :Component(name, fcluster) { // for root folder, fcluster = 2
    this->parent = parent;       // NULL;
  }
  
  Folder(string name, int fcluster, string updatedHour, string updatedDate, int size, Component* parent) 
    :Component(name, fcluster, updatedHour, updatedDate, size) {   // for sub folder
  	this->parent = parent;
  }
  int getSize() {
    if (list==NULL) return 0; 
    int total;
  	for (int i=2; i<52; i++) { //
  		if (list[i]!=NULL) {
  			total += list[i]->getSize();
		}
  		else break;
	}
	return total;
  }
  Component* getParent() {
  	return parent;
  }
  void display() {                   // properties and list
  	load();                          // reload list // all files and subfolder properties are updated// the first thing
    int myoption;
    while(1) { 
      system("cls");
      properties();
	  cout << "\n-----Content-----\n";                       
      cout << "  B. Back\n";
      cout << "  R. Refresh\n\n";
	  cout << name<< endl; 
      int i=2;
	  for (; i<52; i++) {
	  	if (list[i]!=NULL) {
	  		cout <<"  "<< i-2<<". "<< list[i]->getName()<<endl;
		}
		else break;
	  }
	  if (i==2) cout << "<empty>\n";
	  
	  myoption = getOption(0, i-1, 1, true); // r/R->0, b/B->1, 0->i-1 (+2) // tuong ung
      if (myoption==0) return;
	  if (myoption==1) load(); // list duoc cap nhat
	  else list[myoption]->display(); 
	}
  }
  
  // SDET, bo 3 entry cua '.' va '..' (wrong entry) 
  // root, parent, this, chi co 1 version, duoc cap nhat chu, khi refresh parent chua no, nhung root khong duoc cap nhat (khong can thiet, fcluster=2 always)	
  void load() {  
  	freeMemory();  
  	list = v->readFolderTree(fcluster, this);   
  	list[0] = parent; 
 	list[1] = this; 
	
  }
  void countChildren() {
  	int nfi=0,nfo=0;
  	for (int i=2; i<52; i++) {
  	 	if (list[i]!=NULL) {
  	 		nfi += list[i]->isFile();
  	 		nfo += list[i]->isFolder();
		}
		else break;
	}
	cout << "Number of files     : " << nfi << endl;
  	cout << "Number of subfolders: " << nfo << endl; 
  }
  void properties() {
  	cout <<"----Properties----\n";
  	cout << "Folder name  : ";
	Component::properties(); 
	countChildren();
  }
  int isFolder() {return 1;};

  void freeMemory() {
	// not for the first time
	if (list==NULL) return;
	// don't delete parent (root in danger), don't delete this (xoa chinh doi tuong goi load())
  	for (int i=2; i<52; i++) { 
    	if (list[i]!=NULL) delete list[i];
    	else break;
	}
	delete[] list; // 
	list = NULL;   // lan dau và sau khi free Memory (khong free tiep)
  }
  ~Folder() {
  	delete parent;
  	freeMemory();
  }
};

class Volume :public Component { 
private:
	char volume_letter;  
  	char volume_format[7]; 
    HANDLE device = NULL;
    int nS;                          // nS=512;
    int sC, sB, nF, sF, sV;          //sC=4, sB=6646, nF=2, sF=773, sV=403456; 
    int fSectorOfFAT, fSectorOfRDET; //fSectorOfFAT = 6646,  fSectorOfRDET = 8192;
    Component* root = NULL;  
public:
	Volume():Component() {};
	void status() {
		cout << "-----Status-----\n";
		if (device==NULL) {
			cout << "Ban chua load volume nao!\n";
		}
		else cout << "Volume hien tai: "<<  volume_letter << " ("<< volume_format<<")\n";
	};

	void display() {
		system("cls");
		if (device==NULL) {
			cout << "Ban chua load volume nao!\n";
			system("pause");
			return;
		}
		root->display();
	}
    
    void load() {
    	system("cls");
    	if (device!=NULL) {
    		cout << "Ban da load volume "<< volume_letter << " roi, ban co thuc su muon load volume khac!\n";
			cout << "0. Quay lai\n";
			cout << "1. Load volume khac\n";
    		if (getOption(0,1,1,0)==0) return;
		}
    	loadVolume();
    	readBootsector();
		this->root = new Folder("root", fcluster, NULL);  
		cout << "Load volume "<< volume_letter << " thanh cong!\n";
	    system("pause");
	}
	void loadVolume() { 
		system("cls");
	  	char my_volume[10] = "\\\\.\\  ";
	  	char letter;
	    do {
			cout << "Nhap volume letter (vd E): ";
			cin >>  letter;
			
			my_volume[4] = letter;
			my_volume[5] = ':';
			my_volume[6] = '\0';
			
			//cout << my_volume<<endl;
			device = CreateFile(my_volume,             // Drive to open  // volume_letter
		                        GENERIC_READ,           // Access mode
		                        FILE_SHARE_READ|FILE_SHARE_WRITE,        // Share Mode
		                        NULL,                   // Security Descriptor
		                        OPEN_EXISTING,          // How to create
		                        0,                      // File attributes
		                        NULL);                  // Handle to template
		    	
	    	if (device == INVALID_HANDLE_VALUE) {
	    		cout << "That bai! Volume "<< letter <<" khong ton tai hoac co dinh dang khac FAT32\n";
			}
			else {
				
				break;
			}
		} while (1);  
		this->volume_letter = letter;
    }
    
	void readBootsector() {
	  	BYTE* bootsector = read(0, 512);
	  	sV = convert(copy(bootsector, 0x20, 4), 4); // (tổng số sector của volume)
	  	sB = convert(copy(bootsector, 0xe, 2), 2);  // (sB)
	  	nS = convert(copy(bootsector, 0xb, 2), 2);  // (số bytes/sector), 
		sC = convert(copy(bootsector, 0xd, 1), 1);  // (số sector/cluster)
		nF = convert(copy(bootsector, 0x10, 1), 1); // (số bảng FAT)
		sF = convert(copy(bootsector, 0x24, 4), 4); // (số sectors/FAT)
	
		strcpy(volume_format, (char*)copy(bootsector, 0x52, 6));  //  chuỗi 'FAT32';
		
		fcluster = convert(copy(bootsector, 0x2C, 4), 4);
		fSectorOfFAT = sB;                          // bootsector, FAT, FAT2, Data Area(RDET, ...)
		fSectorOfRDET = cToS(fcluster); 
	    delete[] bootsector; 
	}
	    

	void properties() {
		system("cls");
		if (device==NULL) {
			cout << "Ban chua load volume nao!\n";
			system("pause");
			return;
		}
		cout << "Dinh dang volume    : "<< volume_format<<endl;
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
	
	~Volume(){CloseHandle(device);};
	
	// reader
	int cToS(int k) {
    	return sB + nF*sF + (k-2)*sC;  
    }
    // đọc k bytes từ vị trí vt
	BYTE* read(int vt, int k) {      
  		DWORD bytesRead;
  		BYTE* buffer = new BYTE[k]; 
  		SetFilePointer (device, vt, NULL, FILE_BEGIN) ;  
  		if (!ReadFile(device, buffer, k, &bytesRead, NULL)) {
			return NULL;
  		}
  		return buffer;
	}
    // doc bang FAT chua fcluster
	BYTE* readFAT(int fcluster, uint &d, uint &r) {	
		d = fcluster/(nS/4);        
		r = fcluster%(nS/4);        
		return read((fSectorOfFAT+d)*nS, nS);  // FAT đánh số từ 0, 128 cluster, 256 KB 
	}
	// tra cuu bang fat, tra ve danh sach cac sectors
	int* lookUpFAT(int fcluster, int &n) {   
	    vector<uint> clusters;
	    uint d, r;         // tương ứng bảng fat thứ d, cluster thứ r   
	    uint t = fcluster; // the real cluster thứ  
		BYTE* fat = readFAT(fcluster, d, r);
		while(d!=0 || r>=2) {      // d==0 && r<2 
			clusters.push_back(t); // oke
			//cout << t << endl; 
			t = convert(copy(fat, r*4, 4), 4); // bảng fat thứ d, cluster thứ r  
			if (t >= 0x0FFFFFF8) break;        // 0x0FFFFFFF, 0x0FFFFFF8 // end
			
			if (t/(nS/4)!=d) {                 // có cần đọc lại bảng fat (!= d hiện tại)
				delete[] fat;
				fat = readFAT(t, d, r);        // đọc lại bảng fat, d, r được cập nhật (ứng với đoạn fat mới)
			}
			else {
				r = t%(nS/4);
			}
			//cout << "hello" << endl;
		};
		n = clusters.size()*sC; // sC=4;
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
    // doc cay thu muc, tra ve danh sach files va subfolders
    Component** readFolderTree(int fcluster, Component* parent) {  
    	int n; 
  	 	int* s = v->lookUpFAT(fcluster, n);      // for RDET or SDET // deu doc tu dau bang (index khac)
  	 	
		Component** list = new Component* [52];  // tối da 50 thư mục con  //  con trỏ cũng nhẹ
		for (int i=0; i<52; i++) list[i] = NULL; 
		int index = 2;                       
		
		string name, ext;
		BYTE* det;
		int t;
		// nS = 512, sC=4;
		int nEntries = nS/32; // nS/32 = 16 entry (so entries tren mot bang det)
    	for (int k=0; k<n; k++) {
    		// nS = 512, sC=4;
    		det = read(s[k]*nS, nS); // mot lan doc nS*k (k>=1)
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
					if (j>=0 && det[j]!=0xE5 && det[j]!=0 && jb>=0 && det[jb]==0xf) { // có it nhat mot entry phu
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
						list[index] = new File(name, fclus, updatedHour, updatedDate, size, isTextFile);
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
			pChar(read(s[i]*nS, nS),nS, nS);
		}
		int t = size%nS;
		if (t==0) pChar(read(s[n-1]*nS, nS),nS, nS);
		else pChar(read(s[n-1]*nS, nS),t, t);
	}	
	
	// test area
	void test() {
		 char my_volume[] = "\\\\.\\E:";
 		 device = CreateFile(my_volume,             // Drive to open  // volume_letter
                        GENERIC_READ,           // Access mode
                        FILE_SHARE_READ|FILE_SHARE_WRITE,        // Share Mode
                        NULL,                   // Security Descriptor
                        OPEN_EXISTING,          // How to create
                        0,                      // File attributes
                        NULL);                  // Handle to template
  		if (device != INVALID_HANDLE_VALUE && device != NULL) cout << "ready to read!\n";
		// testing
		readFileContent(8911, 25729);
	}
};

// menu
class Menu {
private:
	int n = 4;
    const char* list[4] = {"Exit", "Load volume", "Volume property", "Root folder"};;
public: 
	Menu() {};
	void introduce() { // gioi thieu ung dung, thanh vien nhom
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
	void display() {
    	int myoption;
   	 	while(1) { 
   	 		system("cls");
   	 		introduce(); 
   	 		v->status();
   	 		cout << "\n-----Menu-----\n";
   			for (int i=0; i<n; i++) {
   	 			cout << i <<". "<< list[i] <<endl;	
			}
			myoption = getOption(0, n-1, 1, 0);
			switch(myoption) {
				case 1:
					v->load();
					break;
				case 2:
					v->properties();
					break;
				case 3:
					v->display();
					break;
				default:
					cout << "Good bye!\n";
					return;
			}
    	}
	}
	
};

Menu* menu = new Menu;  

int main() {
  v = new Volume;
  menu->display();
  return 0;
}

 
// files và subfolder (thông tin từ entry), read RDET (root folder, khong co thong tin tu entry)
// sau này: load master boot record (lấy thông tin các FAT32 partion) 
//volume size:   200MB * 1024(KB) * 1024(bytes) = 209,715,200 bytes
//sector size:   512 bytes -> 409,600 sectors
//clusters size: 2048 bytes (4 sectors) -> 102,400 clusters
 

/*

Cho Volume kế thừa Component, File & Folder dùng qua component* v( but it's not really) // pass compiler 

/*
really important với classs
không dễ để Parent* p = new Child;
p lấy được các thuộc tính của
-> dùng virtual function Get...(), oke
virtual = 0; không thể tạo đối tượng luôn, bắt buộc phải define hàm ở các con
virtual, accessable các con, cho phép overriding
no virtual, accessable các con, không cho phép overriding
-> dùng method để access child's property or manipulate on it :)))
*/
/*
một số hàm không gọi trực tiếp thì khỏi virtual;
*/










