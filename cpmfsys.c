#include "cpmfsys.h"
#include "diskSimulator.h"
#include<string.h>

bool free_list[256];

/*makes a directory structure containing the file elements from the main memory.*/
DirStructType *mkDirStruct(int index,uint8_t *e){
	DirStructType *d;
	d = malloc(sizeof(DirStructType));
	int j;
	int i=0;
		(d -> status) = (e+index*EXTENT_SIZE)[0];
		i++;
		j=0;
		while(i<9){
			if ((e+index*EXTENT_SIZE)[i] != ' '){	
				(d -> name)[j] = (e+index*EXTENT_SIZE)[i];
			}
			else {
				(d -> name)[j] = '\0';
				break;
			}
			j++;
			i++;
		}
		j=0; 
		i=9;
		while(i<12){
			if ((e+index*EXTENT_SIZE)[i] != ' '){	
				d -> extension[j] = (e+index*EXTENT_SIZE)[i];
			}
			else {
				d -> extension[j] = '\0';
				break;
			}
			j++;
			i++;
		}
		d -> XL = (e+index*EXTENT_SIZE)[12];
		d -> BC = (e+index*EXTENT_SIZE)[13];
		d -> XH = (e+index*EXTENT_SIZE)[14];
		d -> RC = (e+index*EXTENT_SIZE)[15];
		i=16;
		j=0;
		while(i<32){
			d -> blocks[j] = (e+index*EXTENT_SIZE)[i];
			i++;
			j++;
		}
		return d;	
}

/*writes the directory structure into the main memory*/
void writeDirStruct(DirStructType *d,uint8_t index, uint8_t *e){	
	int i=0;
	int j;
	(e+index*EXTENT_SIZE)[0] = d -> status;
	i++;
	j=0;
	while(i<9){
		if (d -> name[j] != '\0'){	
			(e+index*EXTENT_SIZE)[i] = d -> name[j];
		}
		else {
			(e+index*EXTENT_SIZE)[i] = ' ';
		}
	    j++;
		i++;
	}
	j=0;
	while(i<12){
		if (d -> extension[j] != '\0'){	
			(e+index*EXTENT_SIZE)[i] = d -> extension[j];
		}
		else {
			(e+index*EXTENT_SIZE)[i] = ' ';
		}
	    j++;
		i++;
	}
	(e+index*EXTENT_SIZE)[12] = d -> XL;
	(e+index*EXTENT_SIZE)[13] = d -> BC;
	(e+index*EXTENT_SIZE)[14] = d -> XH;
	(e+index*EXTENT_SIZE)[15] = d -> RC;
	i=16;
	j=0;
	while(i<32){
		(e+index*EXTENT_SIZE)[i] = d -> blocks[j];
		i++;
		j++;
	}	
}

/*gives the directory structure with the file length for each file*/
void cpmDir(){
	int index,b_index;
	int block_number;
	int file_length = 0;
	uint8_t buffer[1024];
	DirStructType *cpm_dir;	
	blockRead(buffer,(uint8_t)0);
	printf("DIRECTORY LISTING\n");
	for(index=0;index<32;index++){
		cpm_dir = mkDirStruct(index,buffer);
		if(cpm_dir -> status != 0xe5)
		{
			block_number = 0;
			for(b_index = 0;b_index<16;b_index++){
				if (cpm_dir -> blocks[b_index]!=0)
					block_number++;
			}
			file_length = (block_number-1)*1024+(cpm_dir -> RC)*128+(cpm_dir -> BC);
			fprintf(stdout,"%s.%s %d\n",cpm_dir->name,cpm_dir->extension,file_length);
		}
	}
}

// 0 -free

/*global free list array that contains a list of used and unused blocks*/
void makeFreeList(){
	uint8_t buffer[1024];
	int index,b_index;
	DirStructType *cpm_dir;	
	blockRead(buffer,(uint8_t)0);
	for(index;index<256;index++){
		free_list[index] = true;
	}
	for(index=0;index<32;index++){
		cpm_dir = mkDirStruct(index,buffer);
		for(b_index = 0;b_index<16;b_index++){
			if (cpm_dir -> blocks[b_index] == 0)
				free_list[(int)cpm_dir -> blocks[b_index]] = true;
			else
				free_list[(int)cpm_dir -> blocks[b_index]] = false;	
		}
	}
	free_list[0] = false;
	
}

//free dot
/*prints the freelist */
void printFreeList(){
	int index;
	char addr[16][3] = {"0","10","20","30","40","50","60","70","80",
	"90","a0","b0","c0","d0","e0","f0"};
	int j=0;
	int i=0;	
	printf("FREE BLOCK LIST: (* means in-use)\n");
	fprintf(stdout,"%s ",addr[0]);	
	i++;
	free_list[0]=false;
	while(j>=0 && j<256){
		if (j%16 == 0 && j!=0){
			fprintf(stdout,"\n");
			fprintf(stdout,"%s ",addr[i]);
			i++;
		}
		if (free_list[j]){
			fprintf(stdout,". ");
		}
		else{
			fprintf(stdout,"* ");
		}
		j++;
		
	}
	  fprintf(stdout,"\n");
}

/*checks if the file name is valid or not*/
bool checkLegalName(char *name){
	int i=0;
	int len=0;
	int ext_bit;
	len=strlen(name);
	if (name[0]!= ' ' || name[0] != '.'|| name[0] != '\0'){		
		while (name[i]!= '.' && i<8 && i<len){
			if ((name[i]>=65 && name[i]<=90) || (name[i] >=97 && name[i] <= 122) || (name[i] >=48 && name[i] <= 57) && i<8){
				i++;			
			}
			else {
				
				return false;
			}
		}
		if (name[i]=='.' && i<=8)
		{
			ext_bit = 0;
			i++;
			while(ext_bit<3 && i<12 && i<len)
			{				
				if ((name[i]>=65 && name[i]<=90) || (name[i] >=97 && name[i] <= 122) || (name[i] >=48 && name[i] <= 57) && (ext_bit<3)){
					
					i++;
					ext_bit++;				    
				}
				else {
					
					return false;
				}
			}
			
			return true;
		}
		else {
			
			return false;
		}
	
	return true;
	}
	else{
		
		return false;
	}
}

/*returns the index value of the file for which the data is to be accessed.*/
int findExtentWithName(char *name, uint8_t *block0){
	bool result;
	int index;
	int i=0;
	int j=0;
	char split_name[9];
	char split_ext[4];
	DirStructType *cpm_dir;
	char sp[18];
	int len=0;
	len=strlen(name);
	result = checkLegalName(name);
	if (result == true){
		while(i>=0 && i<len){
			if (name[i]!='.'){
				split_name[j]=name[i];
				j++;
				i++;			
			}
			else if (name[i] == '.'){
				i++;
				split_name[j]='\0';
				j=0;
				while(i>0 && i<len){
					if (name[i]!=' '){
						split_ext[j]=name[i];
						j++;
						i++;
					}
					else if (name[i]==' '){
						split_ext[j]='\0';
						break;
					}
					
				}
				split_ext[j]='\0';
			}
		}
		for(index=0;index<32;index++){
			cpm_dir = mkDirStruct(index,block0);
				if (((strcmp(split_name,cpm_dir->name))==0) && ((strcmp(split_ext,cpm_dir->extension))==0)){
					if(cpm_dir -> status != 0xe5){
						return index;
					}
					else
						return -1;
				}			
			}
	}	
		else 
			return false;
	}

/*	delete a filename and its allocated blocks with the filename passed as a parameter*/
int  cpmDelete(char * name){
	uint8_t buffer[1024];
	DirStructType *cpm_dir;
	int result=0;
	int index;
	blockRead(buffer,(uint8_t)0);
	result= findExtentWithName(name,buffer);
	if (result!=-1){		
	    cpm_dir = mkDirStruct(result,buffer);
		cpm_dir -> status = 0xe5;
		index=0;
		while(index<16){		
			free_list[(int)cpm_dir -> blocks[index]] = true;	
			cpm_dir -> blocks[index] = 0;			
			index ++;
		}
	writeDirStruct(cpm_dir,result,buffer);
	blockWrite(buffer,(uint8_t)0); 
	return 0;
	
	}
	else{
		return -1;
	}
}

/*rename a filename with the old filename and new filename passed as parameters*/
int cpmRename(char *oldName, char * newName){
	uint8_t buffer[1024];
	int i=0;
	int len=0;
	int j=0;
	int result=0;
	bool check;
	len = strlen(newName);
	DirStructType *cpm_dir;
	blockRead(buffer,(uint8_t)0);
	check = checkLegalName(newName);
	if (check == false){
		return -2;
	}
	result= findExtentWithName(oldName,buffer);
	if (result!=-1){
		cpm_dir = mkDirStruct(result,buffer);
		while(i>=0 && i<len){
			if (newName[i]!='.'){
				cpm_dir -> name[j] = newName[i];				
				i++;
				j++;
			}
			else if (newName[i] == '.'){
				i++;
				cpm_dir -> name[j]='\0';
				j=0;
				while(i>0 && i<len){
					if (newName[i]!=' '){
						cpm_dir -> extension[j] = newName[i];
						j++;
						i++;
					}
					else if (newName[i]==' '){
						cpm_dir -> extension[j]='\0';
						break;
					}
					
				}
				cpm_dir -> extension[j]='\0';
			}
		}
	writeDirStruct(cpm_dir,result,buffer);	
	blockWrite(buffer,(uint8_t)0); 
	return 0;
	}
	else 
		return -1;
}