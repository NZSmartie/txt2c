#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <list>
#include <algorithm>

using namespace std;

string weekdays[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
string months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

int main(int argc, char ** argv){
	fstream file,output;
	char ch;
	string name, outputDir, ext, mime, header;
	list<string> names;
	list<string>::iterator namesLt = names.begin();
	ostringstream etag, datestamp;

	bool insertHTML = false,createHeader = false, createServerHeader = false;

	if(argc>1 && strcmp(argv[1],"--help")==0){
		cout << "Txt2C - Converts text files to .c files for use in web servers" << endl;
		cout << "Usage: " << argv[0] << " [options] file1 [file2 ... filen]" << endl;
		cout << "Options:";
		cout << "\r\n\t-d directory :Set output directory for the generate files";
		cout << "\r\n\t-i file      :Generate header file to include in your project.";
		cout << "\r\n\t-h           :Insert HTTP Header inside output source file.";
		cout << "\r\n\t-s           :Create a generic HTTP header for the webserver (mostly for the updated date-time)" << endl;
		exit(0);
	}
	
	time_t timeNow = time(0);
	tm *timeStruct = gmtime(&timeNow);// = (tm*)malloc(sizeof(tm));
	if(!timeStruct){
		cerr << "gmtime_s returned NULL errno" <<endl;
		return 1;
	}

	size_t pos = 0;

	etag << weekdays[timeStruct->tm_wday]
		<< setfill('0') << setw(2) << timeStruct->tm_mday
		<< months[timeStruct->tm_mon]
		<< (1900 + timeStruct->tm_year)
		<< setfill('0') << setw(2) << timeStruct->tm_hour
		<< setfill('0') << setw(2) << timeStruct->tm_min
		<< setfill('0') << setw(2) << timeStruct->tm_sec;

	datestamp << weekdays[timeStruct->tm_wday] << ", " << setfill('0') << setw(2) << timeStruct->tm_mday << " " << months[timeStruct->tm_mon];
	datestamp << " " << (1900 + timeStruct->tm_year) << " " << setfill('0') << setw(2) << timeStruct->tm_hour << ":" << setfill('0') << setw(2) << timeStruct->tm_min;
	datestamp << ":" << setfill('0') << setw(2) << timeStruct->tm_sec << " GMT";
//	cout << put_time(timeStruct,"%a, %d %b %Y %H:%M:%S") << " GMT" << endl;
	cout << datestamp << endl;
	cout << mktime(timeStruct) << endl;

	for(int i=1;i<argc;i++){
		if(argv[i][0] == '-'){
			for(unsigned int j=1;j<strlen(argv[i]);j++){
				ch = argv[i][j];
				if(ch >= 'A' && ch <='Z')
					ch += 32; // convert to lower case a-z
				if(ch == 'h'){
					insertHTML = true;
					cout << "HTML Headers enabled" << endl;
				}else if(ch == 'i'){
					createHeader = true;
					header = argv[i+1];
					i++;
					break;
				}else if(ch == 'd'){
					outputDir = argv[i+1];
					if(outputDir.find_last_of("\\/") != (outputDir.length()-1)){
						outputDir += "/";
					}
					cout << "Output dir set to: " << outputDir << endl;
					i++;
					break;
				}else if(ch == 's'){
					createServerHeader = true;
					cout << "Creating Server HTTP Header" << endl;
				}else{
					cout << "Unknown option '" << ch << "'" << endl;
					return 1;
				}
			}
			continue;
		}
		cout << i << ": " << argv[i];
		file.open(argv[i],fstream::in);
		if(!file.is_open()){
			cout << " - Unable to open file!";
			return 1;
		}

		cout << " - Opened" << endl;

		name = argv[i];
		name = name.substr(name.find_last_of("\\/")+1);
		
		pos = name.find_last_of(".");
		if(pos != string::npos)
			ext = name.substr(pos+1);
		pos = 0;
		while((pos = name.find_first_of('.',pos))!=string::npos){
			name[pos] = '_';
		}
		cout << "\t- creating output file \"" << (name + ".c") << "\"" << endl;

		output.open(string(outputDir + name + ".c").c_str(),fstream::out | fstream::trunc);

		output << "#include <time.h>\n\n";

		output << "const char " << name << "_LM[] = {\"" << etag.str() << "\"};\n";

		names.push_back(name);
		if(insertHTML){

			if(ext == "css"){
				mime = "text/css";
			}else if(ext == "js"){
				mime = "text/javascript";
			}else if(ext == "html"){
				mime = "text/html";
			}else if(ext == "htm"){
				mime = "text/html";
			}else{
				cout << "Unknown extention \"" << ext << "\""<<endl;
				mime = "text/plain";
			}

			output << "const char " << name << "_304[] = {\n\"";
			output << "HTTP/1.0 304  Not Modified\\r\\n\"\n";
			output << "\"Date: " << datestamp.str() << "\\r\\n\"\n";
			output << "\"Server: PSTN-ETH/2.07\\r\\n\"\n";
			output << "\"ETag: " << etag.str() << "\\r\\n\"\n";
			//output << "\"Expires: Thu, 01 Jan 1970 00:00:00 GMT\\r\\n\"\n";
			//output << "\"Cache-Control: no-cache\\r\\n\"\n";
			output << "\"Accept-Ranges : bytes\\r\\n\"\n";
			output << "\"Content-type: " << mime << "; charset=UTF-8\\r\\n\"\n";
			output << "\"Content-Language: en\\r\\n\"\n";
			output << "\"\\r\\n\"\n};\n\n";

			output << "const char " << name << "[] = {\n\"";
			output << "HTTP/1.0 200 OK\\r\\n\"\n";
			output << "\"Date: " << datestamp.str() << "\\r\\n\"\n";
			output << "\"Server: PSTN-ETH/2.07\\r\\n\"\n";
			output << "\"ETag: " << etag.str() << "\\r\\n\"\n";
			//output << "\"Expires: Thu, 01 Jan 1970 00:00:00 GMT\\r\\n\"\n";
			//output << "\"Cache-Control: no-cache\\r\\n\"\n";
			output << "\"Accept-Ranges : bytes\\r\\n\"\n";
			output << "\"Content-type: " << mime << "; charset=UTF-8\\r\\n\"\n";
			output << "\"Content-Language: en\\r\\n\"\n";
			output << "\"\\r\\n\"\n\"";
		}else{
			output << "const char " << name << "[] = {\n\"";
		}

		while(file.good()){
			ch = file.get();
			if(file.eof())
				break;
			switch(ch){
				case '\r':
					break;
				case '\n':
					output << "\\r\\n\"\n\"";
					break;
				case '"':
					output << "\\\"";
					break;
				case '\\':
					output << "\\\\";
					break;
				default:
					output.put(ch);
					break;
			}
		}
		output << "\"\n};" << endl;
		output.close();
		file.close();
	}

	if(createServerHeader){
		output.open(string(outputDir + "server.c").c_str(),fstream::out | fstream::trunc);
		output << "const char https_Head[] = {\n";
		output << "\"Date: " << datestamp.str() << "\\r\\n\"\n";
		output << "\"Last-Modified: " << datestamp.str() << "\\r\\n\"\n";
		output << "\"Server: PSTN-ETH/2.07\\r\\n\"\n";
		output << "\"Expires: Thu, 01 Jan 1970 00:00:00 GMT\\r\\n\"\n";
		output << "\"Cache-Control: no-cache\\r\\n\"\n";
		output << "\"Accept-Ranges : bytes\\r\\n\"\n";
		output << "\"Content-Language: en\\r\\n\"\n";
		output << "\n};" << endl;
		output.close();
		names.push_back("https_Head");
	}

	if(createHeader){
		cout << "Creating header file '" << header << "'" << endl;
		output.open(string(outputDir + header).c_str(),fstream::trunc | fstream::out);
		if(!output.is_open()){
			cout << " - Unable to create file!" << endl;
			return 1;
		}
		output << "/*\n * This file has been generated by " << argv[0] << "\n * Do not modify if file gets generated on each build\n *\n * Author: Roman Vaughan\n * Date: " << __DATE__ << "\n */\n\n";
		pos = 0;
		while((pos = header.find_first_of('.',pos))!=string::npos){
			header[pos] = '_';
		}
		for(string::iterator it = header.begin();it!=header.end();it++)
			*it = toupper(*it);
		
		output << "#ifndef __" << header << "\n#define __" << header << "\n\n";
		for(namesLt = names.begin();namesLt != names.end();namesLt++){
			output << "extern struct tm " << *namesLt << "_LM;\n";
			output << "extern const char " << *namesLt << "_304[];\n\n";
			output << "extern const char " << *namesLt << "[];\n\n";
		}

		output << "\n #endif\n";
		output.close();
	}
	
	return 0;
}
