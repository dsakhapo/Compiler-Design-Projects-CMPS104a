// $Id: main.cpp,v 1.6 2014-10-09 15:44:18-07 - - $

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;
#include "auxlib.h"
#include "stringset.h"
#include "string.h"
#include <unistd.h>

const string CPP = "/usr/bin/cpp";
string cpp_opts = "";
constexpr size_t LINESIZE = 1024;
ofstream str_file; //Create a new file that'll have the .str suffix

// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

/*** PROCESS C PREPROCESSOR OUTPUT ***/
void cpplines (FILE* pipe, char* filename) {
   int linenr = 1;   //line number
   char inputname[LINESIZE];

   string str_name = filename;
   size_t found = str_name.find_first_of(".");
   str_name = str_name.substr(0, found);
   str_name += ".str";
   str_file.open(str_name);

   strcpy (inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;  //When you reach the end of file
      chomp (buffer, '\n');
     /* cout << filename << ":line " << linenr << ": "
               << "[" << buffer << "]" << endl;*/
      // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
      int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
                              &linenr, filename);
      if (sscanf_rc == 2) {
         /*cout << "DIRECTIVE: line " << linenr << " file " << "\""
                  << filename << "\"" << endl;*/
         continue;
      }
      char* savepos = NULL;
      char* bufptr = buffer;
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
         /*cout << "token " << linenr << "." <<
                  tokenct << ": [" << token << "]" << endl;*/
         const string* str = intern_stringset (token);
      }
      ++linenr;
   }
}

/*** USER INPUT OPTIONS/OPERATIONS ***/
void user_opts(int argc, char** argv){
   int opt;
   while((opt = getopt(argc, argv, "@:D:ly")) != -1){
      switch(opt){
      case '@'  : set_debugflags(optarg);               break;
      case 'l'  :                                       break;
      case 'y'  :                                       break;
      case 'D'  : cpp_opts = "-D" + string(optarg);     break;
      default   : cerr << "Error: user_opts()" << endl; break;
      }
   }
}

int main (int argc, char** argv) {
   //Set the program name
   set_execname(argv[0]);

   //Check to make sure the program is run w/ arguments
   if(argc < 2){
      cerr << "Error: Run the program with a file input." << endl;
      return EXIT_FAILURE;
   }

   //Process user input options/flag options first
   user_opts(argc, argv);

   //Get the source file name, and make sure it is of type .oc
   char* filename = argv[argc - 1];
   if(string(filename).find(".oc") == string::npos){
      cerr << "Invalid source file." << endl;
      return EXIT_FAILURE;
   }

   //Command to be executed by the shell as a subprocess,
   //the C preprocessor is used to filter a .oc file
   string command = CPP + " " + cpp_opts + " " + filename;
   /*cout << "command = " << "\"" << command << "\""<< endl;*/

   /*** C PREPROCESSOR OPERATIONS ***/
   FILE* pipe = popen (command.c_str(), "r");
   if (pipe == NULL) {
      syserrprintf (command.c_str());
   }else {
      cpplines (pipe, filename);
      int pclose_rc = pclose (pipe);
      eprint_status (command.c_str(), pclose_rc);
      if (pclose_rc != 0) set_exitstatus (EXIT_FAILURE);
   }
  //Dump stringset into .str file
   dump_stringset (cout, str_file);
   str_file.close();
   return EXIT_SUCCESS;
}
