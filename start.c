#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "reader_api.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

reader_t r1;

typedef struct list_tag1 {
	struct tag1 *h;
	int size;
}list_tag1;

void print_tag1(struct tag1 *h, int i);
void add_tag1(struct list_tag1 *list,struct tag1 tag);
bool cmp_tag1(struct tag1 a, struct tag1 b);
bool cmp_li_tag1(struct list_tag1 *list, struct tag1 c);
void print_li_tag1(struct list_tag1 *list);
void add_unrepeated(struct list_tag1 *list, struct tag1 c);
void print_scan(tag1_t tag_scan1, int32_t val);
struct list_tag1* catch_tags(int n_cycles);
void init();

int32_t main(void){
	int i=1;
	init();
	while(1){
		struct list_tag1 *list = catch_tags(10);
		system("clear");
		printf("ciclo %d:\n",i);
		i++;
		print_li_tag1(list);
		sleep(5);
		free(list);
	}
	return 0;
}

struct list_tag1* catch_tags(int n_cycles){ //Função para captura de CÓDIGOS de Tags, retorna uma lista encadeada de TAGS não repetidas, a quantidade de cilos determinar quantas vezes a leitora ira fazer um pacote de TAGS para aumentar a chance de todas as TAGS serem lidas
	struct list_tag1 *list = malloc(sizeof(struct list_tag1));
	list->h = malloc(1);
	list->size = 0;
	ip_stack_t stats;
	int32_t val=0, i=0, j=0;
	_rcp msg, response;
	uint32_t pause;
	tag1_t tag_scan1;
	tag2_t tag_scan2;
	tag3_t tag_scan3;
	tag4_t tag_scan4;
	tag5_t tag_scan5;
	tag6_t tag_scan6;
	msg.code = START_AUTO_READ2;
	msg.payload_length[0] = 0x00;
	msg.payload_length[1] = 0x05;
	msg.payload[0] = 0x02; // Reserved
	msg.payload[1] = TAGLIST_SIZE; //Max number of tags/packet must be TAGLIST_SIZE
	msg.payload[2] = 0x01; // Maximum elapsed time to tagging (sec)
	msg.payload[3] = 0x00; // How many times reader perform inventory round [MSB]
	msg.payload[4] = 0x14; // How many times reader perform inventory round [LSB]
	
	for(int y=0; y<n_cycles;y++){
		val = handle_rfid_module(&r1, &msg, &response, &tag_scan1, &tag_scan2, &tag_scan3, &tag_scan4, &tag_scan5, &tag_scan6);
		if (val < 0)
		{
			printf("\noperation failed! (%d)\n", val);
			Gen2ReaderDestroy(&r1);
			val = Gen2ReaderCreate("192.168.5.10", &r1);
			if (val)
			{
				printf("Error creating reader session.\n");
				return NULL;
			}
		} else if(response.code == MSG_TYPE_ERROR){
			printf("\nError on RFID module: %02x %02x %02x %02x", response.code, response.payload[0], response.payload[1], response.payload[2]);
		} else if(val==0){
        } else {
			for(i = 0; i < val; i++) {
				if(list->size<1){
					add_tag1(list,tag_scan1.tags1[i]);
				} else {
					add_unrepeated(list,tag_scan1.tags1[i]);
				}
			}
		}
	}
	return list;
}

void init(){ //inicia o socket???(a definir)
	int32_t val = Gen2ReaderCreate("192.168.5.10", &r1);
	if (val)
	{
		printf("Error creating reader session.\n");
		exit(1);
	}
}

void add_unrepeated(struct list_tag1 *list, struct tag1 c){ //Adiciona um Tag à lista se ela não for repetida
	if(cmp_li_tag1(list,c))
		add_tag1(list,c);
}



bool cmp_li_tag1(struct list_tag1 *list, struct tag1 c){ //Compara Lista com tag, retorna true caso não seja identificado a tag dentro da lista
	for(int j = 0; j < list->size; j++){
		if(cmp_tag1(c, *((list->h)+j))){
			return false;
		}
	}
	return true;
}



bool cmp_tag1(struct tag1 a, struct tag1 b){ //Compara duas tags, retorna true se forem iguais
	uint16_t epclen = (a.pc[0] >> 2);
	if((a.pc[0]!=b.pc[0])||(a.pc[1]!=b.pc[1]))
			return false;
	for(int k = 0; k < epclen; k++){
		if(a.epc[k]!=b.epc[k])
			return false;
	}
	return true;
}



void add_tag1(struct list_tag1 *list,struct tag1 tag){ //Adiciona uma Tag na lista
	list->size++;
	list->h = realloc(list->h,sizeof(struct tag1)*list->size);
	*(list->h-1+list->size) = tag;
}



void print_li_tag1(struct list_tag1 *list){ //Printa uma lista de Tags
	printf("Lista de Tags:\n\n");
	for(int i = 0; i<list->size;i++){
		print_tag1(list->h+i,i);
	}
}



void print_tag1(struct tag1 *h, int i){ //Printa uma tag
	printf("\nTag %d:\nPC: %.2x %.2x | EPC: ", i+1, h->pc[0], h->pc[1]);
	uint16_t epclen = (h->pc[0] >> 2);
	for(int j = 0; j < epclen; j++)
		printf("%.2x ", h->epc[j]);
	printf("\n");
}



void print_scan(tag1_t tag_scan1, int32_t val){ //apenas armazenando um código excluido que pode vir a ser útil
	int j, i;
	if(val == 0)
		printf("\nNenhuma TAG Lida.");
	else
	{
		printf("\n%d tags inventoried.", val);
		for(i = 0; i < val; i++)
	{
		uint16_t epclen = (tag_scan1.tags1[i].pc[0] >> 2);
		printf("\nPC: %.2x %.2x | ", tag_scan1.tags1[i].pc[0], tag_scan1.tags1[i].pc[1]);
		printf("EPC: ");
		for(j = 0; j < epclen; j++)
			printf("%.2x ", tag_scan1.tags1[i].epc[j]);
		}
	}
}
