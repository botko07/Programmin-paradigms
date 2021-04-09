using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";




imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}




struct actor{
  string name;
  const void * ptr;
  actor(string name, const void * ptr){
    this->name = name;
    this->ptr = ptr;
  }
};


// compare method for actor/actress
int cmp_fn(const void * p1, const void * p2){
    actor * act_ptr = (actor *)p1;
    char * ptr = (char *)act_ptr->ptr;
    string s1 = act_ptr->name;
    int offset = *(int *)p2;
    string s2 = "";
    char * ptr2 = (char *)ptr + offset;
    while(*ptr2 != '\0'){
      s2 += *ptr2;
      ptr2 += 1;
    }
    if(s1 == s2) return 0;
    if(s1 < s2) return -1;
    return 1;
}

struct data{
  film f;
  const void * ptr;
  data(film f, const void * ptr){
    this->f = f;
    this->ptr = ptr;
  }
};

// compare method for movies
int cmp_film(const void * p1, const void * p2){
    data * mov_ptr = (data *)p1;
    char * ptr = (char *)mov_ptr->ptr;
    string movie = "";
    int offset = *(int *)p2;
    char * pt = (char *)ptr + offset;
    while(*pt != '\0'){
      movie.push_back(*pt);
      pt += 1;
    }
    film original = mov_ptr->f;
    int years = *(pt+1); 
    film f = {movie, years+1900};
    if(f == original) return 0;
    if(original < f)  return -1;
    return 1;
}
void fill_films(char * ptr,vector<film> &films){
    string name = "";
    while(*ptr != '\0'){
      name.push_back(*ptr);
      ptr += 1;
    }
    ptr += 1;
    int year = *(char *)ptr;
    film f = {name, year+1900};
    films.push_back(f);
}


// method return true if actor/actress is in data(in that case it fills vector too), false if not  
bool imdb::getCredits(const string& player, vector<film>& films) const {
  
  actor * info = new actor(player, actorFile);
  int elems = *(int *)actorFile;  
  void *pt = (char *)actorFile + sizeof(int);
  void * ptr = bsearch(info,pt,elems,sizeof(int), cmp_fn);
  if(ptr == NULL) return false;
  // ENDS HERE

  int offset = *(int *) ptr;
  string name;
  char * curr_ptr = (char *)actorFile + offset;
  int len = 0;
  while(*curr_ptr != '\0'){
      name += *curr_ptr;
      curr_ptr += 1;
      len++;
  }
  if(len % 2 == 0) curr_ptr += 1, len++;
  len++;
  curr_ptr += 1;

  
  short movies_amount = *(short *)curr_ptr;
  len += sizeof(short);
  curr_ptr += sizeof(short);
  if(len % 4 == 2){
    len += 2;
    curr_ptr += 2;
  }

  
  for(int i=0 ;i < movies_amount; i++){
      char * pptr = (char *)curr_ptr + sizeof(int)*i;
      offset = *(int *)pptr;
      char * m_ptr = (char *)movieFile + offset;
      fill_films(m_ptr,films);
  }
  delete info;


  return films.size() > 0; 
}


void fill_actors(const void * p, vector<string> &players, int offset){
    char * ptr = (char *)p + offset;
    string name = "";
    while(*ptr != '\0'){
      name.push_back(*ptr);
      ptr += 1;
    }
    players.push_back(name);
}

bool imdb::getCast(const film& movie, vector<string>& players) const { 
  data * info = new data(movie,movieFile);
  int elems = *(int *)movieFile;  
  void *pt = (char *)movieFile + sizeof(int);
  void *ptr = bsearch(info, pt,elems,sizeof(int), cmp_film);
  if(ptr == NULL) return false;

  string movie_name = "";
  char * p = (char *)movieFile + *(int *)ptr;
  while(*p != '\0'){
    movie_name.push_back(*p);
    p += 1;
  }
  p += 1;
  int len = movie_name.size() + 2;
  int year = *p;
  if(movie_name.size() % 2) p += 1, len++;
  p += 1;
  int actors_amount = *(short *)p;
  len += 2; p += 2;
  if(len % 4 == 2){
    p += 2;
    len += 2;
  }

  for(int i=0; i < actors_amount; i++){
    int offset = *(int *) ((char *)p + sizeof(int)*i);
    fill_actors(actorFile,players,offset);
  }
  delete info;

  return players.size() > 0; 
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
