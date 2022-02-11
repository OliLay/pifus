#define _GNU_SOURCE

/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* local includes */
#include "pifus_byte_buffer.h"


int main(int argc, char *argv[]) {
  struct pifus_byte_buffer byte_buffer;

  pifus_byte_buffer_create(&byte_buffer, 8);

  char buffer[8];

  char* to_add_buf = "1234567";
  pifus_byte_buffer_add(&byte_buffer, buffer, to_add_buf, 7);

  char retrv[8];
  uint16_t retrv_len = pifus_byte_buffer_pop(&byte_buffer, buffer, 8, retrv);

  printf("1:");
  fwrite(retrv, sizeof(char), retrv_len, stdout);
  printf("\n");

  to_add_buf = "12345";
  pifus_byte_buffer_add(&byte_buffer, buffer, to_add_buf, 5);
  to_add_buf = "67";
  pifus_byte_buffer_add(&byte_buffer, buffer, to_add_buf, 2);
  
  retrv_len = pifus_byte_buffer_pop(&byte_buffer, buffer, 7, retrv);
  printf("2:");
  fwrite(retrv, sizeof(char), retrv_len, stdout);
  printf("\n");


  to_add_buf = "123";
  pifus_byte_buffer_add(&byte_buffer, buffer, to_add_buf, 3);
  
  retrv_len = pifus_byte_buffer_pop(&byte_buffer, buffer, 3, retrv);
  printf("3:");
  fwrite(retrv, sizeof(char), retrv_len, stdout);
  printf("\n");

  to_add_buf = "1234567";
  pifus_byte_buffer_add(&byte_buffer, buffer, to_add_buf, 7);
  
  retrv_len = pifus_byte_buffer_pop(&byte_buffer, buffer, 7, retrv);
  printf("4:");
  fwrite(retrv, sizeof(char), retrv_len, stdout);
  printf("\n");

  return 0;
}