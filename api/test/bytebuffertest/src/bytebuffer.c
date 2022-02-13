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

  pifus_byte_buffer_create(&byte_buffer, 9);

  char buffer[9] = {};

  char* to_add_buf = "12345678";
  pifus_byte_buffer_add(&byte_buffer, buffer, to_add_buf, 8);

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


  to_add_buf = "12";
  pifus_byte_buffer_add(&byte_buffer, buffer, to_add_buf, 2);
  
  retrv_len = pifus_byte_buffer_pop(&byte_buffer, buffer, 2, retrv);
  printf("3:");
  fwrite(retrv, sizeof(char), retrv_len, stdout);
  printf("\n");

  to_add_buf = "123456789";
  pifus_byte_buffer_add(&byte_buffer, buffer, to_add_buf, 9);
  
  memset(retrv, 0x0, 8);
  retrv_len = pifus_byte_buffer_pop(&byte_buffer, buffer, 9, retrv);
  printf("4:");
  fwrite(retrv, sizeof(char), retrv_len, stdout);
  printf("\n");

  return 0;
}