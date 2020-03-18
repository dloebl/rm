/***
* rm
* Written in 2020
* To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
* You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
***/

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE   700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define INVALID_SYNTAX_MSG "Invalid syntax.\n\nSyntax is:\nrm [-iRr] file...\nrm -f [-iRr] [file...]\n"
#define IS_DIR(mode)        ((mode & S_IFMT) == S_IFDIR)
//
// Static variables
//
static int IsRecursive;
static int IsInteractive;
static int IsForced;
//
// Forward declarations
//
static int  GetConfirmation (void);
static void ParseOption     (const char* sOption);
static int  DeleteEntry     (const char* sFile);
static int  ParseArgs       (int NumArgs, const char** pArgs);
static int  DeleteDir       (const char* sDir);
static int  DeleteEntry     (const char* sEntry);

/***
* Description: Checks whether the user has confirmed the current operation (with 'y' or 'Y').
*
* Returns:     1 on success or 0 on error
***/
static int GetConfirmation(void) {
  char c;
  int  r;

  c = getchar();
  if (c == 'y' || c == 'Y') { // Test whether the user has confirmed with 'y' or 'Y'
    r = 1;  
  } else {
    r = 0;
  }
  //
  // Go to the end of the line (will clear the input-buffer)
  //
  while(c != EOF && c != '\n') {
    c = getchar();
  }
  return r;
}

/***
* Description: Parses the given option string and saves the set options into the corresponding static's
*
* Parameters:
*   sOption:   Option string to be parsed
***/
static void ParseOption(const char* sOption) {
  while(*sOption) {
    switch(*sOption++) {
    case 'f':
      IsForced      = 1;
      IsInteractive = 0;
      break;
    case 'i':
      IsInteractive = 1;
      IsForced      = 0;
      break;
    case 'r': // Falltrough
    case 'R':
      IsRecursive   = 1;
    }
  }
}

/***
* Description: Parses the arguments given. Calls ParseOption if an option block is encountered.
*              Stops on first file argument found (from left to right).
* Parameters:
*   NumArgs:   Number of arguments
*   pArgs:     Array of arguments
* 
* Returns:     Index of first file argument to be deleted or 0 on error
***/
static int ParseArgs(int NumArgs, const char** pArgs) {
  int i;
  //
  // Process every option provided
  //
  for (i = 1; i < NumArgs; ++i) {
    if (pArgs[i][0] == '-') {
      ParseOption(pArgs[i]);
    } else {
      return i;
    }
  }
  return 0;
}

/***
* Description: Deletes a whole directory. Called by DeleteEntry.
*
* Parameters:
*   sDir:      Path to the directory to be deleted
* 
* Returns:     0 on success or -1 on error
***/
static int DeleteDir(const char* sDir) {
  size_t         LenDirPath;
  size_t         LenEntry;
  int            r;
  DIR*           pDir;
  struct dirent* pDirEntry;
  char*          pPath;

  r    = -1;
  pDir = opendir(sDir);
  if (pDir != NULL) {
    LenDirPath            = strlen(sDir);
    pPath                 = malloc(LenDirPath + 1); // One additional byte is needed ('/')
    if (pPath == NULL) {
      return -1;    
    }
    pPath[LenDirPath]     = '/';                    // Add directory delimiter at the end
    memcpy(pPath, sDir, LenDirPath);                // Copy rest of the name of the directory (Note: String is not nul-terminated yet) */
    //
    // Delete every directory-entry in the current directory (sDir).
    //
    while((pDirEntry = readdir(pDir))) {
      //
      // Skip both the directory entry "." and ".."
      //
      if (strcmp(pDirEntry->d_name, ".") != 0 && strcmp(pDirEntry->d_name, "..") != 0) {     
        //
        // Build the actual path of the directory entry to be deleted
        //
        LenEntry = strlen(pDirEntry->d_name);
        pPath    = realloc(pPath, LenDirPath + 1 + LenEntry + 1); // Two addiontal bytes needed: For '/' and the terminating 0
        if (pPath == NULL) {
          return -1;      
        }
        memcpy(pPath + LenDirPath + 1, pDirEntry->d_name, LenEntry + 1);
        r = DeleteEntry(pPath);
      }
    }
    r = rmdir(sDir);                                             // Delete the directory itself at last
    free(pPath);
    closedir(pDir);
  }
  return r;
}

/***
* Description: Deletes a directory entry. Might call DeleteDir.
*
* Parameters:
*   sDir:      Path to directory entry to be deleted
* 
* Returns:     0 on success or -1 on error
***/
static int DeleteEntry(const char* sEntry) {
  int         r;
  struct stat statbuf;
  //
  // In the interactive-mode, the user has to confirm whether to delete the current directory entry.
  //
  if (IsInteractive) {
    fprintf(stderr, "rm: Do you really want to delete '%s' (y/N)? ", sEntry);
    r = GetConfirmation();
    if (r == 0) {
      return 0;
    }
  }
  //
  // Distinguish the current directory entry between a directory (will be deleted recursively) or a regular file, symbolic link etc.
  //
  r = lstat(sEntry, &statbuf);                   // Get stat of the current target (Note: Symbolic links will not be followed)
  if (r == 0) {
    if (IS_DIR(statbuf.st_mode)) {
      r = IsRecursive ? DeleteDir(sEntry) : -1;  // Only delete a directory when the recursion option (-r or -R) is set.
    } else {
      r = unlink(sEntry);
    }
  }
  return r;
}

int main(int NumArgs, const char** pArgs) {
  int i;
  int r;
  int Return;

  Return = 0;
  i      = ParseArgs(NumArgs, pArgs); // Parse all arguments provided. This function returns the index of the first entry to be deleted.
  if (i) {
    //
    // Delete every entry specified
    //
    for (; i < NumArgs; ++i) {
      r = DeleteEntry(pArgs[i]);
      if (r == -1 && IsForced == 0) {
        fprintf(stderr, "rm: Failed to delete '%s'.\n", pArgs[i]);
        Return = 1;
      }
    }    
  } else if (IsForced == 0) {   // "rm -f" is actually allowed, catch this case here.
    fputs(INVALID_SYNTAX_MSG, stderr);
    Return = 1;
  }
  return Return;
}
