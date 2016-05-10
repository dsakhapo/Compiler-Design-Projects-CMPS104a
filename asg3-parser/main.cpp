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
#include "lyutils.h"

const string CPP = "/usr/bin/cpp";
string cpp_opts = "";
constexpr size_t LINESIZE = 1024;
ofstream str_file; //Create a new file that'll have the .str suffix

// Open a pipe from the C preprocessor.
// Exit failure if can't.
// Assigns opened pipe to FILE* yyin.
void cpp_popen (const char* filename) {
   string cpp_command = CPP + " " + filename;

   //Take care of .tok suffix
   string tok_name = filename;
   size_t found = tok_name.find_first_of(".");
   tok_name = tok_name.substr(0, found);
   tok_name += ".tok";
   tok_file = fopen(tok_name.c_str(), "w");

   yyin = popen (cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (cpp_command.c_str());
      exit (EXIT_SUCCESS);
    //yy_flex_debug messages
   }else {
      if (yy_flex_debug) {
         fprintf (stderr, "-- popen (%s), fileno(yyin) = %d\n",
                  cpp_command.c_str(), fileno (yyin));
      }
      scanner_newfilename (cpp_command.c_str());
   }
}

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
      case 'l'  : yy_flex_debug = 1;                    break;
      case 'y'  : yydebug = 1;                          break;
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
   dump_stringset (str_file);
   str_file.close();

   /***** .TOK FILE OPERATIONS *****/
   cpp_popen(filename);
   /*for(;;){*/
   int yy_val = yyparse();
      /*if(yy_val == YYEOF) break;
   }*/
   fclose(tok_file);

   string ast_name = filename;
   size_t found = ast_name.find_first_of(".");
   ast_name = ast_name.substr(0, found);
   ast_name += ".ast";
   ast_file = fopen(ast_name.c_str(), "w");
   dump_astree(ast_file, yyparse_astree);
   fclose(ast_file);

   //Close the pipe and indicate failure if failed to close.
   int pclose_rc = pclose (yyin);
   if (pclose_rc != 0) exit(EXIT_FAILURE);
   return EXIT_SUCCESS;
}
