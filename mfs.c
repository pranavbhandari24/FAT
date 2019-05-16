/* 
  Pranav Bhandari
*/   




// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <ctype.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments


//Initializing Variables
uint16_t BPB_RsvdSecCnt;
uint16_t BPB_BytsPerSec;
uint16_t BPB_SecPerClus;
uint16_t BPB_FATSz32;
uint16_t BPB_NumFATs;
FILE *fptr = NULL;
int current_add=-1,i,j,flag =0;
uint16_t current_cluster = 0;


//A structure attribute to read each directory easily
struct __attribute__((__packed__)) DirectoryEntry {
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstClusterHigh;
  uint8_t Unused2[4];
  uint16_t DIR_FirstClusterLow;
  uint32_t DIR_FileSize; 
};
struct DirectoryEntry dir[16];

int LBAToOffset(int32_t sector)
{
  return ((sector-2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

int16_t NextLB(uint32_t sector)
{
  uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector*4);
  int16_t val;
  fseek(fptr, FATAddress, SEEK_SET);
  fread(&val, 2, 1, fptr);
  return val;
}


void set_values()
{
  fseek(fptr, 11, SEEK_SET);
  fread(&BPB_BytsPerSec,1,2,fptr);

  fseek(fptr,14,SEEK_SET);
  fread(&BPB_RsvdSecCnt,2,1,fptr);

  fseek(fptr,13,SEEK_SET);
  fread(&BPB_SecPerClus,1,1,fptr);

  fseek(fptr,36,SEEK_SET);
  fread(&BPB_FATSz32,2,2,fptr);

  fseek(fptr,16,SEEK_SET);
  fread(&BPB_NumFATs,1,1,fptr);
}


int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    /*int token_index  = 0;
    for( token_index = 0; token_index < token_count; token_index ++ ) 
    {
      printf("token[%d] = %s\n", token_index, token[token_index] );  
    }*/

    //MY code
    
    if(strcmp(token[0],"open") == 0){
      if (fptr != NULL)
      {
        printf("Error: A File system is image already open.\n");
        continue;
      }
      fptr = fopen(token[1], "r+");
      if(fptr == NULL)
      {
        printf("Error: File system image not found. \n");
      }
      else
      {
        set_values();
        current_add = LBAToOffset(2);
      }  
    }

    else if(strcmp(token[0],"close") == 0){
      if(fptr == NULL)
      {
        printf("Error: File System Image must be opened First.\n");
        continue;
      }
      fclose(fptr);
      fptr = NULL;      
    }

    else if(strcmp(token[0],"info") == 0){
      if(fptr == NULL)
      {
        printf("Error: File System Image must be opened First.\n");
        continue;
      }
      printf("BPB_BytsPerSec : %d \nBPB_BytsPerSec : %x\n\n",BPB_BytsPerSec,BPB_BytsPerSec);
      printf("BPB_SecPerClus : %d \nBPB_SecPerClus : %x\n\n",BPB_SecPerClus,BPB_SecPerClus);
      printf("BPB_RsvdSecCnt : %d \nBPB_RsvdSecCnt : %x\n\n",BPB_RsvdSecCnt,BPB_RsvdSecCnt);
      printf("BPB_NumFATs : %d \nBPB_NumFATs : %x\n\n",BPB_NumFATs,BPB_NumFATs);
      printf("BPB_FATSz32 : %d \nBPB_FATSz32 : %x\n\n",BPB_FATSz32,BPB_FATSz32);
    }

    else if(strcmp(token[0],"ls") == 0){
      if(fptr == NULL)
      {
        printf("Error: File System Image must be opened First.\n");
        continue;
      }
      fseek(fptr,current_add,SEEK_SET);
      fread(&dir,16,32,fptr);
      for(i =0;i<16;i++)
      {
        if(dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
        { 
          if(!(dir[i].DIR_Name[0] == (char)0xE5 || dir[i].DIR_Name[0] == (char)0x00 || dir[i].DIR_Name[0] == (char)0x05)) 
          {
            for(j=0;j<11;j++)
              printf("%c",dir[i].DIR_Name[j]);
            printf("\n");
          }
        }  
      }
    }

    else if(strcmp(token[0],"cd") == 0){
        if(fptr == NULL)
        {
          printf("Error: File System Image must be opened First.\n");
          continue;
        }
        flag =0;
        fseek(fptr,current_add,SEEK_SET);
        fread(&dir,16,32,fptr);

        char sub_dir[11];
        for(i=0;i<=11;i++)
        {
          if(i<strlen(token[1]))
            sub_dir[i] = token[1][i];
          else if(i == 11)
            sub_dir[i] = (char)16;
          else 
            sub_dir[i] = ' ';
        }
        for(i =0;i<16;i++)
        {
          if(dir[i].DIR_Attr == 0x10 && (strcmp(sub_dir,dir[i].DIR_Name)==0))
          {
              if(dir[i].DIR_FirstClusterLow == 0)
                current_add = LBAToOffset(2);
              else
                current_add = LBAToOffset(dir[i].DIR_FirstClusterLow);
              flag =1;
              break;
          }
        }
        if(flag == 0)
            printf("No such directory\n");
    }

    else if(strcmp(token[0],"read")==0){
      if(fptr == NULL)
      {
        printf("Error: File System Image must be opened First.\n");
        continue;
      }
      flag = 0;
      fseek(fptr,current_add,SEEK_SET);
      fread(&dir,16,32,fptr);
      char filename[12];
      memset( filename, ' ', 12 );
      char *tok = strtok( token[2], "." );
      strncpy( filename, tok, strlen( tok ) );
      tok = strtok( NULL, "." );
      if( tok )
      {
        strncpy( (char*)(filename+8), tok, strlen(tok ) );
      }
      filename[11] = '\0';
      int i;
      for( i = 0; i < 11; i++ )
        filename[i] = toupper( filename[i] );

      for(i=0;i<16;i++)
      {
        if( strncmp( filename, dir[i].DIR_Name, 11 ) == 0 )
        {
          flag =1;
          int cluster = dir[i].DIR_FirstClusterLow;
          int offset = atoi(token[3]);
          while(offset>512)
          {
            cluster = NextLB(cluster);
            offset = offset - 512;
          }
          int addr = LBAToOffset(cluster);
          fseek(fptr,addr,SEEK_SET);
          fseek(fptr,offset,SEEK_CUR);
          int f_size = atoi(token[4])+1;
          if(atoi(token[4])>dir[i].DIR_FileSize-1)
          {
            f_size = dir[i].DIR_FileSize;
          }
          char bytes[f_size];
          fread(bytes,f_size,1,fptr);
          bytes[f_size] = '\0';
          if(strcmp(token[1],"hex")==0)
          {
            printf("The data read (in hexadecimal) is : ");
            for(i=0;i<f_size;i++)
              printf("%x ",bytes[i]);
          }
          else if(strcmp(token[1],"ASCII")==0)
          {
            printf("The data read (in ASCII) is : ");
            for(i=0;i<f_size;i++)
              printf("%c",bytes[i]);
          }
          else
          {
            printf("No such command.\n");
          }
          printf("\n");
        }
      }
      if(flag == 0)
        printf("No such directory\n");
    }

    else if(strcmp(token[0],"stat") == 0){
      if(fptr == NULL)
      {
        printf("Error: File System Image must be opened First.\n");
        continue;
      }
      flag =0;
      fseek(fptr,current_add,SEEK_SET);
      fread(&dir,16,32,fptr);
      char filename[12];
      memset( filename, ' ', 12 );
      char *tok = strtok( token[1], "." );
      strncpy( filename, tok, strlen( tok ) );
      tok = strtok( NULL, "." );
      if( tok )
      {
        strncpy( (char*)(filename+8), tok, strlen(tok ) );
      }
      filename[11] = '\0';
      int i;
      for( i = 0; i < 11; i++ )
        filename[i] = toupper( filename[i] );

      for(i=0;i<16;i++)
      {
        if( strncmp( filename, dir[i].DIR_Name, 11 ) == 0 )
        {
          flag=1;
          printf("Attributes              : %d\n",dir[i].DIR_Attr);
          printf("Starting Cluster Number : %d\n",dir[i].DIR_FirstClusterLow);
          if(dir[i].DIR_Attr == 0x10)
            printf("Size                    : 0\n");
          else 
            printf("Size                    : %d\n",dir[i].DIR_FileSize);
        }
      }
      if(flag == 0)
        printf("Error: File not found.\n");

    }
    else if(strcmp(token[0],"put") == 0){
      if(fptr == NULL)
      {
        printf("Error: File System Image must be opened First.\n");
        continue;
      }
      char temp[12];
      strcpy(temp,token[2]);
      FILE *fp = fopen(temp, "r");
      if(fp == NULL ) 
      {
        printf("Error: not able to open file\n");
        continue;
      }
      flag =0;
      fseek(fptr,current_add,SEEK_SET);
      fread(&dir,16,32,fptr);
      
      char getname[12];
      memset( getname, ' ', 12 );
      char *tok = strtok( token[2], "." );
      strncpy( getname, tok, strlen( tok ) );
      tok = strtok( NULL, "." );
      if( tok )
      {
        strncpy( (char*)(getname+8), tok, strlen(tok ) );
      }
      getname[11] = '\0';
      int i;
      for( i = 0; i < 11; i++ )
        getname[i] = toupper( getname[i] );
      for(i=0;i<16;i++)
      {
        if(dir[i].DIR_Name[0] == (char)0xE5 || dir[i].DIR_Name[0] == (char)0x00 || dir[i].DIR_Name[0] == (char)0x05)
        {
          flag = 1;
          strcpy(dir[i].DIR_Name,getname);
          dir[i].DIR_Attr = 0x01;
          int cluster = dir[i].DIR_FirstClusterLow;
          char buffer[512];
          fseek( fp , 0 , SEEK_END);
          int lSize = ftell(fp);
          dir[i].DIR_FileSize = lSize;
          fseek(fp, 0 ,SEEK_SET);
          fseek(fptr,current_add,SEEK_SET);
          fwrite(&dir,16,32,fptr);
          int addr = LBAToOffset(cluster);
          fread( buffer , 512, 1 , fp);
          fseek(fptr, addr, SEEK_SET);
          //fprintf(fptr,"%s",buffer);
          if(strcmp(token[1],"hex")==0)
          {
            for(int j=0;j<512;j++)
              fprintf(fptr,"%x",buffer[j]);
          }
          else if(strcmp(token[1],"ASCII")==0)
          {
            for(int j=0;j<512;j++)
              fprintf(fptr,"%c",buffer[j]);
          }
          lSize = lSize - 512;
          fclose(fp);
          break;
        }
      }
      if(flag == 0)
        printf("Not enough space\n");
    }

    else if(strcmp(token[0],"get") == 0){
      if(fptr == NULL)
      {
        printf("Error: File System Image must be opened First.\n");
        continue;
      }
      flag =0;
      fseek(fptr,current_add,SEEK_SET);
      fread(&dir,16,32,fptr);
      char temp[12];
      strcpy(temp,token[2]);
      char getname[12];
      memset( getname, ' ', 12 );
      char *tok = strtok( token[2], "." );
      strncpy( getname, tok, strlen( tok ) );
      tok = strtok( NULL, "." );
      if( tok )
      {
        strncpy( (char*)(getname+8), tok, strlen(tok ) );
      }
      getname[11] = '\0';
      int i;
      for( i = 0; i < 11; i++ )
        getname[i] = toupper( getname[i] );
      for(i=0;i<16;i++)
      {
        if((strncmp( getname, dir[i].DIR_Name, 11 ) == 0) && (dir[i].DIR_Attr != 0x10))
        {
          flag =1;
          FILE *fp = fopen(temp, "w");
          char data[512];
          int cluster = dir[i].DIR_FirstClusterLow;
          int size = dir[i].DIR_FileSize;
          int file_add = LBAToOffset(cluster);
          fseek(fptr,file_add,SEEK_SET);
          fseek(fp,0,SEEK_SET);
          fread( data , 512, 1 , fptr);
          if(strcmp(token[1],"hex")==0)
          {
            for(i=0;i<512;i++)
              fprintf(fp,"%x",data[i]);
          }
          else if(strcmp(token[1],"ASCII")==0)
          {
            for(i=0;i<512;i++)
              fprintf(fp,"%c",data[i]);
          }
          size = size - 512;
          while(size>0)
          {
            cluster = NextLB(cluster);
            file_add = LBAToOffset(cluster);
            fseek(fptr,file_add,SEEK_SET);
            fread(&data[0],512,1,fptr);
            if(strcmp(token[1],"hex")==0)
            {
              for(i=0;i<512;i++)
                fprintf(fp,"%x",data[i]);
            }
            else if(strcmp(token[1],"ASCII")==0)
            {
              for(i=0;i<512;i++)
                fprintf(fp,"%c",data[i]);
            }
            size = size - 512;
          }
          fclose(fp);
          break;
        }
      }  
      if(flag == 0)
        printf("No such directory\n");
    }
    else if(strcmp(token[0],"exit") == 0 || strcmp(token[0],"quit") == 0)
      exit(0);
    else
    {
      printf("Command not found. Try again.\n");
    }
    
    free( working_root );

  }
  return 0;
}
