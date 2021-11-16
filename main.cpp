/*
DESCRIPTION: JSON C library
VERSION: 0.1
AUTHOR: Marat Nagaev
HOMEPAGE: https://github.com/nagayev/simple-json
DESCRIPTION: 
Simple JSON lib for C++
Don't use it in production, 'cause it's Very slow, 
only for educational purposes
LICENSE: MIT
*/
//201112L
/*#if (__STDC_VERSION__)<201112L
#error "Your compiler is old. Update it to support C++11"
#else 

#endif */
//types.h
#define BOOLEAN 0
#define NUMBER 1
#define STRING 2
#define ARRAY 3
#define OBJECT 4

//erros.h
#define SUCCESS 0
#define ERROR_UNKNOWN 1
#define ERROR_INVALID_BOOLEAN 2
#define ERROR_INVALID_NUMBER 3
#define ERROR_STRING 4
#define ERROR_ARRAY 6
#define ERROR_OBJECT 6


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stack>
#include <sstream>

using namespace std;

struct JSON_data {
        int8_t error;
        int8_t root_type; //for proper type casting
        void* data; //NOTE: MAGIC! dynamic typing
};

int8_t determite_type(string buffer){
    char c = buffer[0];
     if (c=='\"'){
        return STRING;
    }
  else if ( (int)c-48>=0 && (int)c-49<=9 ){
       return NUMBER;
    }
  else if (c=='['){
    return ARRAY;
    } 
  else if (c=='{'){
      return OBJECT;
    }
 return BOOLEAN;
}

//see https://stackoverflow.com/a/27511119
vector<string> split(const string &s, char delim) {
  stringstream ss(s);
  string item;
  vector<string> elems;
  while (getline(ss, item, delim)) {
    elems.push_back(std::move(item));
  }
  return elems;
}

//see https://stackoverflow.com/a/63087333
double* parse_number(string buffer) { 
    //cout<<"parse_number was called with args: "<<buffer<<endl;
    char *ep; // to point to first char after the number

    if (isdigit(*buffer.c_str())) { // make sure first char is a digit

        // convert, and find first invalid char
        static double x = strtod(buffer.c_str(), &ep); 

        // return conversion if first invalid char was the
          // terminating null
        if (!(*ep)){
            cout<<"x is"<<x<<endl;
            return &x;
        }
        else return &x;
    }
    cout<<"fuck"<<endl;
    return nullptr; // otherwise return nullptr
}

bool* parse_boolean (string buffer){
  bool* b;
  if (buffer=="true") *b = true;
  else if (buffer=="false") *b = false;
  else b = nullptr;
  return b;  
}

map<string,string> parse_object(string buffer){
  map<string, string> m;
  vector<string> v; //key-value
  //if we don't have , push it
  bool have_comma = buffer.find(',',0)!=string::npos;
  if (!have_comma) buffer.push_back(',');
  //v=split("a:1,b:2",','); //['a:1','b:2']
  v=split(buffer,',');
  string s,key,value;
  int index;
  //parsing pairs
  for(int i=0;i<v.size();i++){
	  s = v[i];
	  index=s.find(":",0); //TODO: symbol `:` in the key or value will break this 
	  if (index==string::npos) {
		  //TODO: raise error ERROR_OBJECT
		  cout<<"ERROR: no :"<<endl;
	  }
	  key=s.substr(0,index); //s.slice(0,index)
	  value=s.substr(index+1);
	  m[key]=value;
	  //cout<<"key: "<<key<<" value: "<<value<<endl;
  }
  return m;
}


JSON_data parse_json(string json_str){
  int8_t type,element;
  stack<int8_t> braces; //for [,{,"
  static vector<JSON_data> v; //for array recursion
  map <string,void*> m; //for objects
  char c,previous; //current and previous symbols
  string buffer, child_buffer;
  
  struct JSON_data result,object_node;
  result.error=SUCCESS;
  /*static*/ void* data = nullptr;
  
  //determite type of the root element
  
  c = previous = json_str[0];
  
  //алгоритм такой: видим начало элемента, читаем все в буфер. видим конец - парсим строку
  type=determite_type(json_str);
  result.root_type=type;
  if (type!=BOOLEAN){
	braces.push(type);  
  }
  for (int i=1;i<json_str.size();i++){
      char c = json_str[i];
      if(c==']'){
          //array was closed
          //TODO: parse_array
            int j;
            buffer.push_back(','); //последний элемент массива не попадет в буфер
            for (j=0; j<buffer.size(); j++) {
                c = buffer[j];
                if (c==','){
                    type = determite_type(child_buffer);
                    object_node.error=SUCCESS;
                    switch (type){
                        case NUMBER: {
							double d = stod(child_buffer);
							//TODO: replace stod with correct parse_number
							if(0){
								object_node.error=ERROR_INVALID_NUMBER;
								break;
							}
                            object_node.data=(void*)(new double(d));
                            object_node.root_type=NUMBER;
                            break;
							}
                        case STRING:
                            object_node.data=(void*)(new string(child_buffer));
                            object_node.root_type=STRING;
                            break;
                        default:
                            object_node.data=nullptr;
                            object_node.error=ERROR_UNKNOWN;
                    }
					v.push_back(object_node);
                    child_buffer="";
                }
              else child_buffer+=c;    
            }
            i+=buffer.size()-1;
            data=(void*)&v;
            buffer="";
          
          /*if (braces.size()==0){
             //error:
             result.error=1;
          }
          element = braces.top();
          if (element!=ARRAY){
              //error: not array started
              result.error=2;
          }
          if (previous==','){
              //error: element expected
              result.error=3;
          }
          if (result.error!=0) return result; */
          braces.pop();
          continue;
      }
      else if (c=='}'){
          //object ended
          parse_object(buffer);
          buffer="";
          /*if (braces.size()==0){
             //error: 
             result.error=1;
          }
          element = braces.top();
          if (element!=OBJECT){
              //error: not object started
              result.error=2;
          }
          if (result.error!=0) return result; */
          braces.pop();
      }
      else if (c=='[' || c=='{'){
         type = (c=='[') ? ARRAY:OBJECT;
         braces.push(type); 
      }
      else if (c=='\"'){
         element = braces.top();
		 //ignore parsing strings inside objects
		 if(element==ARRAY || element==OBJECT){
			 buffer+=c;
			 continue;
		 }
		  
         if (element!=STRING){
             //строка еще не началась
             braces.push(STRING);
        }
        else {
            //close string
            data=(void*)new string(buffer);
            buffer="";
            braces.pop();
        }
      }
      /*else if (c==','){
         //',' used as a separator
          element = braces.top(); 
          if (element==ARRAY || element==OBJECT){ //if we don't inside of the string
             //parse buffer
              
          }
      } */
      else {
        //user's data
        buffer+=c;
      }
      previous=c;
  }
  if(buffer.size()!=0 && data==nullptr){
     //we have unclearned buffer
     switch(result.root_type){
         case NUMBER:
            data=(void*)parse_number(buffer);
            if (data==nullptr) result.error=ERROR_INVALID_NUMBER;
            break;
         case STRING:
             data=(void*)&buffer; //it isn't nesseacery to parse string
             if (data==nullptr) result.error=ERROR_STRING;
             break;
         case BOOLEAN:
             data=(void*)parse_boolean(buffer);
             if (data==nullptr) result.error=ERROR_INVALID_BOOLEAN;
    }
    if (result.error) return result;
    result.data=data;
    return result;
  }
  if (data!=nullptr){
     result.data=data;    
  }
  result.error = braces.size()==0?SUCCESS:ERROR_UNKNOWN; //FIXME:
  return result;
}

int main ()
{
  string json_str = "{\"a\":123,\"b\":456}";
  struct JSON_data JSON;
  struct JSON_data inner_JSON;
  JSON = parse_json(json_str);
  vector<JSON_data> v;
  if (JSON.error!=0) cout<<"Fatal JSON Error"<<endl;
  
  else{
      //Getting data
	  cout<<"NOTE: root_type is";
      switch(JSON.root_type){
          case BOOLEAN:
              cout<<"boolean!"<<endl;
              cout<<*(bool*)JSON.data;
              break;
          case NUMBER:
              cout<<"number!"<<endl;
              cout<<*(double*)JSON.data;
              break;
          case STRING: {
              cout<<"string!"<<endl;
			  string s = *(string*)JSON.data;
              cout<<s;
              break;
		  }
          case ARRAY:
              cout<<"array!"<<endl;
              v = *(vector<JSON_data>*)JSON.data;
              if (v.size() == 0) cout<<"Error: size of vector is 0";
              else{
                cout<<"NOTE: Size of the vector is: "<<v.size()<<endl;
                inner_JSON = v[1];
                if (inner_JSON.data==nullptr) cout<<"ERROR: no data"<<endl;
                else{
					auto value = *(double*)inner_JSON.data;
					cout<<value<<endl;
				}   
              }
              break;
          case OBJECT:
              cout<<"object!"<<endl;
              break;
          default:
              cout<<endl<<"FIXME: Unknown type or NotImplemented";
    }
  }
  cout<<endl;
  
  return 0;
}
  
