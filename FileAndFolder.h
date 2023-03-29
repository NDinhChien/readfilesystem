#pragma once
#ifndef FILEANDFOLDER_H
#define FILEANDFOLDER_H

class File :public Component{
private:
  bool isTextFile;
public:
  File():Component() {};
  ~File() {};
  File(string name, int fcluster, string updatedHour, string updatedDate, int size, Component* parent, bool isTextFile) 
  	: Component(name, fcluster, updatedHour, updatedDate, size, parent) {
  		this->isTextFile = isTextFile;
  };
  void display() {  
  	system("cls");
  	properties(); 
  	cout << "\n-----Content-----\n";       
  	
	if (isTextFile) {
		if (size==0) {
			cout << "<empty>\n";
		}
		else v->readFileContent(fcluster, size); 
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
  int isFile() {return 1;};
};


class Folder :public Component{
private:
  Component** list=NULL;  

public:
  Folder():Component() {};
  Folder(string name, int fcluster,  Component* parent) 
    :Component(name, fcluster, parent) {};                                 // for root folder, fcluster = 2, parent==NULL;
      
  Folder(string name, int fcluster, string updatedHour, string updatedDate, int size, Component* parent) 
    :Component(name, fcluster, updatedHour, updatedDate, size, parent) {}; // for sub folder
  int getSize() {
    if (list==NULL) return 0; 
    int total;
  	for (int i=2; i<52; i++) { 
  		if (list[i]!=NULL) {
  			total += list[i]->getSize();
		}
  		else break;
	}
	return total;
  }

  void display() {                    
  	load();        // reload list // all files and subfolder properties are updated 
    int myoption;
    while(1) { 
      system("cls");
      properties();
	  cout << "\n-----Content-----\n";   
	  cout << name << endl;                     
      cout << "  B. Back\n";
      cout << "  R. Refresh\n\n";
      int i=2;
	  for (; i<52; i++) {
	  	if (list[i]!=NULL) {
	  		cout <<"  "<< i-2<<". "<< list[i]->getName()<<endl;
		}
		else break;
	  }
	  if (i==2) {
	  	cout << "<empty>\n\n";	
	  }
	  myoption = getOption(0, i-3, 1, true); 
      if (myoption==0) return;
	  if (myoption==1) load();  
	  else list[myoption]->display(); 
	}
  }
  
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