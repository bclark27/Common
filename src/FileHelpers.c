#include "FileHelpers.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////

unsigned char * fileHelpers_getBytes(char * file, unsigned int * size)
{
  FILE * fp = fopen(file, "r");

  // if cant open file, return null
  if (!fp)
  {
    printf("Can not open file\n");
    return NULL;
  }

  ssize_t sizeRead;
  ssize_t currSize = 0;
  unsigned char * fileBytes = NULL;
  unsigned char * buffer[READ_BUF_SIZE];

  while ((sizeRead = fread(buffer, 1, READ_BUF_SIZE, fp)) > 0)
  {
    // create extra space for new incoming bytes
    unsigned char * newBytesArray = (unsigned char *)malloc(currSize + sizeRead);
    if (fileBytes)
    {
      memcpy(newBytesArray, fileBytes,  currSize);
      free(fileBytes);
    }

    fileBytes = newBytesArray;

    // copy from the location of the data to the array of all data collected so far
    memcpy(fileBytes + currSize, buffer, sizeRead);
    currSize += sizeRead;
  }

  fclose(fp);

  if (size) *size = currSize;
  return fileBytes;
}

char * fileHelpers_getText(char * file, unsigned int * size)
{
  unsigned int size_temp = 0;
  char * text = (char *)fileHelpers_getBytes(file, &size_temp);

  if (text == NULL || size_temp == 0)
  {
    return NULL;
  }

  text = realloc(text, ++size_temp);
  text[size_temp - 1] = 0;

  if (size) *size = size_temp;

  return text;
}

void fileHelpers_writeBytes(char * file, void * bytes, unsigned int size)
{
  FILE * fp = fopen(file, "wb");

  // if cant open file, return null
  if (!fp)
  {
    printf("Can not open file\n");
    return;
  }

  fwrite(bytes, size, 1, fp);

  fclose(fp);
}

void fileHelpers_createDir(char * dir)
{
  #ifdef __linux__
  mkdir(dir, 511);
  #else
  _mkdir(dir);
  #endif
}

void fileHelpers_cd(char * dir)
{
  #ifdef __linux__
  chdir(dir);
  #else
  _chdir(dir);
  #endif
}

char * fileHelpers_getDir(char * file)
{
  int len = strlen(file);

  for (len -= 1; len >= 0; len--)
  {
    if (file[len] == SLASH)
    {
      break;
    }
  }

  char * dir;
  if (len == -1)
  {
    dir = malloc(3);
    dir[0] = '.';
    dir[1] = SLASH;
    dir[2] = 0;
  }
  else
  {
    dir = malloc(len + 1);
    memcpy(dir, file, len);
    dir[len] = 0;
  }

  return dir;
}

char * fileHelpers_getName(char * file)
{
  int pos = strlen(file);
  int dotPos = -1;

  for (pos -= 1; pos >= 0; pos--)
  {
    if (file[pos] == '.')
    {
      dotPos = pos;
    }

    if (file[pos] == SLASH)
    {
      pos++;
      break;
    }
  }

  char * name;
  if (pos == -1)
  {
    pos = 0;
  }

  int len = (strlen(file) - pos) - (strlen(file) - dotPos);
  name = malloc(len + 1);
  memcpy(name, file + pos, len);
  name[len] = 0;

  return name;
}
