#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "reader_api.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>

FILE *log;
struct timeval  tv1, tv2, tv3, tv4, tv5, tv6, tv7, tv8;
bool comprando = true, debug = true;
int count=1, n_cycles = 2, arg, pid;
int8_t n_antenas = 1;
uint8_t a = 0;

typedef struct reader_s {
	tag1_t tag_scan1;
	tag2_t tag_scan2;
	tag3_t tag_scan3;
	tag4_t tag_scan4;
	tag5_t tag_scan5;
	tag6_t tag_scan6;
	reader_t r1;
	readerconfig_t r2;
	_rcp msg, response;
	
}reader_s;

struct reader_s r;

typedef struct node {
	struct node *next;
	struct tag1 tag;
} node;

typedef struct list_e_tag1{
	struct node *head;
	int size;
} list_e_tag1;

void print_tag1(struct tag1 *h);
bool cmp_tag1(struct tag1 a, struct tag1 b);
void catch_tags(struct list_e_tag1 *list, int n_cycles);
void init_reader();
void init_reader_config();
void add_e_tag1(struct list_e_tag1 *list, struct tag1 tag);
void print_e_list(struct list_e_tag1 *list);
bool cmp_li_e_tag1(struct list_e_tag1 *list, struct tag1 c);
void add_li_e_unrepeated(struct list_e_tag1 *list, struct tag1 c);
void del_e_li(struct list_e_tag1 *list);
void open_log();
int32_t handle_rfid_module_read2();
struct list_e_tag1* catch_tags_debug(struct list_e_tag1 *list, int n_cycles);
void set_antenna(uint8_t arg);
void init_compra();
void antena_8(struct list_e_tag1 *);

void finish(){
	comprando=false;
}

void erro_compra(){
	int32_t val = Gen2ReaderGPIOWrite(&r.r1, (uint8_t)1, (uint8_t)2);
	sleep(2);
	val = Gen2ReaderGPIOWrite(&r.r1, (uint8_t)1, (uint8_t)0);
}

//-------------------------------------------------------------------------------------- INÍCIO MAIN --------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------- INÍCIO MAIN --------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------- INÍCIO MAIN --------------------------------------------------------------------------------------//

int main(int argc, char **argv){
	sleep(2);
	int32_t val = Gen2ReaderGPIOWrite(&r.r1, (uint8_t)1, (uint8_t)0);
	arg=argc;
	if(arg==2)
		pid = atoi(argv[1]);
	init_reader_config();
	init_reader();
	struct list_e_tag1 *list = malloc(sizeof(struct list_e_tag1));
	list->size=0;
	list->head=NULL;
	//signal(SIGUSR1, sleeep);
	signal(SIGALRM, finish);
	signal(SIGUSR1, erro_compra);
	while(1){
		del_e_li(list);
		sleep(1);
		for(a=0;a<n_antenas;a++){
			set_antenna(a+1);
			catch_tags(list, n_cycles);
		}
		usleep(100);
		if(list->size>0){
			printf("Começando Compra!(N tags: %d)\n",list->size);
			init_compra(list);
			comprando=true;
			printf("\n\nCompra Finalizada!\n");
			while(list->size>0){
				printf("Checando se a compra já saiu o portal!\n");
				del_e_li(list);
				for(a=0;a<n_antenas;a++){
					set_antenna(a+1);
					catch_tags(list, n_cycles);
				}
			}
		} else{
			printf("Nada detectado, dormindo 1 segundo para tentar de novo!\n");
		}
		del_e_li(list);
		antena_8(list);
	}
}

//-------------------------------------------------------------------------------------- FIM MAIN --------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------- FIM MAIN --------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------- FIM MAIN --------------------------------------------------------------------------------------//

void antena_8(struct list_e_tag1 *list){
	set_antenna(8);
	catch_tags(list,1);
	int temp = list->size;
	if((list->size>0)&&(list->size<2)){
		printf("Antena 8 leu uma TAG! Enviando sinal para GUI!\n");
		FILE *fp = fopen("Tag.txt","w+");
		uint16_t epclen = (list->head->tag.pc[0] >> 2);
		fprintf(fp,"%.2x\n%.2x\n",list->head->tag.pc[0], list->head->tag.pc[1]);
		for(int j = 0; j < epclen; j++)
			fprintf(fp,"%.2x\n", list->head->tag.epc[j]);
		if(arg==2)
			kill(pid, SIGINT);
		fclose(fp);
		int32_t val = Gen2ReaderGPIOWrite(&r.r1, (uint8_t)1, (uint8_t)8);
		usleep(500000);
		val = Gen2ReaderGPIOWrite(&r.r1, (uint8_t)1, (uint8_t)0);
		
	} else if(list->size>1){
		printf("Mais de uma Tag lida pela antena 8!\n");
	}
	del_e_li(list);
	set_antenna(1);
}

void init_compra(struct list_e_tag1 *list){
	int32_t val = Gen2ReaderGPIOWrite(&r.r1, (uint8_t)1, (uint8_t)5);
	printf("Carrinho passando! Captando tags.\n");
	int n = list->size, nn=0;
	alarm(2);
	gettimeofday(&tv7, NULL);
	while(comprando==true){
		catch_tags(list, n_cycles);
		nn=list->size;
		if(nn>n){
			printf("Detectado %d novas compras! Nº Total Tags: %d! finalização da compra adiada para daqui 2 segundos!\n", nn-n, list->size);
			n=nn;
			alarm(2);
			gettimeofday(&tv7, NULL);
		} else {
			gettimeofday(&tv8, NULL);
			printf("Nenhuma nova Tag Detectada! Nº Total Tags: %d! Compra finalizando em %f s!\n", list->size, 2-((double)(tv8.tv_usec - tv7.tv_usec)/1000000 + (double)(tv8.tv_sec - tv7.tv_sec)));
		}
	}
	char *tags[list->size+4];
	struct node *aux = list->head;
	tags[0] = "python3";
	tags[1] = "send.py";
	tags[list->size+2] = malloc(100);
	sprintf(tags[list->size+2],"%d",getpid());
	tags[list->size+3] = NULL;
	for(int tem = 2; tem<(list->size+2); tem++){
		tags[tem]=malloc(100);
		struct tag1 tempt = aux->tag;
		sprintf(tags[tem],"%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x",tempt.pc[0],tempt.pc[1],tempt.epc[0],tempt.epc[1],tempt.epc[2],tempt.epc[3],tempt.epc[4],tempt.epc[5],tempt.epc[6],tempt.epc[7],tempt.epc[8],tempt.epc[9],tempt.epc[10],tempt.epc[11]);
		printf("%s\n",tags[tem]);
		if(aux!=NULL)
			aux=aux->next;
	}
	pid_t pid = fork();
	if(pid==0){
		printf("return:%d\n",execvp(tags[0],tags));
		exit(1);
	}
	
	for(int tem = 2; tem<(list->size+3); tem++)
		free(tags[tem]);
	val = Gen2ReaderGPIOWrite(&r.r1, (uint8_t)1, (uint8_t)0);
}


void set_antenna(uint8_t arg){
	if((arg>0)||(arg<9)){
		int32_t val = SetAntenna(&r.r2, arg);
		if (val < 0){
			printf("\noperation failed! (%d)\n", val);
			Gen2ReaderDestroy(&r.r2);
			init_reader_config();
		}
	} else{
		printf("\nargumento errado para o set_antenna\n");
	}
}

void open_log(){
	log = fopen("log.txt","r+");
	if(log==NULL){
		fprintf(stderr, "Erro ao tentar abrir a.txt.");
		debug = false;
	}
	fseek(log, 0, SEEK_END);
}


void catch_tags(struct list_e_tag1 *list, int n_cycles){ //Função para captura de CÓDIGOS de Tags, retorna uma lista encadeada de TAGS não repetidas, a quantidade de cilos determinar quantas vezes a leitora ira fazer um pacote de TAGS para aumentar a chance de todas as TAGS serem lidas
	int te = 1;
	int32_t val=0;
	for(int y=0; y<n_cycles;y++){
		val = handle_rfid_module_read2();
		if(val>0){
			te=1;
			for(int i = 0; i < val; i++) {
				if(list->size<1){
					add_e_tag1(list,r.tag_scan1.tags1[i]);
				} else {
					add_li_e_unrepeated(list,r.tag_scan1.tags1[i]);
				}
			}
		}
	}
}

struct list_e_tag1* catch_tags_debug(struct list_e_tag1 *list, int n_cycles){ //Função com debug
	int te;
	list->head=NULL;
	list->size = 0;
	ip_stack_t stats;
	int32_t val=0, i=0, j=0;
	for(int y=0; y<n_cycles;y++){
		gettimeofday(&tv1, NULL);
		gettimeofday(&tv5, NULL);
		val = handle_rfid_module_read2();
		gettimeofday(&tv2, NULL);
		fprintf(log,"\n\n-----------------------------------------------------------\n\nTempo da %dª Leitura: %f s\n\n\n", y+1, (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec));
		if(val>0){
			te=1;
			gettimeofday(&tv3, NULL);
			for(i = 0; i < val; i++) {
				if(debug)
					gettimeofday(&tv1, NULL);
				if(list->size<1){
					add_e_tag1(list,r.tag_scan1.tags1[i]);
				} else {
					add_li_e_unrepeated(list,r.tag_scan1.tags1[i]);
				}
				gettimeofday(&tv2, NULL);
				if(te>9){
					fprintf(log,"Tempo para adicionar %dª tag: %f s\n", te++, (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec));
				} else {
					fprintf(log,"Tempo para adicionar 0%dª tag: %f s\n", te++, (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec));
				}
			}
			gettimeofday(&tv4, NULL);
			gettimeofday(&tv6, NULL);
			fprintf(log,"\n\nTempo para Adicionar todas as tags: %f s\nTempo total da %dª Leitura: %f s\nTamanho da lista de TAGs após a leitura: %d\n", (double)(tv4.tv_usec - tv3.tv_usec)/1000000 + (double)(tv4.tv_sec - tv3.tv_sec), y+1, (double)(tv6.tv_usec - tv5.tv_usec)/1000000 + (double)(tv6.tv_sec - tv5.tv_sec),  list->size);
		}
	}
	return list;
}

int32_t handle_rfid_module_read2(){
	r.msg.code = START_AUTO_READ2;
	r.msg.payload_length[0] = 0x00;
	r.msg.payload_length[1] = 0x05;
	r.msg.payload[0] = 0x02; // Reserved
	r.msg.payload[1] = TAGLIST_SIZE; //Max number of tags/packet must be TAGLIST_SIZE
	r.msg.payload[2] = 0x01; // Maximum elapsed time to tagging (sec)
	r.msg.payload[3] = 0x00; // How many times reader perform inventory round [MSB]
	r.msg.payload[4] = 0x05; // How many times reader perform inventory round [LSB]
	int32_t val = handle_rfid_module(&r.r1, &r.msg, &r.response, &r.tag_scan1, &r.tag_scan2, &r.tag_scan3, &r.tag_scan4, &r.tag_scan5, &r.tag_scan6);
	if (val < 0){
		printf("\noperation failed! (%d)\n", val);
		Gen2ReaderDestroy(&r.r1);
		init_reader();
	} else if(r.response.code == MSG_TYPE_ERROR){
		printf("\nError on RFID module: %02x %02x %02x %02x", r.response.code, r.response.payload[0], r.response.payload[1], r.response.payload[2]);
	}
	return val;
}

void del_e_li(struct list_e_tag1 *list){ //Da free em todos os nós da lista
	struct node *n = list->head;
	while(list->head!=NULL){
		list->head =n->next;
		free(n);
		n = list->head;
	}
	list->size=0;
}

void add_e_tag1(struct list_e_tag1 *list, struct tag1 tag){
	struct node *n = malloc(sizeof(struct node));
	n->next = list->head;
	list->head = n;
	n->tag = tag;
	list->size++;
}

void print_e_list(struct list_e_tag1 *list){
	if(list->head==NULL){
		printf("Lista de TAGs Vazia!\n\n");
	} else {
		printf("Lista de Tags:\n\n");
		struct node *n = list->head;
		int i = 0;
		while(n!=NULL){
			printf("Tag %d",i+1);
			print_tag1(&n->tag);
			i++;
			n = n->next;
		}
	}
}

void add_li_e_unrepeated(struct list_e_tag1 *list, struct tag1 c){
	if(cmp_li_e_tag1(list,c))
		add_e_tag1(list,c);
}

bool cmp_li_e_tag1(struct list_e_tag1 *list, struct tag1 c){
	struct node *n = list->head;
	while(n!=NULL){
		if(cmp_tag1(c, n->tag))
			return false;
		n = n->next;
	}
	return true;
}

void init_reader(){ //inicia o socket???(a definir)
	//~ Gen2ReaderDestroy(&r.r2);
	//~ Gen2ReaderDestroy(&r.r1);
	int32_t val = Gen2ReaderCreate("192.168.5.10", &r.r1);
	if (val)
	{
		printf("Error creating reader session.\n");
		exit(1);
	}
	r.msg.code = SET_TX_POWER_LEVEL;
	r.msg.payload_length[0] = 0x00;
	r.msg.payload_length[1] = 0x02;
	// MSB of the tx power level
	r.msg.payload[0] = (((20 * 10) >> 8) & 0x00ff);
	// LSB of the tx power level
	r.msg.payload[1] = ((20 * 10) & 0x00ff);
	val = handle_rfid_module(&r.r2, &r.msg, &r.response);
	if (val)
	{
		printf("Error setting 20 dbm.\n");
		exit(1);
	}
}

void init_reader_config(){
	//~ Gen2ReaderDestroy(&r.r2);
	//~ Gen2ReaderDestroy(&r.r1);
	int32_t val = Gen2ReaderCreate("192.168.5.10", &r.r2);
	if (val)
	{
		printf("Error creating reader session.\n");
		exit(1);
	}
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

void print_tag1(struct tag1 *h){ //Printa uma tag
	printf("\nPC: %.2x %.2x | EPC: ",h->pc[0], h->pc[1]);
	uint16_t epclen = (h->pc[0] >> 2);
	for(int j = 0; j < epclen; j++)
		printf("%.2x ", h->epc[j]);
	printf("\n");
}
