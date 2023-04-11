#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H

class Component :public IFace{
protected:
  string name; 
  uint size; 
  // fat32 file and folder
  uint fcluster;    
  string createdHour;  
  string createdDate;  
  IFace* parent;
  // ntfs file and folder
  uint ID;
  int parentID;
public:
  Component() {}; 
  ~Component() {};
  // fat32
  Component(string name, uint fcluster, IFace* parent) {  // for root folder
  	this->name = name;
    this->fcluster = fcluster;
    this->parent = parent;
  }
  Component(string name, uint fcluster, string createdHour, string createdDate, uint size, IFace* parent) { // for file or sub folder 
    this->name = name;
    this->fcluster = fcluster;
    this->createdHour = createdHour;  
  	this->createdDate = createdDate; 
  	this->size = size;
  	this->parent = parent;
  }
  // ntfs
  Component(string name, uint size, uint ID, int parentID) {
    this->name = name;
    this->size = size;
	this->ID = ID;
    this->parentID = parentID;
  }  

  string getName() {return name;};
  uint getSize() {return size;};
  IFace* getParent() {return parent;};
  int getParentID() { return parentID;};
  uint getID() { return ID;};
  void properties() {  
  	cout << name << endl;
  	cout << "First cluster: "<< fcluster<<endl;
  	cout << "Size         : "<< getSize() <<" bytes" <<endl;
  	if (name=="root" && parent==NULL) return;  
	cout << "Created hour : " << createdHour<<endl;
  	cout << "Created date : " << createdDate<<endl;
  }
}; 

class File :public Component{
private:
  bool isTextFile;
  // ntfs file
  uint dataOffset; // to read file content
  bool isResident; 
public:
  File():Component() {};
  ~File() {};
  File(string name, uint fcluster, string createdHour, string createdDate, uint size, IFace* parent, bool isTextFile) // for fat32
  	: Component(name, fcluster, createdHour, createdDate, size, parent) {
  		this->isTextFile = isTextFile;
  };
  File(string name, uint size, uint ID, int parentID, bool isResident, uint dataOffset, bool isTextFile) // for ntfs
    : Component(name, size, ID, parentID) {
    	this->isResident = isResident;
    	this->dataOffset = dataOffset;
        this->isTextFile = isTextFile;
	}
  void display() {  
  	system("cls");
  	properties(); 
  	cout << "\n-----Content-----\n";    
	if (isTextFile) {
		if (v->isNTFSVolume()) {
			if (!isResident) {
  			cout << "Opp! We can not read file that has non-resident attribute!\n";	
			}
			else if (size==0) {
				cout << "<empty>\n";
			}
			else v->readFileContent(ID, dataOffset, size);
		}
		else {
			if (size==0) {
			cout << "<empty>\n";
			}
			else if (v->isFAT32Volume()) {
				v->readFileContent(fcluster, size);	
			}
		}
	}
	else {
		cout << "Opp, Our system can only be able to read the content of text files!"; 
	}
	cout << endl;
    system("pause");
    system("cls");
  }
  void properties() { 
    cout <<"----Properties----\n";
    if (v->isNTFSVolume()) {
    	cout << "File name    : "<< name << endl;
    	cout << "ID           : "<< ID << endl;
    	if(isTextFile) cout << "Resident     : "<< isResident<<endl;
    	if (isResident) cout << "Size         : "<< size <<endl;
  		cout << "Parent id    : "<< parentID <<endl;  		
    	return;
	} 
    cout << "File name    : ";
    Component::properties();
  }
  int isFile() {return 1;};
};


class Folder :public Component {
private:
  // fat32 folder
  IFace** list=NULL;  
  // ntfs folder
  vector<int> chIndex;   // children index
  
public:
  Folder():Component() {};
  // for fat32 folder
  Folder(string name, int fcluster, IFace* parent)      // for root folder , fcluster = 2, parent==NULL 
    :Component(name, fcluster, parent) {};                                
  Folder(string name, int fcluster, string createdHour, string createdDate, int size, IFace* parent) // for sub folder 
    :Component(name, fcluster, createdHour, createdDate, size, parent) {}; 
  // for ntfs folder
  Folder(string name, uint size, uint ID, int parentID) // for sub folder, root folder
    :Component(name, size, ID, parentID) {};

  uint getSize() {
  	if (v->isNTFSVolume()) {
  		return size;
	}
  	// fat32
    if (list==NULL) return 0; 
    uint total;
  	for (int i=2; i<52; i++) { 
  		if (list[i]!=NULL) {
  			total += list[i]->getSize();
		}
  		else break;
	}
	return total;
  }

  void display() {
  	load();
    IFace* tmp;  
    int myoption;
    bool isNTFSVolume = v->isNTFSVolume();
    while(1) { 
      system("cls");
      properties();
	  cout << "\n-----Content-----\n";   
	  cout << name << endl;                     
      cout << "  B. Back\n";
      cout << "  R. Refresh\n\n";
      int i=2;
      if (isNTFSVolume) {  
        int l = chIndex.size();
      	for (; i<l; i++) {
      		tmp = v->getComponent(chIndex[i]); 
	  		if (tmp!=NULL) {  
	  			cout <<"  "<< i-2 <<". "<< tmp->getName()<<endl;
			}
			else {
				cout << "bat on 2\n";
			}
	  	}
	  }
      else {  
      	for (; i<52; i++) {
	  		if (list[i]!=NULL) {
	  			cout <<"  "<< i-2 <<". "<< list[i]->getName()<<endl;
			}
			else break;
	  	}
	  }
	  if (i==2) {
	  	cout << "<empty>\n\n";	
	  }
	  myoption = getOption(0, i-3, 1, true); 
      if (myoption==0) return; 
	  if (myoption==1) load();  
	  else if (isNTFSVolume) v->getComponent(chIndex[myoption])->display(); 
	  else list[myoption]->display(); 
	}
  }
  
  void load() {
    if (v->isNTFSVolume()) { 
	    freeMemory(); 
	    chIndex.push_back(parentID); 
		chIndex.push_back(ID);       
		v->freeMemory(ID);  
    	v->loadAllComponent();
    	v->readFolderTree(ID, chIndex); 
		return;
	} 
	// fat 32
  	freeMemory(); 
	IFace* me = (IFace*)this;
  	list = v->readFolderTree(fcluster, me);   
  	list[0] = parent; 
 	list[1] = this; 
  }
  void countChildren() {
  	int nfi=0,nfo=0;
  	IFace* tmp;
  	if (v->isNTFSVolume()) {
  		int l = chIndex.size();
  		for (int i=2; i<l; i++) {
  			tmp = v->getComponent(chIndex[i]); 
  			if (tmp!=NULL) {
  				nfi += tmp->isFile();
  				nfo += tmp->isFolder();	
			} 
		}
	}
	else { // fat32
		for (int i=2; i<52; i++) {
  	 		if (list[i]!=NULL) {
  	 			nfi += list[i]->isFile();
  	 			nfo += list[i]->isFolder();
			}
			else break;
		}
	}
	cout << "Number of files     : " << nfi << endl;
  	cout << "Number of subfolders: " << nfo << endl; 
  }
  
  void properties() {
  	cout <<"----Properties----\n";
  	if (v->isNTFSVolume()) {
  		cout << "Folder name  : "<< name << endl;
  		cout << "ID           : " << ID <<endl;
  		if (parentID!=-1) cout << "Parent id    : "<< parentID <<endl;
	}
	else {
		cout << "Folder name  : ";
		Component::properties(); 
	}
	countChildren();
  }
  int isFolder() {return 1;};
  void freeMemory() {
  	if (v->isNTFSVolume()) {
  		chIndex.clear();
  		return;
	}
  	// fat32
	if (list==NULL) return;
	for (int i=2; i<52; i++) { 
    	if (list[i]!=NULL) delete list[i];
    	else break;
	}
	delete[] list; 
	list = NULL;
  }
  ~Folder() {
  	freeMemory();
  }
};
#endif