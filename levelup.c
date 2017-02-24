//definitions
#include "levelup.h"
//functions and logic
#include "parse_ENSDF.c"
#include "db_ops.c"

int main(int argc, char *argv[])
{
	if(!getenv("ENSDF")) 
		{
			printf("\nThe environment variable ENSDF is not set.\nWithout it, this program cannot find the ENSDF data files.  Set the envirnoment variable to point to the directory containing ENSDF data files using the following command (replace the path with the appropriate one):\n\n");
			printf ("export ENSDF=/path/to/ensdf/\n\n");
			exit(-1);
		}
    
	FILE *db;
  
  //allocate structures
  gdata *gd=(gdata*)malloc(sizeof(gdata));
  initialize_database(gd);
	printf("Data structures allocated.\n");
	
	
	//process the ENSDF database file
	char fileName[256];
	int i;
	strcpy(fileName,"");
	strcat(fileName,getenv("ENSDF"));
	strcat(fileName,"ensdf_db");
	if((db=fopen(fileName,"r"))==NULL)
		{
			printf("ENSDF database file not found, rebuilding database...\n");
			rebuildDatabase(gd,db);
			db=fopen(fileName,"r");//reopen file for reading
		}
	i=fread(gd,sizeof(gdata),1,db);
	if(i==1)
		{
			printf("ENSDF database retrieved from disk.  Data for %i nuclei found.\n",gd->numNucl);
		}
	else
		{
			printf("ERROR: ENSDF database could not be read from file '%s'.  Exiting...\n",fileName);
		}
	fclose(db);
	
	
	//process user commands
	printf("\nEnter command.  Enter 'help' for a list of commands.\n");
	char cmd[256],cmd2[256];
	char *tok;
	while(1)
		{
			printf("LevelUp > ");
  		fgets(cmd,256,stdin);
  		cmd[strcspn(cmd, "\r\n")] = 0;//strips newline characters from the string read by fgets
  		if(strcmp(cmd,"help")==0)
  			{
  				printf("Command list:\n");
  				printf("  casc NUCL - prints cascade data for the specified nucleus\n");
  				printf("              (NUCL is the nucleus name, eg. '68SE')\n");
  				printf("  lev NUCL - prints level data for the specified nucleus\n");
  				printf("             (NUCL is the nucleus name, eg. '68SE')\n");
  				printf("  listnuc - list names of nuclei in the ENSDF database\n");
  				printf("  rebuild - rebuild the ENSDF database from ENSDF files on disk\n");
  				printf("  help - list commands\n");
  				printf("  exit - exit the program\n");
  			}
  		else if((strcmp(cmd,"exit")==0)||(strcmp(cmd,"quit")==0))
  			{
  				printf("Exiting...\n");
  				exit(0);
  			}
  		else if(strTokCmp(cmd,"casc",0)==0)
  			{
  				strcpy(cmd2,cmd);
  				tok=strtok (cmd2," ");
					tok = strtok (NULL, " ");//read the 2nd entry in the command
					int nucl=nameToNuclIndex(tok,gd);
					if ((tok != NULL) && (nucl>=0))
						{
  						printf("Printing cascade data for nucleus %s.\n",tok);
  						showCascadeData(nucl,gd);
  					}
  				else
  					printf("Unknown nucleus: %s\n",tok);
  			}
  		else if(strTokCmp(cmd,"lev",0)==0)
  			{
  				strcpy(cmd2,cmd);
  				tok=strtok (cmd2," ");
					tok = strtok (NULL, " ");//read the 2nd entry in the command
					int nucl=nameToNuclIndex(tok,gd);
					if ((tok != NULL) && (nucl>=0))
						{
  						printf("Printing level data for nucleus %s.\n",tok);
  						showLevelData(nucl,gd);
  					}
  				else
  					printf("Unknown nucleus: %s\n",tok);
  			}
  		else if(strcmp(cmd,"listnuc")==0)
  			{
  				showNuclNames(gd);
  			}
  		else if(strcmp(cmd,"rebuild")==0)
  			{
  				printf("Rebuilding ENSDF database...\n");
					rebuildDatabase(gd,db);
					db=fopen(fileName,"r");//reopen file for reading
					i=fread(gd,sizeof(gdata),1,db);
					if(i==1)
						{
							printf("ENSDF database retrieved from disk.  Data for %i nuclei found.\n",gd->numNucl);
						}
					else
						{
							printf("ERROR: ENSDF database could not be read from file '%s'.  Exiting...\n",fileName);
						}
					fclose(db);
  			}
  		else if(strcmp(cmd,"")==0)
  			{
  				//do nothing
  			}
  		else
  			{
  				printf("Unknown command: %s\nEnter 'help' for a list of commands.\n",cmd);
  			}
		}
	

  return 0; //great success
}
