
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"

FILE* ast_file = nullptr;
astree::astree (int symbol, int filenr, int linenr,
                int offset, const char* clexinfo):
        symbol (symbol), filenr (filenr), linenr (linenr),
        offset (offset), lexinfo (intern_stringset (clexinfo)) {
   DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           (void*) this, filenr, linenr, offset,
           get_yytname (symbol), lexinfo->c_str());
}

astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->lexinfo->c_str(),
           child, child->lexinfo->c_str());
   return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;
}

astree* adopt3 (astree* root, astree* child1,
         astree* child2, astree* child3){
   adopt1(root, child1);
   adopt1(root, child2);
   adopt1(root, child3);
   return root;
}

/*astree* adopt_sym (astree* root, astree* child, int symbol){
   root->symbol = symbol;
}*/

static void dump_node (FILE* outfile, astree* node) {
   const char *tname = get_yytname (node->symbol);
   if (strstr (tname, "TOK_") == tname) tname += 4;

   fprintf (outfile, "%s \"%s\" %ld:%ld.%03ld",
            tname, node->lexinfo->c_str(),node->filenr,
            node->linenr, node->offset);
   bool need_space = false;
   for (size_t child = 0; child < node->children.size();
        ++child) {
      if (need_space) fprintf (outfile, " ");
      need_space = true;
      /*fprintf (outfile, "%p", node->children.at(child));*/
   }
   /*fprintf (outfile, "]}");*/
}

static void dump_astree_rec (FILE* outfile, astree* root,
                             int depth) {
   if (root == NULL) return;
   int size = depth/3;
   for (int i = 0; i <depth; i++)
      fprintf (outfile, "%s%*s ", "|", i+3, "");
   dump_node (outfile, root);
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size();
        ++child) {
      dump_astree_rec (outfile, root->children[child],
                       depth + 1);
   }
}

void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void yyprint (FILE* outfile, unsigned short toknum,
              astree* yyvaluep) {
   if (is_defined_token (toknum)) {
      dump_node (outfile, yyvaluep);
   }else {
      fprintf (outfile, "%s(%d)\n",
               get_yytname (toknum), toknum);
   }
   fflush (NULL);
}

void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
   DEBUGF ('f', "free [%p]-> %d:%d.%d: %s: \"%s\")\n",
           root, root->filenr, root->linenr, root->offset,
           get_yytname (root->symbol), root->lexinfo->c_str());
   delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
   free_ast (tree1);
   free_ast (tree2);
}

