#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "reader_api.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <time.h>

reader_t r1;
FILE *log;
struct timeval  tv1, tv2, tv3, tv4, tv5, tv6;
#define debug true

typedef struct node {
	struct node *next;
	struct tag1 tag;
} node;

typedef struct list_e_tag1{
	struct node *head;
	int size;
} list_e_tag1;

typedef struct list_tag1 {
	struct tag1 *h;
	int size;
}list_tag1;

void print_tag1(struct tag1 *h);
//~ void add_tag1(struct list_tag1 *list,struct tag1 tag);
bool cmp_tag1(struct tag1 a, struct tag1 b);
//~ bool cmp_li_tag1(struct list_tag1 *list, struct tag1 c);
//~ void print_li_tag1(struct list_tag1 *list);
//~ void add_unrepeated(struct list_tag1 *list, struct tag1 c);
//~ void print_scan(tag1_t tag_scan1, int32_t val);
struct list_e_tag1* catch_tags(int n_cycles);
void init();
static void activate(GtkApplication* app, gpointer user_data);
void add_e_tag1(struct list_e_tag1 *list, struct tag1 tag);
void print_e_list(struct list_e_tag1 *list);
bool cmp_li_e_tag1(struct list_e_tag1 *list, struct tag1 c);
void add_li_e_unrepeated(struct list_e_tag1 *list, struct tag1 c);
void del_e_li(struct list_e_tag1 *list);
void open_log();

int main(int argc, char **argv){
	init();
	if(debug){
		log = fopen("log.txt","w+");
		fprintf(log, "Log de Temporização:\n");
		fclose(log);
	}
	//~ GtkApplication *app;
	//~ int status;
	//~ app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	//~ g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
	//~ status = g_application_run(G_APPLICATION (app), argc, argv);
	//~ g_object_unref(app);
    struct timeval  tv1, tv2;
	int i=1;
	while(1){
		if(debug){
			open_log();
			fprintf(log, "\n\nCiclo %d.",i);
			fclose(log);
		}
		gettimeofday(&tv1, NULL);
		struct list_e_tag1 *list = catch_tags(10);
		gettimeofday(&tv2, NULL);
		system("clear");
		printf("ciclo %d:\n\n",i);
		i++;
		print_e_list(list);
		del_e_li(list);
		free(list);
		printf("\nDigite 's' para fazer um novo ciclo:\n");
		while(getc(stdin)=='\n');
		if(debug){
			open_log();
			fprintf(log, "\n\nTempo do ciclo: %f\n-----------------------------------------------------------\n-----------------------------------------------------------\n-----------------------------------------------------------\n\n\n",(double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec));
			fclose(log);
		}
		//sleep(2);
	}
	return 0;
}

void open_log(){
	log = fopen("log.txt","r+");
	if(log==NULL){
		fprintf(stderr, "Erro ao tentar abrir a.txt.");
		exit(1);
	}
	fseek(log, 0, SEEK_END);
}

//~ void cycle(){
	//~ int i=1;
	//~ struct list_e_tag1 *list = catch_tags(10);
	//~ system("clear");
	//~ printf("ciclo %d:\n\n",i);
	//~ i++;
	//~ print_e_list(list);
	//~ del_e_li(list);
	//~ free(list);
//~ }

struct list_e_tag1* catch_tags(int n_cycles){ //Função para captura de CÓDIGOS de Tags, retorna uma lista encadeada de TAGS não repetidas, a quantidade de cilos determinar quantas vezes a leitora ira fazer um pacote de TAGS para aumentar a chance de todas as TAGS serem lidas
	struct list_e_tag1 *list1 = malloc(sizeof(struct list_tag1));
	if(debug)
		open_log();
	int te = 1;
	list1->head=NULL;
	list1->size = 0;
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
		if(debug){
			gettimeofday(&tv1, NULL);
			gettimeofday(&tv5, NULL);
			
		}
		val = handle_rfid_module(&r1, &msg, &response, &tag_scan1, &tag_scan2, &tag_scan3, &tag_scan4, &tag_scan5, &tag_scan6);
		if(debug){
			gettimeofday(&tv2, NULL);
			fprintf(log,"\n\n-----------------------------------------------------------\n\nTempo da %dª Leitura: %f s\n\n\n", y+1, (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec));
		}
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
			if(debug)
				gettimeofday(&tv3, NULL);
			for(i = 0; i < val; i++) {
				if(debug)
					gettimeofday(&tv1, NULL);
				if(list1->size<1){
					add_e_tag1(list1,tag_scan1.tags1[i]);
				} else {
					add_li_e_unrepeated(list1,tag_scan1.tags1[i]);
				}
				if(debug){
					gettimeofday(&tv2, NULL);
					fprintf(log,"Tempo para adicionar tag da %dª leitura : %f s\n", te, (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec));
					te++;
				}
			}
			if(debug){
				gettimeofday(&tv4, NULL);
				fprintf(log,"\n\nTempo para Adicionar todas as tags: %f s\n", (double)(tv4.tv_usec - tv3.tv_usec)/1000000 + (double)(tv4.tv_sec - tv3.tv_sec));
			}
		}
		if(debug){
			gettimeofday(&tv6, NULL);
			fprintf(log,"Tempo total da %dª Leitura: %f s\n", y+1, (double)(tv6.tv_usec - tv5.tv_usec)/1000000 + (double)(tv6.tv_sec - tv5.tv_sec));
		}
	}
	if(debug)
		fclose(log);
	return list1;
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

void init(){ //inicia o socket???(a definir)
	int32_t val = Gen2ReaderCreate("192.168.5.10", &r1);
	if (val)
	{
		printf("Error creating reader session.\n");
		exit(1);
	}
}

//~ void add_unrepeated(struct list_tag1 *list, struct tag1 c){ //Adiciona um Tag à lista não encadeada se ela não for repetida
	//~ if(cmp_li_tag1(list,c))
		//~ add_tag1(list,c);
//~ }



//~ bool cmp_li_tag1(struct list_tag1 *list, struct tag1 c){ //Compara Lista não encadeada com tag, retorna true caso não seja identificado a tag dentro da lista
	//~ for(int j = 0; j < list->size; j++)
		//~ if(cmp_tag1(c, *((list->h)+j)))
			//~ return false;
	//~ return true;
//~ }



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



//~ void add_tag1(struct list_tag1 *list,struct tag1 tag){ //Adiciona uma Tag na lista não encadeada
	//~ list->size++;
	//~ list->h = realloc(list->h,sizeof(struct tag1)*list->size);
	//~ *(list->h-1+list->size) = tag;
//~ }



//~ void print_li_tag1(struct list_tag1 *list){ //Printa uma lista não encadeada de Tags
	//~ printf("Lista de Tags:\n\n");
	//~ for(int i = 0; i<list->size;i++){
		//~ printf("\nTag %d",i+1);
		//~ print_tag1(list->h+i);
	//~ }
//~ }



void print_tag1(struct tag1 *h){ //Printa uma tag
	printf("\nPC: %.2x %.2x | EPC: ",h->pc[0], h->pc[1]);
	uint16_t epclen = (h->pc[0] >> 2);
	for(int j = 0; j < epclen; j++)
		printf("%.2x ", h->epc[j]);
	printf("\n");
}



//~ void print_scan(tag1_t tag_scan1, int32_t val){ //apenas armazenando um código excluido que pode vir a ser útil
	//~ int j, i;
	//~ if(val == 0)
		//~ printf("\nNenhuma TAG Lida.");
	//~ else
	//~ {
		//~ printf("\n%d tags inventoried.", val);
		//~ for(i = 0; i < val; i++)
	//~ {
		//~ uint16_t epclen = (tag_scan1.tags1[i].pc[0] >> 2);
		//~ printf("\nPC: %.2x %.2x | ", tag_scan1.tags1[i].pc[0], tag_scan1.tags1[i].pc[1]);
		//~ printf("EPC: ");
		//~ for(j = 0; j < epclen; j++)
			//~ printf("%.2x ", tag_scan1.tags1[i].epc[j]);
		//~ }
	//~ }
//~ }

//~ static void activate(GtkApplication* app, gpointer user_data){
	//~ GtkWidget *window;
	//~ GtkWidget *button;
	//~ GtkWidget *button_box;
	//~ window = gtk_application_window_new(app);
	//~ gtk_window_set_title(GTK_WINDOW (window), "Window");
	//~ gtk_window_set_default_size(GTK_WINDOW (window), 200, 200);
	
	//~ button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	//~ gtk_container_add(GTK_CONTAINER (window), button_box);
	
	//~ button = gtk_button_new_with_label("cycle");
	//~ g_signal_connect(button, "clicked", G_CALLBACK(cycle), NULL);
	//~ g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_destroy), window);
	//~ gtk_container_add(GTK_CONTAINER (button_box), button);
	
	//~ gtk_widget_show_all(window);
//~ }
