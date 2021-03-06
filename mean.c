#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define MAX_NUM_FILES 128
#define NUMS_CHUNK_SIZE 1024

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

typedef struct {
   char *filename;
   double *nums;
   unsigned numcount;
   FILE *fd;
} meanfile;

meanfile* alloc_meanfile(char*);
int is_numeric(const char *);
void add_num(meanfile*, double);
double get_numeric(const char *);
unsigned read_word(FILE *f, char *buffer);

static meanfile* meanfiles[MAX_NUM_FILES];

int main(int argc, char **argv){
   unsigned num_mf = argc-1;

   // Allocate/open all files
   for(unsigned i=1; i<argc; i++){
      char word[1024];
      meanfiles[i-1] = alloc_meanfile(argv[i]);
      meanfile *mf = meanfiles[i-1];
      while(read_word(mf->fd, word) == 1){
         if(is_numeric(word)){
            add_num(mf, get_numeric(word));
         }
      }
   }

   // Ensure that all of the files have the same number of numbers
   unsigned g_numcount = meanfiles[0]->numcount;
   for(unsigned i=1; i<num_mf; i++){
      meanfile *mf = meanfiles[i];
      if(mf->numcount != g_numcount){
         fprintf(stderr, "These files have different structures!\n");
         exit(2);
      }
   }

   double *meannums = malloc(sizeof(double) * g_numcount);
   double *minnums = malloc(sizeof(double) * g_numcount);
   double *maxnums = malloc(sizeof(double) * g_numcount);
   for(unsigned i=0; i<g_numcount; i++){
      // We'll take the arithmetic mean.
      double sum = 0;
      double g_min = meanfiles[0]->nums[i];
      double g_max = meanfiles[0]->nums[i];
      for(unsigned j=0; j<num_mf; j++){
         sum += meanfiles[j]->nums[i];
         g_min = min(g_min, meanfiles[j]->nums[i]);
         g_max = max(g_max, meanfiles[j]->nums[i]);
      }
      double mean = sum / num_mf;
      meannums[i] = mean;
      minnums[i]  = g_min;
      maxnums[i]  = g_max;
   }

   // Using the first file as a model, spit back out the averaged file.
   rewind(meanfiles[0]->fd);
   char word[1024];
   char line[1024*1024];
   unsigned current_num = 0;
   while(fgets(line, sizeof(line), meanfiles[0]->fd)){
      int first = 1;
      char* token;
      const char seps[6] = " \n\r\t";
      token = strtok(line, seps);
      while(token != NULL){
         char tmp[strlen(token)];
         strcpy(tmp, token);

         int had_comma = tmp[strlen(tmp)-1] == ',';
         if(had_comma)
            tmp[strlen(tmp)-1] = '\0';

         int is_num = is_numeric(tmp);

         if(!first)
            printf(" ");

         if(is_num){
            char *mr_str = getenv("MEAN_REDUCTION");
            if(mr_str && strcmp(mr_str, "min")==0){
               printf("%0.3f", minnums[current_num++]);
            }else if(mr_str && strcmp(mr_str, "max")==0){
               printf("%0.3f", maxnums[current_num++]);
            }else{
               printf("%0.3f", meannums[current_num++]);
            }
         }else{
            printf("%s", tmp);
         }

         if(had_comma)
            printf(",");

         token = strtok(NULL, seps);
         first = 0;
      }
      printf("\n");
   }

   return 0;
}

int is_numeric(const char *s){
   if (s == NULL || *s == '\0' || isspace(*s))
      return 0;
   char * p;
   strtod (s, &p);
   return *p == '\0';
}

double get_numeric(const char *s){
   assert(is_numeric(s));
   char * p;
   return strtod (s, &p);
}

meanfile* alloc_meanfile(char* filename){
   meanfile *mf = calloc(1, sizeof(meanfile));
   mf->nums = malloc(sizeof(double)*NUMS_CHUNK_SIZE);
   mf->filename = filename;
   mf->fd = fopen(filename, "r");
   if(mf->fd < 0){
      fprintf(stderr, "Couldn't open file \"%s\" for reading.\n", filename);
      exit(1);
   }
   return mf;
}

void add_num(meanfile *mf, double n){
   if(mf->numcount == NUMS_CHUNK_SIZE){
      fprintf(stderr,
            "Error: exceeded per-file number storage limit of %u doubles!\n",
            NUMS_CHUNK_SIZE);
      exit(1);
   }
   mf->nums[mf->numcount++] = n;
}

unsigned read_word(FILE *f, char *buffer){
   int matches = fscanf(f, " %1023s", buffer);
   unsigned last_char_idx = strlen(buffer)-1;
   if(matches==1 && last_char_idx > 0 && buffer[last_char_idx] == ',')
      buffer[last_char_idx] = '\0';
   return matches;
}

/* vim: set ts=3 sw=3 expandtab shiftround: */
