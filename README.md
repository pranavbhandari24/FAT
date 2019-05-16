# FAT
This program implements a user space shell application that is capable of interpreting and performing commands on a FAT32 file system image.

# Commands Supported:
The shell supports the following commands:
  1) open <filename> 
  
            This command opens the fat32 image file.
  2) close
  
            This command closes the fat32 image file.
  3) info
  
            This command prints out the following information about the file system in both hexadecimal and base 10:
               • BPB_BytesPerSec
               • BPB_SecPerClus
               • BPB_RsvdSecCnt
               • BPB_NumFATS
               • BPB_FATSz32 
  4) stat <filename>
  
             This command prints the attributes and starting cluster number of the file or directory name.
  5) ls
  
             Lists the directory contents. 
  6) cd <directory>
  
             This command changes the current working directory to the given directory. 
  7) get hex <filename>
  
             This command retrieves the file from the FAT 32 image and places it in your current working
             directory (in hex format).
  8) get ASCII <filename>
  
             This command retrieves the file from the FAT 32 image and places it in your current working
             directory (in ASCII format).
  9) put hex <filename>
  
             This command retrieves the file from the current working directory and places it in your FAT
             32 image (in hex format).
  10) put ASCII <filename>
  
             This command retrieves the file from the current working directory and places it in your FAT
             32 image (in ASCII format).
  11) read hex <filename> <position> <number of bytes>
  
             Reads from the given file at the position, in bytes, specified by the position parameter and output
             the number of bytes specified (in hex format).  
  12) read ASCII <filename> <position> <number of bytes>
  
             Reads from the given file at the position, in bytes, specified by the position parameter and output
             the number of bytes specified (in ASCII format). 
             
            
# How to run:
  To run the program type the following commands:
  
    make
    ./mfs

# Note: This code cannot be run without a FAT32.img file.
