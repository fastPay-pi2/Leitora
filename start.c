#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "reader_api.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <time.h>

FILE *log;
struct timeval  tv1, tv2, tv3, tv4, tv5, tv6, tv7, tv8;
bool debug = true;
int count=1, n_cycles = 3;
int8_t n_antenas = 2;

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

typedef struct list_tag1 {
	struct tag1 *h;
	int size;
}list_tag1;

void print_tag1(struct tag1 *h);
bool cmp_tag1(struct tag1 a, struct tag1 b);
struct list_e_tag1* catch_tags(struct list_e_tag1 *list, int n_cycles);
void init_reader();
void init_reader_config();
static void activate(GtkApplication* app, gpointer user_data);
void add_e_tag1(struct list_e_tag1 *list, struct tag1 tag);
void print_e_list(struct list_e_tag1 *list);
bool cmp_li_e_tag1(struct list_e_tag1 *list, struct tag1 c);
void add_li_e_unrepeated(struct list_e_tag1 *list, struct tag1 c);
void del_e_li(struct list_e_tag1 *list);
void open_log();
int32_t handle_rfid_module_read2();
struct list_e_tag1* catch_tags_debug(struct list_e_tag1 *list, int n_cycles);
void cycle();
void set_antenna(uint8_t arg);

//-------------------------------------------------------------------------------------- INÍCIO MAIN --------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------- INÍCIO MAIN --------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------- INÍCIO MAIN --------------------------------------------------------------------------------------//

int main(int argc, char **argv){
	if(debug){
		log = fopen("log.txt","w+");
		fprintf(log, "Log de Temporização:\n");
		gettimeofday(&tv7, NULL);
	}
	init_reader_config();
	init_reader();
	if(debug){
		gettimeofday(&tv8, NULL);
		fprintf(log, "Tempo de init_reader() && init_reader_config(): %f s\n",(double)(tv8.tv_usec - tv7.tv_usec)/1000000 + (double)(tv8.tv_sec - tv7.tv_sec));
		fclose(log);
	}
	//~ GtkApplication *app;
	//~ int status;
	//~ app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	//~ g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
	//~ status = g_application_run(G_APPLICATION (app), argc, argv);
	//~ g_object_unref(app);
	while(1){
		struct list_e_tag1 *list = malloc(sizeof(struct list_tag1));
		if(debug)
				gettimeofday(&tv7, NULL);
		for(uint8_t a=0;a<n_antenas;a++){
			if(debug){
				open_log();
				fprintf(log, "\n\nCiclo %d.",count);
				fclose(log);
				list = catch_tags_debug(list, n_cycles);
			} else {
				list = catch_tags(list, n_cycles);
			}
			set_antenna(a+1);
		}
		if(debug)
				gettimeofday(&tv8, NULL);
		system("clear");
		printf("ciclo %d:\n\n",count++);
		print_e_list(list);
		if(debug){
			open_log();
			fprintf(log, "\n\nTempo do ciclo: %f\n\n-----------------------------------------------------------\n-----------------------------------------------------------\n-----------------------------------------------------------\n\n\n",(double)(tv8.tv_usec - tv7.tv_usec)/1000000 + (double)(tv8.tv_sec - tv7.tv_sec));
			fclose(log);
		}
		del_e_li(list);
		free(list);
		printf("\nDigite 's' para fazer um novo ciclo:\n");
		while(getc(stdin)=='\n');
		//sleep(2);
	}
	return 0;
}

//-------------------------------------------------------------------------------------- FIM MAIN --------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------- FIM MAIN --------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------- FIM MAIN --------------------------------------------------------------------------------------//

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


struct list_e_tag1* catch_tags(struct list_e_tag1 *list, int n_cycles){ //Função para captura de CÓDIGOS de Tags, retorna uma lista encadeada de TAGS não repetidas, a quantidade de cilos determinar quantas vezes a leitora ira fazer um pacote de TAGS para aumentar a chance de todas as TAGS serem lidas
	int te = 1;
	list->head=NULL;
	list->size = 0;
	ip_stack_t stats;
	int32_t val=0, i=0, j=0;
	for(int y=0; y<n_cycles;y++){
		val = handle_rfid_module_read2();
		if(val>0){
			te=1;
			for(i = 0; i < val; i++) {
				if(list->size<1){
					add_e_tag1(list,r.tag_scan1.tags1[i]);
				} else {
					add_li_e_unrepeated(list,r.tag_scan1.tags1[i]);
				}
			}
		}
	}
	return list;
}

struct list_e_tag1* catch_tags_debug(struct list_e_tag1 *list, int n_cycles){ //Função com debug
	open_log();
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
	if(debug)
		fclose(log);
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
	r.msg.payload[4] = 0x04; // How many times reader perform inventory round [LSB]
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
