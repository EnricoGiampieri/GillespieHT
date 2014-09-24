typedef struct {
	char * name;
        double * stechioMatrix;
	double * Karray;
	double * status;
	int numReactions;
	int numSpecies;
	double time;
} BioSystem;


char * fget_contents(char * );
BioSystem extract_from_json(char * json_system);
int readDirectoryNum(char *);
void readDirectory(char * ,char ** , int );
void store_data(char *, char **);
char * getTime(char *);


