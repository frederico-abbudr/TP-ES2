#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <math.h>
#include <time.h>


const float FPS = 100;  

const int SCREEN_W = 960;
const int SCREEN_H = 540;

//Para o campo de força e o tanque, theta é o angulo de cima do triangulo
const float THETA = M_PI/4;
const float RAIO_CAMPO = 30;

const float VEL_TANQUE = 3.5; //é a velocidade que o tanque anda pra frente e pra tras
const float PASSO_ANGULO = M_PI/90; // velocidade que o tanque rotaciona

const float RAIO_TIRO = 5;
const float VEL_TIRO = 7;

float OBS1_SUP_X = 350;
float OBS1_SUP_Y = 150;
float OBS1_INF_X = 400;
float OBS1_INF_Y = 450;

const float OBS2_SUP_X = 550;
const float OBS2_SUP_Y = 150;
const float OBS2_INF_X = 600;
const float OBS2_INF_Y = 450;

typedef struct Ponto{

	float x,y;

}Ponto;

typedef struct Tiro{
	Ponto c;
	float veloc;
}Tiro;

//Para criar o tanque
typedef struct Tanque 
{
	Ponto centro; //coordenadas do campo de força
	Ponto A,B,C; //vertices do triangulo de dentro
	Ponto T; //centro do circulo do tiro
	ALLEGRO_COLOR cor;

	float vel;	
	float angulo;  //para rotacionar ele
	float x_comp, y_comp;  //tambem para rotacionar
	float vel_angular; //velocidade de rotação
	Tiro tiro;

}Tanque;



//inicializa o tanque
void initTanque(Tanque *t, float a, float c,int r, int g, int b){
	

	t->centro.x = SCREEN_W/a;
	t->centro.y = SCREEN_H/c;
	t->cor = al_map_rgb(r,g,b); 

	//Esses A,B,h,w e a lógica tão na vídeo aula certinho, que usa a lei dos senos e pa
	t->A.x = 0; 
	t->A.y = -RAIO_CAMPO;

	float alpha = M_PI/2 - THETA;

	float h = RAIO_CAMPO*sin(alpha);
	float w = RAIO_CAMPO*sin(THETA);

	t->B.x = -w;
	t->B.y = h;

	t->C.x = w;
	t->C.y = h;

	t->vel = 0;
	t->angulo = M_PI/2; //Angulo inicial
	t->x_comp = cos(t->angulo);
	t->y_comp = sin(t->angulo);

	t->vel_angular = 0;

	t->tiro.c.x = 0;
	t->tiro.c.y = -RAIO_CAMPO;
	t->tiro.veloc = 0;
}

void desenhaCenario(){

	al_clear_to_color(al_map_rgb(181,60,60));//DEIXA O CENÁRIO DA COR
	al_draw_filled_rectangle(OBS1_SUP_X,OBS1_SUP_Y,OBS1_INF_X,OBS1_INF_Y, al_map_rgb(230,166,54));
	al_draw_filled_rectangle(OBS2_SUP_X,OBS2_SUP_Y,OBS2_INF_X,OBS2_INF_Y, al_map_rgb(230,166,54)); //DESENHA O OBSTACULO
}

void desenhaTanque(Tanque t){
	al_draw_circle(t.centro.x, t.centro.y, RAIO_CAMPO, t.cor,1); //FAZ O CAMPO DE FORÇA DO TANQUE
	al_draw_filled_triangle(t.A.x + t.centro.x, t.A.y + t.centro.y, //FAZ O TRIANGULO DO TANQUE
		                    t.B.x + t.centro.x, t.B.y + t.centro.y,
		                    t.C.x + t.centro.x, t.C.y + t.centro.y,
		                    t.cor);
	al_draw_filled_circle(t.tiro.c.x + t.centro.x, t.tiro.c.y + t.centro.y, RAIO_TIRO, t.cor); //FAZ O TIRO
}

void rotate(Ponto *P, float angle){

	float x=P->x, y=P->y;

	P->x = (x*cos(angle)) - (y*sin(angle));
	P->y = (y*cos(angle)) + (x*sin(angle));
}

void rotacionaTanque(Tanque *t){

	if(t->vel_angular != 0){

		rotate(&t->A, t->vel_angular);
		rotate(&t->B, t->vel_angular);
		rotate(&t->C, t->vel_angular);
		rotate(&t->tiro.c, t->vel_angular);

		t->angulo += t->vel_angular;

		t->x_comp = cos(t->angulo);
		t->y_comp = sin(t->angulo);
	}
}

void atualizaTanque(Tanque *t, int dist) {
	// Calcula a colisão com o obstáculo
	bool colideComObstaculo =
		(t->centro.x >= OBS1_SUP_X && t->centro.x <= OBS1_INF_X &&
		(t->centro.y + RAIO_CAMPO >= OBS1_SUP_Y && t->centro.y - RAIO_CAMPO <= OBS1_INF_Y) ||
		(t->centro.x >= OBS2_SUP_X && t->centro.x <= OBS2_INF_X &&
		(t->centro.y + RAIO_CAMPO >= OBS2_SUP_Y && t->centro.y - RAIO_CAMPO <= OBS2_INF_Y)));

	// Calcula a colisão com o outro tanque
	bool colideComOutroTanque = (dist == 1 && t->centro.y + RAIO_CAMPO + t->vel * t->y_comp <= SCREEN_H &&
								t->centro.y - RAIO_CAMPO + t->vel * t->y_comp > 0 ||
								dist == 2 && t->centro.y - RAIO_CAMPO - t->vel * t->y_comp >= 0 &&
								t->centro.y + RAIO_CAMPO - t->vel * t->y_comp <= SCREEN_H ||
								dist == 3 && t->centro.x - RAIO_CAMPO - t->vel * t->x_comp >= 0 &&
								t->centro.x + RAIO_CAMPO - t->vel * t->x_comp <= SCREEN_W ||
								dist == 4 && t->centro.x + RAIO_CAMPO + t->vel * t->x_comp <= SCREEN_W &&
								t->centro.x - RAIO_CAMPO + t->vel * t->x_comp >= 0);

	// Move o tanque
	if (!colideComObstaculo && !colideComOutroTanque) {
		t->centro.y += t->vel * t->y_comp;
		t->centro.x += t->vel * t->x_comp;
	}

	// Move o tiro
	t->tiro.c.x += t->tiro.veloc * t->x_comp;
	t->tiro.c.y += t->tiro.veloc * t->y_comp;
}


int calculaDistTanques(Tanque *t1,Tanque *t2){
	float dist, dist1;
	//distancia entre pontos : sqrt((xa-xb)^2 + (ya-yb)^2)

	dist = sqrt(pow((t1->centro.x - t2->centro.x),2) + pow((t1->centro.y - t2->centro.y),2));	

	if(dist <= 2*RAIO_CAMPO + 5 && (t1->centro.x > t2->centro.x) && (t1->centro.y > t2->centro.y)) return 1;

	if(dist <= 2*RAIO_CAMPO + 5 && (t1->centro.x > t2->centro.x) && (t1->centro.y < t2->centro.y)) return 2;
	
	if(dist <= 2*RAIO_CAMPO + 5 && (t1->centro.x < t2->centro.x) && (t1->centro.y > t2->centro.y)) return 3;

	if(dist <= 2*RAIO_CAMPO + 5 && (t1->centro.x < t2->centro.x) && (t1->centro.y < t2->centro.y)) return 4;

	else return 0;
}

int calculaTiro(Tanque *t1, Tanque *t2){
	
	float dist = 0,x,y, aux_x,aux_y,aux=0, aux2, aux3,soma, distTanques;

	dist = sqrt(pow((t1->centro.x - (t2->tiro.c.x + t2->centro.x)),2) + pow((t1->centro.y - (t2->tiro.c.y + t2->centro.y)),2));

	//CONFERE SE O TIRO SCOLIDIU COM O OBSTACULO
	if((t2->tiro.c.y + t2->centro.y >= OBS1_SUP_Y && t2->tiro.c.y + t2->centro.y <= OBS1_INF_Y)&& 
		(t2->tiro.c.x + t2->centro.x + RAIO_TIRO >= OBS1_SUP_X && t2->tiro.c.x + t2->centro.x - RAIO_TIRO <= OBS1_INF_X)){
			
		t2->tiro.veloc = 0;
		t2->tiro.c.x = t2->A.x;
		t2->tiro.c.y = t2->A.y;
		return 0;
	}

	if((t2->tiro.c.y + t2->centro.y >= OBS2_SUP_Y && t2->tiro.c.y + t2->centro.y <= OBS2_INF_Y)&& 
		(t2->tiro.c.x + t2->centro.x + RAIO_TIRO >= OBS2_SUP_X && t2->tiro.c.x + t2->centro.x - RAIO_TIRO <= OBS2_INF_X)){
			
		t2->tiro.veloc = 0;
		t2->tiro.c.x = t2->A.x;
		t2->tiro.c.y = t2->A.y;
		return 0;
	}

	if((t2->tiro.c.x + t2->centro.x>= OBS1_SUP_X && t2->tiro.c.x + t2->centro.x <= OBS1_INF_X) && 
		(t2->tiro.c.y + t2->centro.y + RAIO_TIRO >= OBS1_SUP_Y && t2->tiro.c.y + t2->centro.y - RAIO_TIRO <= OBS1_INF_Y)){

		t2->tiro.veloc = 0;
		t2->tiro.c.x = t2->A.x;
		t2->tiro.c.y = t2->A.y;
		return 0;
	}

	if((t2->tiro.c.x + t2->centro.x>= OBS2_SUP_X && t2->tiro.c.x + t2->centro.x <= OBS2_INF_X) && 
		(t2->tiro.c.y + t2->centro.y + RAIO_TIRO >= OBS2_SUP_Y && t2->tiro.c.y + t2->centro.y - RAIO_TIRO <= OBS2_INF_Y)){

		t2->tiro.veloc = 0;
		t2->tiro.c.x = t2->A.x;
		t2->tiro.c.y = t2->A.y;
		return 0;
	}

	//CONFERE SE O TIRO SAIU DA TELA
	if(t2->tiro.c.x + t2->centro.x >= SCREEN_W || t2->tiro.c.x + t2->centro.x <= 0 || 
		t2->tiro.c.y + t2->centro.y >= SCREEN_H || t2->tiro.c.y + t2->centro.y <= 0){

		t2->tiro.veloc = 0;
		t2->tiro.c.x = t2->A.x;
		t2->tiro.c.y = t2->A.y;
		return 0;
	}

	//CONFERE SE BATEU NO OUTRO TANQUE, POR ISSO O RETURN 1
	if(dist <= RAIO_CAMPO){
		
		t2->tiro.veloc = 0;
		t2->tiro.c.x = t2->A.x;
		t2->tiro.c.y = t2->A.y;
		return 1;
	}

	else return 0;
}

int main(int argc, char **argv){
	
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
   
	//----------------------- rotinas de inicializacao ---------------------------------------
    
	//inicializa o Allegro
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	
    //inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	
	
	//inicializa o modulo que permite carregar imagens no jogo
	if(!al_init_image_addon()){
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}
   
	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
 
	//cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

	//instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}
	
	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();

	//inicializa o modulo allegro que entende arquivos tff de fontes
	if(!al_init_ttf_addon()) {
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}
	
	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);
    ALLEGRO_FONT *size_22 = al_load_font("arial.ttf", 22, 1);

	if(size_32 == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}

	if(size_22 == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}

 	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}
   
	char pont[20],pont1[20],winblue[20], winred[20];
	int pontos_azul = 0, pontos_vermelho=0;
	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	  	
	 //inicializa o modulo allegro que carrega as fontes
    al_init_font_addon();
	//inicializa o modulo allegro que entende arquivos tff de fontes
    al_init_ttf_addon();

    //carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    //ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);

	srand(time(NULL));

	//cria o tanque
	// int a,b,w=1;
	int vitoria_vermelho = 0;
	int vitoria_azul = 0;
	int vitorias_azul = 0;
	int vitorias_vermelho = 0;
	int peso_vitoria = 0;
	float centro_x_tanque1 = 4, centro_y_tanque1 = 4;
	float centro_x_tanque2 = 1.3, centro_y_tanque2 = 1.25;
	Tanque tanque_1, tanque_2;
	Tiro t;
	FILE *azul;
	FILE *vermelho;

	azul = fopen("blue.txt", "a");
	vermelho = fopen("red.txt", "a");
	initTanque(&tanque_1, centro_x_tanque1, centro_y_tanque1 ,0,0,255);
	initTanque(&tanque_2, centro_x_tanque2, centro_y_tanque2, 0,212,119);

	//inicia o temporizador
	al_start_timer(timer);
	
	int playing = 1;
	while(playing) {
		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {

			desenhaCenario();

			//PONTUAÇÃO,CONFERE SE O JOGO ACABOU E FAZ A TELA DE ENDGAME
			if(pontos_azul >= 5){ //SE QUISER PASSAR DESSA PARTE DO CODIGO, PULA PRA LINHA *456*

				al_clear_to_color(al_map_rgb(0,0,0));
			
				tanque_1.centro.x = -1000;
				tanque_2.centro.x = -2000;

 				al_draw_text(size_32, al_map_rgb(0,0,255), OBS1_SUP_X, OBS1_SUP_Y + 16, 0, "FIM DE JOGO");	
 				al_draw_text(size_32, al_map_rgb(0,0,255), OBS1_SUP_X, OBS1_SUP_Y + 48, 0, "AZUL GANHOU!");

 				vitoria_azul = 1;

 				//EXIBE O HISTORICO DE PARTIDAS	
 				fprintf(azul,"%d ",w);
 				fclose(azul);		
 			

				azul = fopen("blue.txt", "r");
				vermelho = fopen("red.txt", "r");

				while(!feof(azul)){
					fscanf(azul, "%d", &peso_vitoria);
					vitorias_azul += peso_vitoria;
				}

				while(!feof(vermelho)){
					fscanf(vermelho, "%d", &peso_vitoria);
					vitorias_vermelho += peso_vitoria;
				}

				fclose(azul);
				fclose(vermelho);

				if(vitorias_azul != 0 || vitorias_vermelho != 0){
					vitorias_azul--;
					vitorias_vermelho--;
				}

				sprintf(winblue, "%d ", vitorias_azul);
				sprintf(winred, "%d ", vitorias_vermelho);

				al_draw_text(size_22, al_map_rgb(255, 255, 0), OBS1_SUP_X + 10,  (OBS2_INF_Y + OBS2_SUP_Y)/2 + 30, 0, "HISTÓRICO");
				al_draw_text(size_22, al_map_rgb(1, 1, 255), OBS1_SUP_X + 10, (OBS1_INF_Y + OBS1_SUP_Y)/2 + 50, 0, "vitorias do azul: ");
				al_draw_text(size_22, al_map_rgb(1, 212, 119), OBS1_SUP_X + 10, (OBS1_INF_Y + OBS1_SUP_Y)/2 + 70, 0, "vitorias do verde: ");
				al_draw_text(size_22, al_map_rgb(1, 1, 255), OBS2_INF_X - 40, (OBS1_INF_Y + OBS1_SUP_Y)/2 + 50, 0, winblue);
				al_draw_text(size_22, al_map_rgb(0, 212, 119), OBS2_INF_X - 40, (OBS2_INF_Y + OBS2_SUP_Y)/2 + 70, 0, winred);
		 }


 		if(pontos_vermelho >= 5){
			al_clear_to_color(al_map_rgb(0,0,0));
				
			tanque_1.centro.x = -1000;
			tanque_2.centro.x = -2000;
				
 			al_draw_text(size_32, al_map_rgb(1, 212, 119), OBS1_SUP_X, OBS1_SUP_Y + 16, 0, "FIM DE JOGO");	
 			al_draw_text(size_32, al_map_rgb(1, 212, 119), OBS1_SUP_X, OBS1_SUP_Y + 48, 0, "VERDE GANHOU!");
 			
 			vitorias_vermelho = 1;

 			fprintf(vermelho,"%d ",w);
			fclose(vermelho);
			

			azul = fopen("blue.txt", "r");
			vermelho = fopen("red.txt", "r");

			while(!feof(azul)){
				fscanf(azul, "%d", &peso_vitoria);
				vitorias_azul += peso_vitoria;
			}

			while(!feof(vermelho)){
				fscanf(vermelho, "%d", &peso_vitoria);
				vitorias_vermelho += peso_vitoria;
			}

			fclose(azul);
			fclose(vermelho);

			if(vitorias_azul != 0 || vitorias_vermelho != 0){
				vitorias_azul--;
				vitorias_vermelho--;
			}

			sprintf(winblue, "%d ", vitorias_azul);
			sprintf(winred, "%d ", vitorias_vermelho);

			al_draw_text(size_22, al_map_rgb(255, 255, 0), OBS1_SUP_X + 10,  (OBS2_INF_Y + OBS2_SUP_Y)/2 + 30, 0, "HISTÓRICO");
			al_draw_text(size_22, al_map_rgb(1, 1, 255), OBS1_SUP_X + 10, (OBS1_INF_Y + OBS1_SUP_Y)/2 + 50, 0, "vitorias do azul: ");
			al_draw_text(size_22, al_map_rgb(1, 212, 119), OBS1_SUP_X + 10, (OBS1_INF_Y + OBS1_SUP_Y)/2 + 70, 0, "vitorias do verde: ");
			al_draw_text(size_22, al_map_rgb(1, 1, 255), OBS2_INF_X - 40, (OBS1_INF_Y + OBS1_SUP_Y)/2 + 50, 0, winblue);
			al_draw_text(size_22, al_map_rgb(0, 212, 119), OBS2_INF_X - 40, (OBS2_INF_Y + OBS2_SUP_Y)/2 + 70, 0, winred);

 		}//FIM DO WHILE DOS PONTOS (SE QUISER CHEGAR NO INICIO DESSA PARTE, PULA PRA LINHA *343*)


 			if(pontos_azul <= 5 && pontos_vermelho <= 5){
			    
			    sprintf(pont, "%d ", pontos_azul);
			    sprintf(pont1, "%d ", pontos_vermelho);
			   	

			    al_draw_text(size_32, al_map_rgb(0, 0, 255), OBS1_SUP_X + 10, (OBS1_INF_Y + OBS1_SUP_Y)/2 - 16, 0, pont);
			    al_draw_text(size_32, al_map_rgb(50, 168, 19), OBS2_INF_X - 30, (OBS2_INF_Y + OBS2_SUP_Y)/2 - 16, 0, pont1);
			    
				int dist_t1_t2 = calculaDistTanques(&tanque_1, &tanque_2);
				int dist_t2_t1 = calculaDistTanques(&tanque_2, &tanque_1);
				
				pontos_azul += calculaTiro(&tanque_2, &tanque_1);
				pontos_vermelho += calculaTiro(&tanque_1, &tanque_2);  

				
				atualizaTanque(&tanque_1, dist_t1_t2); 
				atualizaTanque(&tanque_2, dist_t2_t1); 
				
				desenhaTanque(tanque_1);
				desenhaTanque(tanque_2);
			
			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();
			}
		}


		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}
		

		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN){
			//imprime qual tecla foi
			//printf("\ncodigo tecla: %d", ev.keyboard.keycode);

			if(pontos_azul < 5 && pontos_vermelho < 5){
			switch(ev.keyboard.keycode){

				//Movimenta o tanque
				case ALLEGRO_KEY_W:
				tanque_1.vel -= VEL_TANQUE;
				break;

				case ALLEGRO_KEY_S:
				tanque_1.vel += VEL_TANQUE;
				break;

				case ALLEGRO_KEY_D:
				tanque_1.vel_angular += PASSO_ANGULO;
				break;

				case ALLEGRO_KEY_A:
				tanque_1.vel_angular -= PASSO_ANGULO;
				break;

				case ALLEGRO_KEY_Q:
				tanque_1.tiro.veloc -= VEL_TIRO;
				break;

				//TANQUE 2
				case ALLEGRO_KEY_UP:
				tanque_2.vel -= VEL_TANQUE;
				break;

				case ALLEGRO_KEY_DOWN:
				tanque_2.vel += VEL_TANQUE;
				break;

				case ALLEGRO_KEY_RIGHT:
				tanque_2.vel_angular += PASSO_ANGULO;
				break;

				case ALLEGRO_KEY_LEFT:
				tanque_2.vel_angular -= PASSO_ANGULO;
				break;

				case ALLEGRO_KEY_M:
				tanque_2.tiro.veloc -= VEL_TIRO;
				break;

			}
			}

		}

		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_UP && pontos_azul <= 5 && pontos_vermelho <= 5) {
			//imprime qual tecla foi
			printf("\ncodigo tecla: %d", ev.keyboard.keycode);

			if(pontos_azul < 5 && pontos_vermelho < 5){
			switch(ev.keyboard.keycode){

				//Movimenta o tanque
				case ALLEGRO_KEY_W:
				tanque_1.vel += VEL_TANQUE;
				break;

				case ALLEGRO_KEY_S:
				tanque_1.vel -= VEL_TANQUE;
				break;

				case ALLEGRO_KEY_D:
				tanque_1.vel_angular -= PASSO_ANGULO;
				break;

				case ALLEGRO_KEY_A:
				tanque_1.vel_angular += PASSO_ANGULO;
				break;


				//TANQUE 2
				case ALLEGRO_KEY_UP:
				tanque_2.vel += VEL_TANQUE;
				break;

				case ALLEGRO_KEY_DOWN:
				tanque_2.vel -= VEL_TANQUE;
				break;

				case ALLEGRO_KEY_RIGHT:
				tanque_2.vel_angular -= PASSO_ANGULO;
				break;

				case ALLEGRO_KEY_LEFT:
				tanque_2.vel_angular += PASSO_ANGULO;
				break;

			}
			}
		}

	} //fim do while
     
	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)

	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
   
 
	return 0;
}