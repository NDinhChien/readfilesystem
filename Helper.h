#pragma once
#ifndef HELPER_H
#define HELPER_H

// một số hàm hỗ trợ
// copy n bytes from src starting at k
BYTE* copy(BYTE* src, int k, int n) {
  BYTE* dest = new BYTE[n];
  for(int i=0; i<n; i++) {
    dest[i] = src[k+i];
  }
  return dest;
} 
 
// convert a litte edian number to a interger 
int64_t convert(BYTE number[], int n) {   
  int64_t t = 0;
  memcpy(&t, number, n);
  return t;
}
int64_t Get_Bytes(BYTE* sector, int offset, int number)
{
    int64_t k = 0;
    memcpy(&k, sector + offset, number);
    return k;
}
// convert a 16 bit string to a date
string toDate(BYTE date[]) { 
	bitset<7> y;   
	bitset<4> m;  
	bitset<5> d;  
	bitset<8> r(date[0]); 
	bitset<8> l(date[1]);  
	for (int i=6; i>=0; i--) y[i] = l[i+1]; 
	m[3] = l[0];
	for (int i=2; i>=0; i--) m[i] = r[i+5]; 
	for (int i=4; i>=0; i--) d[i] = r[i]; 
	string str(to_string(y.to_ulong()+1980) + "-"+ to_string(m.to_ulong())+ "-"+ to_string(d.to_ulong()));
	return str;
}
// convert a 16 bit string to a time
string toHour(BYTE hour[]) {
    bitset<5> h;  
	bitset<6> m;  
	bitset<5> s;   
	bitset<8> r(hour[0]);  
	bitset<8> l(hour[1]);  
	for (int i=4; i>=0; i--) h[i] = l[i+3]; 
	m[5] = l[2]; m[4]= l[1]; m[3]= l[0];
	for (int i=2; i>=0; i--) m[i] = r[i+5]; 
	for (int i=4; i>=0; i--) s[i] = r[i]; 
	string str(to_string(h.to_ulong()) + ":"+ to_string(m.to_ulong())+ ":"+ to_string(s.to_ulong()));
	return str;
} 
// print string
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
// print string in hexa form
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

// check if a char is a letter
bool isLetterChar(char chr) {
	if (('a'<=chr && chr <='z') || ('A'<=chr && chr <='Z')) return 1;
	return 0;
}
// check if a char is a number letter
bool isNumberChar(char chr) {
	if ('0'<=chr && chr <='9') return 1;
	return 0;
}
// get option, if isletterAccepted = 1, accept some letters: R/r, B/b
int getOption(int start, int end, int step, bool isletterAccepted) { 
	string rs;
	int tmp, l;
	bool isNumber;
	int plus=isletterAccepted*2;  
	while(1) {  
		cout <<">> ";
		cin >> rs;
		l = rs.length();
		if (isletterAccepted==1 && l==1 && isLetterChar(rs[0])) {
			if (rs[0]=='r' || rs[0]=='R') return 1;
			if (rs[0]=='b' || rs[0]=='B') return 0;
			continue;
		}
		isNumber=true;
		for (int i=0; i<l; i++) {
			if (!isNumberChar(rs[i])) {
				isNumber=false;
				break;
			}
		}
		if (isNumber) {
			tmp = stoi(rs);
			if (tmp>=start && tmp<=end && (tmp-start)%step==0) return tmp+plus;
		} 
	}
} 
// read k bytes starting from vt
BYTE* read(HANDLE device, int vt, int k) {      
	DWORD bytesRead;
	BYTE* buffer = new BYTE[k]; 
	SetFilePointer (device, vt, NULL, FILE_BEGIN) ;  
	if (!ReadFile(device, buffer, k, &bytesRead, NULL)) {
		return NULL;
	}
	return buffer;
}
// convert an integer to binary
string toBinary(int n) {
    string r;
    while(n!=0) {
		r = (n%2==0 ? "0":"1") + r; 
		n /= 2;
	}
    return r;
}
// bitwise complement (~) of a binary string 
string bitwise_complement(string strbit){
    for (int i = 0; i < strbit.length(); i++){
       strbit[i] = (strbit[i] == '0' ? '1' : '0');
    }
    return strbit;
}
#endif